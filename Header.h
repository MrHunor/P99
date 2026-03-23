#pragma once
#include <iostream>
using namespace std;
string TextToAsciiB(string s);
extern std::string BitsToAscii(const std::vector<bool>& bits);
extern string ReadFileToBString(const string& filename);
extern bool WriteToImage(unsigned char* img, size_t imgSize, const string& s, bool verbose, int& bitI, int& stringI);
extern void WriteBitsToFile(const std::string& filename, const std::vector<bool>& bits);
extern string ReadFilenameFromImageC(unsigned char* imgC, unsigned char* imgR, int& bitI, int& stringI);
extern void ReadDataFromImageC(unsigned char* imgC, unsigned char* imgR, int& bitI, int& stringI, vector<bool>& decoded);
extern vector<string> GetFilenamesFromFolder(string path);
extern bool EncodeFolder(const string& foldername);
extern bool EncodeImage(const string& filename);
extern bool DecodeImage(const string& eFilename);