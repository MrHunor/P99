#pragma warning(disable : 4996)
#include <iostream>
#include <string>
#include <bitset>
#include <vector>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <stb_image.h>
#include <stb_image_write.h>
#include "Header.h"
using namespace std;
namespace fs = std::filesystem;

bool createFolder(const string& name)
{
	if (std::filesystem::create_directory(name)) {
		return 0;
	}
	else {
		return 1;
	}
}

vector<string> GetFilenamesFromFolder(string path)
{
	vector<string> PathV;
	for (const auto& entry : fs::directory_iterator(path)) {
		PathV.push_back(entry.path().filename().string());
	}
	return PathV;
}

string ReadFilenameFromImageC(unsigned char* imgC, unsigned char* imgR, int& bitI, int& stringI, ostream& out)
{
	bool end = false;
	vector<bool> decoded;
	decoded.reserve(200);
	while (!end)
	{
		out << "bitI:" << bitI << endl;
		out << "stringI:" << stringI << endl;
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

void ReadDataFromImageC(unsigned char* imgC, unsigned char* imgR, int size, int& bitI, int& stringI, vector<bool>& decoded, ostream& out)
{
	bool end = false;
	while (!end && bitI < size)
	{
		//These two lines may be useful for debugging but are horrible for performace
		//	out << "bitI:" << bitI << "=" << static_cast<int>(img[bitI]) << endl;
	//out << "stringI:" << stringI << "=" << s[stringI] << endl;
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

//For which Filelist1 should be oFilelist and Filelist2 eFilelist
bool CheckFilelists(const vector<string>& FileList1, const vector<string>& FileList2)
{
	if (FileList1.size() != FileList2.size()) return true;

	for (size_t i = 0; i < FileList1.size(); i++)
	{
		string modified = FileList1[i];
		modified.insert(modified.length() - 4, 1, 'M');
		if (modified != FileList2[i]) return true;
	}
	return false;
}

bool DecodeFolder(const string& eFoldername, ostream& out)
{
	//Variables
	unsigned char* imgO{};
	unsigned char* imgE{};
	int w{}, h{}, channels{}, stringI{}, bitI{};
	string placeholder{}, oFoldername{}, fullPath{}, filename{};
	vector<string> eFileList{}, oFileList{};
	vector<bool> decoded{}, decodedBuffer{};

	cout << "Enter Original Foldername:";
	cin >> oFoldername;
	eFileList = GetFilenamesFromFolder(eFoldername);
	oFileList = GetFilenamesFromFolder(oFoldername);
	if (CheckFilelists(oFileList, eFileList) != 0)
	{
		out << "Filelists dont match!" << endl;
		return 1;
	}

	//First image hold the filename
	fullPath = oFoldername + "\\" + oFileList[0];
	out << "Loading:" << fullPath << endl;
	imgO = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);
	fullPath = eFoldername + "\\" + eFileList[0];
	out << "Loading:" << fullPath << endl;
	imgE = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);
	filename = ReadFilenameFromImageC(imgO, imgE, bitI, stringI, out);
	out << "Filename:" << filename << endl;
	//Read remaining data from first image
	ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decoded, out);

	//Read remaining data from the remaining images
	stbi_image_free(imgO);
	stbi_image_free(imgE);
	for (int i = 1; i < eFileList.size(); i++)
	{
		stringI = 0;
		bitI = 0;
		decodedBuffer.clear();
		fullPath = oFoldername + "\\" + oFileList[i];
		out << "Loading:" << fullPath << endl;
		imgO = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);
		fullPath = eFoldername + "\\" + eFileList[i];
		out << "Loading:" << fullPath << endl;
		imgE = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);
		ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decodedBuffer, out);
		decoded.insert(decoded.end(), decodedBuffer.begin(), decodedBuffer.end());
		stbi_image_free(imgO);
		stbi_image_free(imgE);
	}

	out << "Writing to file:'" << filename << "'..." << endl;
	WriteBitsToFile(filename, decoded);
	out << "Finished writing." << endl;
	return 0;
}

bool DecodeImage(const string& eFilename, ostream& out)
{
	string placeholder, Filename;
	int w{}, h{}, channels{}, bitI{}, stringI{};
	vector<bool> decoded;
	unsigned char* imgR = stbi_load(eFilename.c_str(), &w, &h, &channels, 3);
	out << "Loading image '" << eFilename << "'...";
	if (imgR == nullptr)
	{
		out << "\nFailed to load image, exiting.";
		return 1;
	}
	out << "\nLoaded image.\n";
	cout << "Enter Original Filename:";
	cin >> placeholder;
	unsigned char* imgC = stbi_load(placeholder.c_str(), &w, &h, &channels, 3);
	if (imgC == nullptr)
	{
		out << "\nFailed to load image, exiting.";
		return 1;
	}
	out << "\nLoaded image.\n";
	//Pre Processing
	Filename = ReadFilenameFromImageC(imgC, imgR, bitI, stringI, out);
	out << "Filename:" << Filename << endl;
	//Actual Processing
	ReadDataFromImageC(imgC, imgR, (w * h * channels), bitI, stringI, decoded, out);
	out << "Read following binary:";
	for (bool i : decoded)
	{
		if (i == false)out << "0";
		if (i == true)cout << "1";
	}
	out << "\nWhich in ascci is:\n" << BitsToAscii(decoded) << endl;
	out << "Writing to file:'" << Filename << "'..." << endl;
	WriteBitsToFile(Filename, decoded);
	out << "Finished writing.\nFreeing memory..." << endl;
	stbi_image_free(imgR);
	stbi_image_free(imgC);
	out << "Finished";
	return 0;
}

