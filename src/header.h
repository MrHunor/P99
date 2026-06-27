#pragma once
#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <string>
#include <source_location>
using std::ostream;
using std::string;
using std::vector;

class stateClass
{
public:
    int verbose;

    void out(const string &output, int importance, std::source_location location = std::source_location::current())
     {
        if (importance <= verbose) std::cout << location.function_name()<<"->"  <<  output;
        if(verbose >= 4) 
        {
                    auto now = std::chrono::system_clock::now();
            std::cout<<"@"<<now<<"±10ms";
           
        }
         if (importance <= verbose) std::cout<<std::endl;
    }
};

// A stream buffer that does nothing
class NullBuffer : public std::streambuf
{
public:
    int overflow(int c) override
    {
        return c; // pretend everything was written successfully
    }
};

// A stream that uses the null buffer
class NullStream : public std::ostream
{
public:
    NullStream() : std::ostream(&buffer) {}

private:
    NullBuffer buffer;
};

/* =========================================================1. BASIC UTILITIES========================================================= */
void InvalidInputMessage(const string &details= "[No details provided]",std::source_location location = std::source_location::current());
bool createFolder(const string &name);
bool checkEx(const string &path);
vector<string> GetFilenamesFromFolder(string path);
void ReccomendActionFilelistMismatch(const vector<string> &FileList1ORIGINAL, const vector<string> &FileList2ORIGINAL, stateClass &state);
void CheckFilelists(const vector<string> &FileList1, const vector<string> &FileList2, stateClass &state);

/* =========================================================2. BIT / FILE UTILITIES========================================================= */

std::vector<bool> TextToAsciiB(const std::string &s);
string BitsToAscii(const vector<bool> &bits);
void ReadFileToArray(const std::string &filename, std::vector<bool> &array, stateClass &state);
void WriteBitsToFile(const std::string &filename, const std::vector<bool> &bits);
std::uintmax_t ReadbSizeFromFile(const std::string &filename);
bool FileIs(const std::string &filename, const std::string &extension);

/* =========================================================3. LOW-LEVEL IMAGE LOGIC========================================================= */

bool WriteToImage(unsigned char *img, size_t imgSize, const vector<bool> &s, stateClass &state, int &bitI, int &stringI);
void ReadDataFromImageC(unsigned char *imgC, unsigned char *imgR, int size, int &bitI, int &stringI, vector<bool> &decoded, stateClass &state);
string ReadFilenameFromImageC(unsigned char *imgC, unsigned char *imgR, int &bitI, int &stringI, stateClass &state);

/* =========================================================4. HIGH-LEVEL IMAGE & FOLDER OPERATIONS========================================================= */

bool EncodeImage(const string &ifilename, const string &ffilename_, stateClass &state);
bool DecodeImage(const string &mFilename, const string &ffilename_, stateClass &state);
bool EncodeImageFolder(const string &ifoldername, const string &ffilename_, stateClass &state);
bool DecodeImageFolder(const string &mFoldername, const string &iFoldername_, stateClass &state);

/* =========================================================5. LOW-LEVEL WAV LOGIC========================================================= */

void ReadDataFromWavC(float *mSampleData, float *iSampleData, int &bitI, int stringI, vector<bool> &decoded, stateClass &state);
string ReadFilenameFromWavC(vector<short> &mbuffer, vector<short> &ibuffer, int &bitI, int &stringI, stateClass &state);
void ReadDataFromWavC(vector<short> &mbuffer, vector<short> &ibuffer, int &bitI, int &stringI, vector<bool> &decoded, stateClass &state);

/* =========================================================6. HIGH-LEVEL WAV OPERATIONS =========================================================== */

bool EncodeWav(const string &ifilename, const string &ffilename, stateClass &state);
bool DecodeWav(const string &mFilename, const string &iFilename, stateClass &state);