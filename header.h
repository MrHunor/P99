#pragma once
#include <cstdint>
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

/* =========================================================1. BASIC UTILITIES========================================================= */

void InvalidInputMessage(const string& details = "[No Details]");
bool createFolder(const string& name);
bool checkEx(const string& path);
vector<string> GetFilenamesFromFolder(string path);
void ReccomendActionFilelistMismatch(const vector<string>& FileList1ORIGINAL, const vector<string>& FileList2ORIGINAL, ostream& out);
void CheckFilelists(const vector<string>& FileList1, const vector<string>& FileList2, ostream& out);

/* =========================================================2. BIT / FILE UTILITIES========================================================= */

std::vector<bool> TextToAsciiB(const std::string& s);
string BitsToAscii(const vector<bool>& bits);
void ReadFileToArray(const std::string& filename, std::vector<bool>& array, ostream& out);
void WriteBitsToFile(const std::string& filename, const std::vector<bool>& bits);
std::uintmax_t ReadbSizeFromFile(const std::string& filename);
bool FileIs(const std::string& filename, const std::string& extension);

/* =========================================================3. LOW-LEVEL IMAGE LOGIC========================================================= */

bool WriteToImage(unsigned char* img, size_t imgSize, const vector<bool>& s, ostream& out, int& bitI, int& stringI);
void ReadDataFromImageC(unsigned char* imgC, unsigned char* imgR, int size, int& bitI, int& stringI, vector<bool>& decoded, ostream& out);
string ReadFilenameFromImageC(unsigned char* imgC, unsigned char* imgR, int& bitI, int& stringI, ostream& out);

/* =========================================================4. HIGH-LEVEL IMAGE & FOLDER OPERATIONS========================================================= */

bool EncodeImage(const string& ifilename, const string& ffilename_, ostream& out);
bool DecodeImage(const string& mFilename, const string& ffilename_, ostream& out);
bool EncodeImageFolder(const string& ifoldername, const string& ffilename_, ostream& out);
bool DecodeImageFolder(const string& mFoldername, const string& iFoldername_, ostream& out);

/* =========================================================5. LOW-LEVEL WAV LOGIC========================================================= */

void ReadDataFromWavC(float* mSampleData, float* iSampleData, int& bitI, int stringI, vector<bool>& decoded, ostream& out);
string ReadFilenameFromWavC(vector<short>& mbuffer, vector<short>& ibuffer, int& bitI, int& stringI, ostream& out);
void ReadDataFromWavC(vector<short>& mbuffer, vector<short>& ibuffer, int& bitI, int& stringI, vector<bool>& decoded, ostream& out);

/* =========================================================6. HIGH-LEVEL WAV OPERATIONS =========================================================== */

bool EncodeWav(const string& ifilename, const string& ffilename, ostream& out);
bool DecodeWav(const string& mFilename, const string& iFilename, ostream& out);