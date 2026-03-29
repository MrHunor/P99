#pragma once
#include <iostream>
#include <vector>

using std::string;
using std::vector;
using std::ostream;

// A stream buffer that does nothing
class NullBuffer : public std::streambuf {
public:
	int overflow(int c) override {
		return c; // pretend everything was written successfully
	}
};

// A stream that uses the null buffer
class NullStream : public std::ostream {
public:
	NullStream() : std::ostream(&buffer) {}

private:
	NullBuffer buffer;
};

//Functions in header.cpp
string TextToAsciiB(string s);
extern std::string BitsToAscii(const std::vector<bool>& bits);
extern string ReadFileToBString(const string& filename);
extern bool WriteToImage(unsigned char* img, size_t imgSize, const string& s, ostream& out, int& bitI, int& stringI);
extern void WriteBitsToFile(const std::string& filename, const std::vector<bool>& bits);
extern string ReadFilenameFromImageC(unsigned char* imgC, unsigned char* imgR, int& bitI, int& stringI, ostream& out);
extern void ReadDataFromImageC(unsigned char* imgC, unsigned char* imgR, int size, int& bitI, int& stringI, vector<bool>& decoded, ostream& out);
extern vector<string> GetFilenamesFromFolder(string path);
extern bool EncodeFolder(const string& foldername, ostream& out);
extern bool EncodeImage(const string& filename, ostream& out);
extern bool DecodeImage(const string& eFilename, ostream& out);
extern bool DecodeFolder(const string& eFoldername, ostream& out);
extern bool createFolder(const string& name);
extern bool checkEx(const string& path);
extern void invalidInputMessage(const string& details);