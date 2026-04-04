#pragma warning(disable : 4996)
#include <iostream>
#include <string>
#include <bitset>
#include <vector>
#include <fstream>
#include <filesystem>
#include <stb_image.h>
#include <stb_image_write.h>
#include <stdexcept>
#include <cstdint>
#include "Header.h"

namespace fs = std::filesystem;

using std::string;
using std::vector;
using std::cout;
using std::cin;
using std::endl;
using std::ostream;

/* =========================================================1. BASIC UTILITIES========================================================= */

void InvalidInputMessage(const string& details)
{
	cout << "Your Input was Invalid: " << details;
	exit(1);
}

bool createFolder(const string& name)
{
	return !fs::create_directory(name);
}

bool checkEx(const string& path)
{
	fs::path filePath = path; // FIXED
	return !fs::exists(filePath);
}

vector<string> GetFilenamesFromFolder(string path)
{
	vector<string> PathV;
	for (const auto& entry : fs::directory_iterator(path)) {
		PathV.push_back(entry.path().filename().string());
	}
	return PathV;
}

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

/* =========================================================2. BIT / FILE UTILITIES========================================================= */

string TextToAsciiB(const string& s)
{
	string asciiS;
	for (char c : s) {
		std::bitset<8> b(c);
		asciiS += b.to_string();
	}
	return asciiS;
}

string BitsToAscii(const vector<bool>& bits)
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

void ReadFileToArray(const std::string& filename, std::vector<bool>& array)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file)
		throw std::runtime_error("Failed to open file");

	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	// resize the vector to fit the file
	array.resize(size);

	std::vector<char> buffer(size);
	file.read(buffer.data(), size);

	for (size_t i = 0; i < size; i++)
		array[i] = (buffer[i] != 0);
}