bool EncodeImage(const string& filename, ostream& out)
{
	string placeholder{};
	int w{}, h{}, channels{}, imgSize{}, bitI{}, stringI{};
	out << "\nLoading image '" << placeholder << "'...\n";
	unsigned char* img = stbi_load(placeholder.c_str(), &w, &h, &channels, 3);
	imgSize = w * h * 4;
	if (img != nullptr)cout << "Loaded image.\n";
	else
	{
		cout << "Failed to load image.";
		return 1;
	}
	cout << "Enter Filename:";
	cin >> placeholder;
	string s = ReadFileToBString(placeholder);
	out << "\nRaw Binary:" << s << endl;
	out << "Size of text:" << s.length() << endl;
	if (s.length() > imgSize)
	{
		cout << "Input too Large." << endl;
		stbi_image_free(img);
		cout << "Finished.\n";
		return 1;
	}
	out << "Encoding...\n";
	WriteToImage(img, imgSize, s, out, bitI, stringI);
	out << "Writing file 'output.png'...\n";
	stbi_write_png("output.png", w, h, 4, img, 4 * w);
	stbi_image_free(img);
	out << "Finished.\n";
	return 0;
}

bool EncodeFolder(const string& ofoldername, ostream& out)
{
	vector<string> FileList;
	string placeholder{}, fullPath{}, sChunk{}, efoldername{};
	size_t inputSize{};
	int w{}, h{}, channels{}, bitcounter{}, NIL{}, bitI{}, stringI{};

	efoldername = ofoldername + "M";
	createFolder(efoldername);
	FileList = GetFilenamesFromFolder(ofoldername);
	out << "Files in Folder:" << endl;
	for (auto entry : FileList)
	{
		out << entry << endl;
	}
	cout << "Enter Filename(Files Bigger then 1GB might cause issues due to RAM overload):";
	cin >> placeholder;
	string s = ReadFileToBString(placeholder);
	inputSize = s.size();
	out << "Size of text:" << inputSize << endl;
	out << "Evaluating Image Load..." << endl;
	for (int i = 1; bitcounter < inputSize; i++)
	{
		if (i > FileList.size() - 1)
		{
			out << "Input too big to encode into availabe Images.\n";
			out << "Finished.";
			return 1;
		}
		fullPath = ofoldername + "\\" + FileList[i];
		stbi_info(fullPath.c_str(), &w, &h, &channels);
		out << w << " " << h << " " << channels << endl;
		bitcounter += (w * h * channels);
		NIL = i;
	}
	out << "NIL:" << NIL << "/" << FileList.size() << endl;
	out << "Encoding...";
	for (size_t i = 0; i < NIL; i++)
	{
		out << "Loading image '" << FileList[i] << "'..\n";
		fullPath = ofoldername + "\\" + FileList[i];
		out << "Loading:" << fullPath << endl;
		unsigned char* img = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);
		sChunk = s.substr(0, (w * h * channels));
		out << "Selected Chunk size:" << sChunk.length();
		out << "Encoding..." << endl;
		bitI = 0;
		stringI = 0;
		WriteToImage(img, (w * h * channels), sChunk, out, bitI, stringI);
		s.erase(0, stringI);
		out << "Encoded Chunck size:" << stringI;
		out << "Writing file..." << endl;
		fullPath = efoldername + "\\" + FileList[i].insert(FileList[i].length() - 4, 1, 'M');
		out << "Writing to:" << fullPath << endl;
		stbi_write_png(fullPath.c_str(), w, h, channels, img, channels * w);
		stbi_image_free(img);
	}
	out << "Finished!" << endl;
	return 0;
}

bool WriteToImage(unsigned char* img, size_t imgSize, const string& s, ostream& out, int& bitI, int& stringI)
{
	auto sLength = s.length();
	while (stringI < sLength)
	{
		if (bitI > imgSize)
		{
			return 1;
		}
		//These two lines may be useful for debugging but are horrible for performace
			//	out << "bitI:" << bitI << "=" << static_cast<int>(img[bitI]) << endl;
		//out << "stringI:" << stringI << "=" << s[stringI] << endl;
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
			byte <<= 1;
			byte |= bits[i + j] ? 1 : 0;
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