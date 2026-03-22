#pragma warning(disable : 4996)
#include <iostream>
#include <string>
#include <bitset>
#include <vector>
#include <cstring>
#include <fstream>
#include <filesystem>
#include "Header.h"
using namespace std;
namespace fs = std::filesystem;

vector<string> GetFilenamesFromFolder(string path)
{
	vector<string> PathV;
	for (const auto& entry : fs::directory_iterator(path)) {
		PathV.push_back(entry.path().filename().string());
	}
	return PathV;
}

string ReadFilenameFromImageC(unsigned char* imgC, unsigned char* imgR, int& bitI, int& stringI)
{
	bool end = false;
	vector<bool> decoded;
	decoded.reserve(200);
	while (!end)
	{
		cout << "bitI:" << bitI << endl;
		cout << "stringI:" << stringI << endl;
		if (imgC[bitI] > 0 && imgC[bitI] < 255)
		{
			if (imgR[bitI] == imgC[bitI] - 1)
			{
				decoded.push_back(false);
				stringI++;
			}
			else if (imgR[bitI] == imgC[bitI] + 1)
			{
				decoded.push_back(true);
				stringI++;
			}
			else
			{
				end = true;
			}
		}
		bitI++;
		if (decoded.size() % 8 == 0)
		{
			if (BitsToAscii(decoded).find('|') != std::string::npos)
			{
				end = true;
			}
		}
	}
	string s = BitsToAscii(decoded);
	return  s.erase(s.size() - 1, 1);
}
void ReadDataFromImageC(unsigned char* imgC, unsigned char* imgR, int& bitI, int& stringI, vector<bool>& decoded)
{
	bool end = false;
	while (!end)
	{
		cout << "bitI:" << bitI << endl;
		cout << "stringI:" << stringI << endl;
		if (imgC[bitI] > 0 && imgC[bitI] < 255)
		{
			if (imgR[bitI] == imgC[bitI] - 1)
			{
				decoded.push_back(false);
				stringI++;
			}
			else if (imgR[bitI] == imgC[bitI] + 1)
			{
				decoded.push_back(true);
				stringI++;
			}
			else
			{
				end = true;
			}
		}
		bitI++;
	}
}

bool WriteToImage(unsigned char* img, size_t imgSize, const string& s, bool verbose, int& bitI, int& stringI)
{
	auto sLength = s.length();
	while (stringI < sLength)
	{
		if (bitI > imgSize)
		{
			return 1;
		}
		if (verbose)
		{
			cout << "bitI:" << bitI << "=" << static_cast<int>(img[bitI]) << endl;
			cout << "stringI:" << stringI << "=" << s[stringI] << endl;
		}
		if (img[bitI] > 0 && img[bitI] < 255)
		{
			if (s[stringI] == '0')
			{
				img[bitI]--;
				stringI++;
			}
			else if (s[stringI] == '1')
			{
				img[bitI]++;
				stringI++;
			}
		}
		bitI++;
	}
	return 0;
}

string TextToAsciiB(string s)
{
	string asciiS;
	for (char c : s) {
		std::bitset<8> b(c); // Convert char to 8-bit binary
		asciiS += b.to_string();
	}
	return asciiS;
}

//horrible for memory because a string element is a byte and not a bit
string ReadFileToBString(const string& filename)
{
	std::ifstream file(filename, std::ios::binary);

	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::string buffer(size, '\0');
	file.read(buffer.data(), size);
	buffer.insert(0, filename + '|');
	return TextToAsciiB(buffer);
}
void WriteBitsToFile(const std::string& filename, const std::vector<bool>& bits) {
	std::ofstream out(filename, std::ios::binary);
	if (!out) {
		throw std::runtime_error("Failed to open file");
	}

	std::vector<uint8_t> bytes;
	bytes.reserve((bits.size() + 7) / 8);

	for (size_t i = 0; i < bits.size(); i += 8) {
		uint8_t byte = 0;

		for (size_t j = 0; j < 8 && (i + j) < bits.size(); ++j) {
			if (bits[i + j]) {
				byte |= (1u << j);  // pack bit j into the byte
			}
		}

		bytes.push_back(byte);
	}

	out.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}
std::string BitsToAscii(const std::vector<bool>& bits)
{
	if (bits.size() % 8 != 0)
		throw std::runtime_error("Bit vector length must be a multiple of 8");

	std::string result;
	result.reserve(bits.size() / 8);

	for (size_t i = 0; i < bits.size(); i += 8)
	{
		unsigned char value = 0;

		for (int b = 0; b < 8; ++b)
		{
			value <<= 1;
			value |= bits[i + b] ? 1 : 0;
		}

		result.push_back(static_cast<char>(value));
	}

	return result;
}