void WriteBitsToFile(const std::string& filename, const std::vector<bool>& bits)
{
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

/* =========================================================3. LOW-LEVEL IMAGE LOGIC========================================================= */

bool WriteToImage(unsigned char* img, size_t imgSize, const vector<bool>& s, ostream& out, int& bitI, int& stringI)
{
	auto sLength = s.size();
	while (stringI < sLength)
	{
		if (bitI > imgSize) return 1;

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

void ReadDataFromImageC(unsigned char* imgC, unsigned char* imgR, int size, int& bitI, int& stringI, vector<bool>& decoded, ostream& out)
{
	bool end = false;
	while (!end && bitI < size)
	{
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

string ReadFilenameFromImageC(unsigned char* imgC, unsigned char* imgR, int& bitI, int& stringI, ostream& out)
{
	bool end = false;
	vector<bool> decoded;
	decoded.reserve(200);

	while (!end)
	{
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
	return s.erase(s.size() - 1, 1);
}

/* =========================================================4. IMAGE OPERATIONS========================================================= */

bool EncodeImage(const string& filename, ostream& out)
{
	string placeholder{};
	int w{}, h{}, channels{}, imgSize{}, bitI{}, stringI{};
	vector<bool> array;
	out << "\nLoading image '" << filename << "'...\n";
	unsigned char* img = stbi_load(filename.c_str(), &w, &h, &channels, 3);
	imgSize = w * h * 4;

	if (!img) return 1;

	cout << "Enter the containing Filename:";
	cin >> placeholder;
	if (checkEx(placeholder)) InvalidInputMessage("");

	ReadFileToArray(placeholder, array);

	if (array.size() > imgSize)
	{
		InvalidInputMessage("Given File is too big to encode. Encoding File size=" + std::to_string(array.size()) + "; imageSize" + std::to_string(imgSize));
		stbi_image_free(img);
		return 1;
	}

	WriteToImage(img, imgSize, array, out, bitI, stringI);

	stbi_write_png("output.png", w, h, 4, img, 4 * w);
	stbi_image_free(img);
	return 0;
}

bool DecodeImage(const string& eFilename, ostream& out)
{
	string placeholder, Filename;
	int w{}, h{}, channels{}, bitI{}, stringI{};
	vector<bool> decoded;

	unsigned char* imgR = stbi_load(eFilename.c_str(), &w, &h, &channels, 3); // FIXED
	if (!imgR) return 1;

	cout << "Enter Original Filename:";
	cin >> placeholder;
	if (checkEx(placeholder)) InvalidInputMessage("");

	unsigned char* imgC = stbi_load(placeholder.c_str(), &w, &h, &channels, 3);
	if (!imgC) return 1;

	Filename = ReadFilenameFromImageC(imgC, imgR, bitI, stringI, out);
	ReadDataFromImageC(imgC, imgR, (w * h * channels), bitI, stringI, decoded, out);

	WriteBitsToFile(Filename, decoded);

	stbi_image_free(imgR);
	stbi_image_free(imgC);
	return 0;
}

/* =========================================================5. FOLDER OPERATIONS========================================================= */

bool EncodeFolder(const string& ofoldername, ostream& out)
{
	vector<string> FileList = GetFilenamesFromFolder(ofoldername);
	vector<bool> array,aChunk;
	string placeholder{}, fullPath{}, efoldername{};
	size_t inputSize{};
	int w{}, h{}, channels{}, bitcounter{}, NIL{}, bitI{}, stringI{};

	efoldername = ofoldername + "M";
	createFolder(efoldername);

	cout << "Enter Encoding Filename:";
	cin >> placeholder;
	if (checkEx(placeholder)) InvalidInputMessage("");
	ReadFileToArray(placeholder, array);
	for (int i = 1; bitcounter < inputSize; i++)
	{
		if (i > FileList.size() - 1) return 1;

		fullPath = ofoldername + "\\" + FileList[i];
		stbi_info(fullPath.c_str(), &w, &h, &channels);

		bitcounter += (w * h * channels);
		NIL = i;
	}

	for (size_t i = 0; i < NIL; i++)
	{
		fullPath = ofoldername + "\\" + FileList[i];
		unsigned char* img = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);

		aChunk.assign(array.begin(), array.begin() + (w * h * channels));

		bitI = 0;
		stringI = 0;

		WriteToImage(img, (w * h * channels), aChunk, out, bitI, stringI);
		array.erase(array.begin(), array.begin()+stringI);

		fullPath = efoldername + "\\" + FileList[i].insert(FileList[i].length() - 4, 1, 'M');
		stbi_write_png(fullPath.c_str(), w, h, channels, img, channels * w);

		stbi_image_free(img);
	}

	return 0;
}

bool DecodeFolder(const string& eFoldername, ostream& out)
{
	unsigned char* imgO{};
	unsigned char* imgE{};
	int w{}, h{}, channels{}, stringI{}, bitI{};
	string oFoldername{}, fullPath{}, filename{};
	vector<string> eFileList{}, oFileList{};
	vector<bool> decoded{}, decodedBuffer{};

	cout << "Enter Original Foldername:";
	cin >> oFoldername;
	if (checkEx(oFoldername)) InvalidInputMessage("");

	eFileList = GetFilenamesFromFolder(eFoldername);
	oFileList = GetFilenamesFromFolder(oFoldername);

	if (CheckFilelists(oFileList, eFileList)) return 1;

	fullPath = oFoldername + "\\" + oFileList[0];
	imgO = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);

	fullPath = eFoldername + "\\" + eFileList[0];
	imgE = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);

	filename = ReadFilenameFromImageC(imgO, imgE, bitI, stringI, out);
	ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decoded, out);

	stbi_image_free(imgO);
	stbi_image_free(imgE);

	for (int i = 1; i < eFileList.size(); i++)
	{
		stringI = 0;
		bitI = 0;
		decodedBuffer.clear();

		imgO = stbi_load((oFoldername + "\\" + oFileList[i]).c_str(), &w, &h, &channels, 3);
		imgE = stbi_load((eFoldername + "\\" + eFileList[i]).c_str(), &w, &h, &channels, 3);

		ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decodedBuffer, out);
		decoded.insert(decoded.end(), decodedBuffer.begin(), decodedBuffer.end());

		stbi_image_free(imgO);
		stbi_image_free(imgE);
	}

	WriteBitsToFile(filename, decoded);
	return 0;
}

/* =========================================================6. NI VARIANTS========================================================= */

void EncodeImageNI(const string& ofilename, const string& cfilename, ostream& out)
{
	int w{}, h{}, channels{}, imgSize{}, bitI{}, stringI{};

	unsigned char* img = stbi_load(ofilename.c_str(), &w, &h, &channels, 3);
	imgSize = w * h * 4;

	if (!img) exit(1);
	if (checkEx(cfilename)) InvalidInputMessage("");

	string s = ReadFileToBString(cfilename);

	if (s.length() > imgSize) exit(1);

	WriteToImage(img, imgSize, s, out, bitI, stringI);
	stbi_write_png("output.png", w, h, 4, img, 4 * w);

	stbi_image_free(img);
}

void DecodeImageNI(const string& oFilename, const string& eFilename, ostream& out)
{
	string filename{};
	int w{}, h{}, channels{}, bitI{}, stringI{};
	vector<bool> decoded;

	unsigned char* imgE = stbi_load(eFilename.c_str(), &w, &h, &channels, 3);
	if (!imgE) exit(1);

	if (checkEx(oFilename)) InvalidInputMessage("");

	unsigned char* imgO = stbi_load(oFilename.c_str(), &w, &h, &channels, 3);
	if (!imgO) exit(1);

	filename = ReadFilenameFromImageC(imgO, imgE, bitI, stringI, out);
	ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decoded, out);

	WriteBitsToFile(filename, decoded);

	stbi_image_free(imgO);
	stbi_image_free(imgE);
}

