#pragma once
#include <iostream>
#include <vector>
#include <string>

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
extern string TextToAsciiB(const string& s);
extern string BitsToAscii(const std::vector<bool>& bits);
extern void ReadFileToArray(const std::string& filename, bool array[], size_t maxSize);
extern bool WriteToImage(unsigned char* img, size_t imgSize, const vector<bool>& s, ostream& out, int& bitI, int& stringI);
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
extern void InvalidInputMessage(const string& details = "[No Details]");
extern void EncodeImageNI(const string& ifilename, const string& cfilename, ostream& out);
extern void DecodeImageNI(const string& oFilename, const string& eFilename, ostream& out);
extern void EncodeFolderNI(const string& ofoldername, const string& cfilename, ostream& out);
extern void DecodeFolder(const string& oFoldername, const string& eFoldername, ostream& out);
extern int ReadbSizeFromFile(const string& filename);