void EncodeFolderNI(const string& ofoldername, const string& cfilename, ostream& out)
{
	vector<string> FileList = GetFilenamesFromFolder(ofoldername);
	string  fullPath{}, sChunk{}, efoldername{};
	size_t inputSize{};
	int w{}, h{}, channels{}, bitcounter{}, NIL{}, bitI{}, stringI{};

	efoldername = ofoldername + "M";
	createFolder(efoldername);

	if (checkEx(cfilename)) InvalidInputMessage("");

	string s = ReadFileToBString(cfilename);
	inputSize = s.size();

	for (int i = 1; bitcounter < inputSize; i++)
	{
		if (i > FileList.size() - 1)InvalidInputMessage("Availale Images cannot fit the requested file.");

		fullPath = ofoldername + "\\" + FileList[i];
		stbi_info(fullPath.c_str(), &w, &h, &channels);

		bitcounter += (w * h * channels);
		NIL = i;
	}

	for (size_t i = 0; i < NIL; i++)
	{
		fullPath = ofoldername + "\\" + FileList[i];
		unsigned char* img = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);

		sChunk = s.substr(0, (w * h * channels));

		bitI = 0;
		stringI = 0;

		WriteToImage(img, (w * h * channels), sChunk, out, bitI, stringI);
		s.erase(0, stringI);

		fullPath = efoldername + "\\" + FileList[i].insert(FileList[i].length() - 4, 1, 'M');
		stbi_write_png(fullPath.c_str(), w, h, channels, img, channels * w);

		stbi_image_free(img);
	}


}

void DecodeFolder(const string& oFoldername,const string& eFoldername, ostream& out)
{
	unsigned char* imgO{};
	unsigned char* imgE{};
	int w{}, h{}, channels{}, stringI{}, bitI{};
	string  fullPath{}, filename{};
	vector<string> eFileList{}, oFileList{};
	vector<bool> decoded{}, decodedBuffer{};


	if (checkEx(oFoldername)) InvalidInputMessage("");

	eFileList = GetFilenamesFromFolder(eFoldername);
	oFileList = GetFilenamesFromFolder(oFoldername);

	if (CheckFilelists(oFileList, eFileList)) InvalidInputMessage("Filelists Mismatch");

	fullPath = oFoldername + "\\" + oFileList[0];
	imgO = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);

	fullPath = eFoldername + "\\" + eFileList[0];
	imgE = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);

	filename = ReadFilenameFromImageC(imgO, imgE, bitI, stringI, out);
	ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decoded, out);

	stbi_image_free(imgO);
	stbi_image_free(imgE);

	for (int i = 1; i < eFileList.size(); i++)
	{
		stringI = 0;
		bitI = 0;
		decodedBuffer.clear();

		imgO = stbi_load((oFoldername + "\\" + oFileList[i]).c_str(), &w, &h, &channels, 3);
		imgE = stbi_load((eFoldername + "\\" + eFileList[i]).c_str(), &w, &h, &channels, 3);

		ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decodedBuffer, out);
		decoded.insert(decoded.end(), decodedBuffer.begin(), decodedBuffer.end());

		stbi_image_free(imgO);
		stbi_image_free(imgE);
	}

	WriteBitsToFile(filename, decoded);
}