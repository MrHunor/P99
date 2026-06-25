#pragma warning(disable : 4996)
#define endo << "\n"
#define ts(x) std::to_string(x)
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
#include "header.h"
#include <algorithm>
#include <sndfile.h>
namespace fs = std::filesystem;

//test command: main.exe -v -a decode -m testfiles/soundm.wav -i testfiles/sound.wav
using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

/* =========================================================1. BASIC UTILITIES========================================================= */

void InvalidInputMessage(const string &details)
{
    cout << "-> ERR: Invalid Input\n";
    cout << "Your Input was Invalid: " << details;
    exit(1);
}

bool createFolder(const string &name)
{
    cout << "-> FS: Create Dir " << name endo;
    return !fs::create_directory(name);
}

bool checkEx(const std::string &path)
{
    cout << "-> FS: Check Exists " << path endo;
    return fs::exists(path);
}

vector<string> GetFilenamesFromFolder(string path)
{
    cout << "-> Dir: Scan " << path endo;
    vector<string> PathV;
    for (const auto &entry : fs::directory_iterator(path))
    {
        PathV.push_back(entry.path().filename().string());
    }
    cout << "-> Dir: Sort " << PathV.size() << " items" endo;
    std::sort(PathV.begin(), PathV.end(), [](const std::string &a, const std::string &b)
              {
        auto getNumber = [](const std::string& s) {
            size_t pos = 0;
            while (pos < s.size() && isdigit(s[pos])) pos++;
            return std::stoi(s.substr(0, pos));
            };
        return getNumber(a) < getNumber(b); });
    cout << "-> Dir: Done" endo;
    return PathV;
}

void ReccomendActionFilelistMismatch(const vector<string> &FileList1ORIGINAL, const vector<string> &FileList2ORIGINAL, ostream &out)
{
    out << "-> Act: Mismatch Check" endo;
    vector<string> FileList1 = FileList1ORIGINAL;
    vector<string> FileList2 = FileList2ORIGINAL;
    out << "Size Diffrence:\nFilelist1:" << FileList1.size() << "\nFilelist2:" << FileList2.size() endo;
    
    out << "-> Act: Filter Match" endo;
    for (size_t i = 0; i < FileList1.size(); i++)
    {
        for (size_t z = 0; z < FileList2.size(); z++)
        {
            string modified = FileList1[i];
            modified.insert(modified.length() - 4, 1, 'M');
            if (modified == FileList2[z])
            {
                FileList1.erase(FileList1.begin() + i);
                FileList2.erase(FileList2.begin() + z);

                --i;   // re-check new element at i
                break; // stop using z, it's now invalid context
            }
        }
    }
    out << "Following images were NOT found:\nFilelist1:";
    for (auto i : FileList1)
    {
        out << i endo;
    }
    out << "Filelist2:";
    for (auto i : FileList2)
    {
        out << i endo;
    }
    out << "-> Act: Done" endo;
}

void CheckFilelists(const vector<string> &FileList1, const vector<string> &FileList2, ostream &out)
{
    out << "-> Check: Filelists" endo;
    if (FileList1.size() != FileList2.size())
    {
        out << "-> Check: Size Mismatch" endo;
        ReccomendActionFilelistMismatch(FileList1, FileList2, out);
        InvalidInputMessage("Filelist size mismatch.");
    }
    
    out << "-> Check: Loop Items" endo;
    for (size_t i = 0; i < FileList1.size(); i++)
    {
        out << "Current mapping:Filelist1:" << FileList1[i] << "->" << FileList2[i] endo;
        string modified = FileList1[i];
        modified.insert(modified.length() - 4, 1, 'M');
        if (modified != FileList2[i])
        {
            out << "-> Check: Pair Mismatch" endo;
            ReccomendActionFilelistMismatch(FileList1, FileList2, out);
            InvalidInputMessage("A Filelist Item does not match its pair. Details:\nN(Null Initialised):" + ts(i) + "\nName in Filelist 1:" + FileList1[i] + "Name in Filelist 2:" + FileList2[i]);
        }
    }
    out << "-> Check: Done" endo;
}

/* =========================================================2. BIT / FILE UTILITIES========================================================= */

std::vector<bool> TextToAsciiB(const std::string &s)
{
    std::vector<bool> result;
    result.reserve(s.size() * 8);

    for (unsigned char c : s)
    {
        std::bitset<8> b(c);
        for (int i = 7; i >= 0; --i)
        {
            result.push_back(b[i]);
        }
    }
    return result;
}

string BitsToAscii(const vector<bool> &bits)
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

void ReadFileToArray(const std::string &filename, std::vector<bool> &array, ostream &out)
{
    out << "-> File: Read Array" endo;
    std::ifstream file(filename, std::ios::binary);
    if (!file)
        throw std::runtime_error("Failed to open file");

    out << "-> File: Seek End" endo;
    file.seekg(0, std::ios::end);
    std::streampos end = file.tellg();
    if (end < 0)
        throw std::runtime_error("tellg() failed");

    size_t size = static_cast<size_t>(end);
    file.seekg(0, std::ios::beg);

    out << "-> File: Read Buffer" endo;
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
        throw std::runtime_error("Failed to read file");

    out << "-> File: Prep Metadata" endo;
    std::vector<bool> header = TextToAsciiB(filename + "|");

    array.clear();
    out << "Reserving array for Size:" << header.size() + size << endl;
    array.reserve(header.size() + size);
    out << "Reserving finished" endo;
    
    out << "Inserting header..." endo;
    array.insert(array.end(), header.begin(), header.end());
    
    out << "Appending file data..." endo;
    for (unsigned char c : buffer)
    {
        for (int bit = 7; bit >= 0; --bit)
        {
            array.push_back((c >> bit) & 1);
        }
    }
    out << "-> File: Done" endo;
}

void WriteBitsToFile(const std::string &filename, const std::vector<bool> &bits)
{
    cout << "-> File: Write Bits " << filename endo;
    std::ofstream out(filename, std::ios::binary);
    if (!out)
    {
        throw std::runtime_error("Failed to open file");
    }

    std::vector<uint8_t> bytes;
    bytes.reserve((bits.size() + 7) / 8);

    for (size_t i = 0; i < bits.size(); i += 8)
    {
        uint8_t byte = 0;

        for (size_t j = 0; j < 8 && (i + j) < bits.size(); ++j)
        {
            byte <<= 1;
            byte |= bits[i + j] ? 1 : 0;
        }

        bytes.push_back(byte);
    }

    out.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
    cout << "-> File: Done" endo;
}

std::uintmax_t ReadbSizeFromFile(const std::string &filename)
{
    return std::filesystem::file_size(filename) * 8;
}

bool FileIs(const std::string &filename, const std::string &extension)
{
    return std::filesystem::path(filename).extension() == extension;
}

/* =========================================================3. LOW-LEVEL IMAGE LOGIC========================================================= */

bool WriteToImage(unsigned char *img, size_t imgSize, const vector<bool> &s, ostream &out, int &bitI, int &stringI)
{
    out << "-> Img: Write Bytes" endo;
    auto sLength = s.size();
    while (stringI < sLength)
    {
        if (bitI >= imgSize)
        {
            out << "-> Img: Max Size Hit" endo;
            return 1;
        }

        if (img[bitI] > 0 && img[bitI] < 255)
        {
            if (s[stringI] == 0)
            {
                img[bitI]--;
                stringI++;
            }
            else if (s[stringI] == 1)
            {
                img[bitI]++;
                stringI++;
            }
        }
        bitI++;
    }
    out << "-> Img: Write Done" endo;
    return 0;
}

void ReadDataFromImageC(unsigned char *imgC, unsigned char *imgR, int size, int &bitI, int &stringI, vector<bool> &decoded, ostream &out)
{
    out << "-> Img: Read Bytes" endo;
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
    out << "-> Img: Read Done" endo;
}

string ReadFilenameFromImageC(unsigned char *imgC, unsigned char *imgR, int &bitI, int &stringI, ostream &out)
{
    out << "-> Img: Extract Filename" endo;
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

        if (decoded.size() % 8 == 0 && decoded.size() > 0)
        {
            if (BitsToAscii(decoded).find('|') != std::string::npos)
            {
                end = true;
                out << "-> Img: Found '|' delimiter" endo;
            }
        }
    }
    if (decoded.size() == 0)
        InvalidInputMessage("Encoded Filename is not readable.");
    string s = BitsToAscii(decoded);
    out << "-> Img: Filename Extracted" endo;
    return s.erase(s.size() - 1, 1);
}

/* =========================================================4. HIGH-LEVEL IMAGE & FOLDER OPERATIONS========================================================= */

bool EncodeImage(const string &ifilename, const string &ffilename_, ostream &out)
{
    out << "-> EncImg: Init" endo;
    string ffilename = ffilename_;
    int w{}, h{}, channels{}, imgSize{}, bitI{}, stringI{};
    vector<bool> array;
    
    out << "\nLoading image '" << ifilename << "'...\n";
    unsigned char *img = stbi_load(ifilename.c_str(), &w, &h, &channels, 3);
    imgSize = w * h * 3;

    if (!img)
        InvalidInputMessage();
        
    out << "-> EncImg: User Input Check" endo;
    if (ffilename == "")
    {
        cout << "Enter the containing Filename:";
        cin >> ffilename;
    }
    
    out << "-> EncImg: File Check" endo;
    if (!checkEx(ffilename))
        InvalidInputMessage("This file does not exist");
        
    out << "Reserving array for Size:" << ReadbSizeFromFile(ffilename) << endl;
    ReadFileToArray(ffilename, array, out);

    out << "-> EncImg: Capacity Check" endo;
    if (array.size() > imgSize)
    {
        InvalidInputMessage("Given File is too big to encode. Encoding File size=" + std::to_string(array.size()) + "; imageSize" + std::to_string(imgSize));
        stbi_image_free(img);
        return 1;
    }
    
    out << "Writing to Image (Memory)..." endo;
    out << "ARRAY Size:" << array.size() << "bit/" << array.size() / 8 << "Bytes" endo;
    WriteToImage(img, imgSize, array, out, bitI, stringI);
    
    out << "Writing to Image (Disk)..." endo;
    stbi_write_png("output.png", w, h, 3, img, 3 * w);
    
    out << "Freeing Memory..." endo;
    stbi_image_free(img);
    
    out << "-> EncImg: Complete" endo;
    return 0;
}

bool DecodeImage(const string &mFilename, const string &ffilename_, ostream &out)
{
    out << "-> DecImg: Init" endo;
    string ffilename = ffilename_;
    int w{}, h{}, channels{}, bitI{}, stringI{};
    string Filename{};
    vector<bool> decoded;

    out << "-> DecImg: Load Mod Img" endo;
    unsigned char *imgR = stbi_load(mFilename.c_str(), &w, &h, &channels, 3); 
    if (!imgR)
        return 1;
        
    out << "-> DecImg: User Input Check" endo;
    if (ffilename == "")
    {
        cout << "Enter Original Filename:";
        cin >> ffilename;
    }
    if (!checkEx(ffilename))
        InvalidInputMessage("");

    out << "-> DecImg: Load Orig Img" endo;
    unsigned char *imgC = stbi_load(ffilename.c_str(), &w, &h, &channels, 3);
    if (!imgC)
        return 1;

    out << "-> DecImg: Extract Filename" endo;
    Filename = ReadFilenameFromImageC(imgC, imgR, bitI, stringI, out);
    out << "Read Filename:" << Filename endo;
    out << "Channels:" << channels endo;
    
    out << "-> DecImg: Read Bytes" endo;
    ReadDataFromImageC(imgC, imgR, (w * h * channels), bitI, stringI, decoded, out);

    out << "-> DecImg: Write Output" endo;
    WriteBitsToFile(Filename, decoded);

    out << "-> DecImg: Free Mem" endo;
    stbi_image_free(imgR);
    stbi_image_free(imgC);
    
    out << "-> DecImg: Complete" endo;
    return 0;
}

bool EncodeImageFolder(const string &ifoldername, const string &ffilename_, ostream &out)
{
    out << "-> EncFolder: Init" endo;
    vector<string> FileList = GetFilenamesFromFolder(ifoldername);
    vector<bool> array, aChunk;
    string fullPath{}, mfoldername{};
    string ffilename = ffilename_;
    int w{}, inputSize{}, h{}, channels{}, bitcounter{}, NIL{}, bitI{}, stringI{}, offset{};

    mfoldername = ifoldername + "M";
    out << "-> EncFolder: Create M-Dir" endo;
    createFolder(mfoldername);
    
    if (ffilename == "")
    {
        cout << "Enter Encoding Filename:";
        cin >> ffilename;
    }
    if (!checkEx(ffilename))
        InvalidInputMessage("The File you specified does not exist or could not be found");
        
    out << "-> EncFolder: Read File" endo;
    ReadFileToArray(ffilename, array, out);
    inputSize = array.size();
    out << "ARRAY Size:" << array.size() << "bit/" << array.size() / 8 << "Bytes" endo;

    out << "Calculating NIL..." endo; 
    for (int i = 0; bitcounter < inputSize; i++)
    {
        if (i >= FileList.size())
            InvalidInputMessage("Not enough images to store data.\n Bitcounter:" + std::to_string(bitcounter) + ".\ninputSize:" + std::to_string(inputSize));

        fullPath = ifoldername + "\\" + FileList[i];
        stbi_info(fullPath.c_str(), &w, &h, &channels);

        bitcounter += (w * h * channels);
        NIL = i + 1;
    }
    out << "Expected NIL" << NIL << "/" << FileList.size() endo;
    
    out << "-> EncFolder: Loop Files" endo;
    for (size_t i = 0; i < NIL; i++)
    {
        out << "Iteration:" << i << "/" << NIL endo;
        fullPath = ifoldername + "\\" + FileList[i];
        unsigned char *img = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);
        
        out << "Assigning Chunk of Size:" << (w * h * channels) endo;
        int capacity = w * h * channels;
        int chunkSize = std::min((int)array.size() - offset, capacity);
        aChunk.assign(array.begin() + offset, array.begin() + chunkSize + offset);
        
        bitI = 0;
        stringI = 0;
        out << "Writing to Image (Memory)..." endo;
        WriteToImage(img, (w * h * channels), aChunk, out, bitI, stringI);
        out << "Actual Chunk Size:" << stringI endo;
        offset = offset + stringI;
        
        fullPath = mfoldername + "\\" + FileList[i].insert(FileList[i].length() - 4, 1, 'M');
        out << "Writing to Image (Disk)..." endo;
        stbi_write_png(fullPath.c_str(), w, h, channels, img, channels * w);
        
        out << "Freeing Memory..." endo;
        stbi_image_free(img);
    }

    out << "-> EncFolder: Complete" endo;
    return 0;
}

bool DecodeImageFolder(const string &mFoldername, const string &iFoldername_, ostream &out)
{
    out << "-> DecFolder: Init" endo;
    unsigned char *imgO{};
    unsigned char *imgE{};
    int w{}, h{}, channels{}, stringI{}, bitI{};
    string iFoldername = iFoldername_;
    string fullPath{}, filename{};
    vector<string> eFileList{}, oFileList{};
    vector<bool> decoded{}, decodedBuffer{};
    
    if (iFoldername == "")
    {
        cout << "Enter Original Foldername:";
        cin >> iFoldername;
    }
    if (!checkEx(iFoldername))
        InvalidInputMessage("");

    out << "-> DecFolder: Get Lists" endo;
    eFileList = GetFilenamesFromFolder(mFoldername);
    oFileList = GetFilenamesFromFolder(iFoldername);

    CheckFilelists(oFileList, eFileList, out);

    out << "-> DecFolder: Load Primary" endo;
    fullPath = iFoldername + "\\" + oFileList[0];
    imgO = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);

    fullPath = mFoldername + "\\" + eFileList[0];
    imgE = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);

    out << "-> DecFolder: Extract Name" endo;
    filename = ReadFilenameFromImageC(imgO, imgE, bitI, stringI, out);
    out << "Filename:" << filename endo;
    
    out << "Reading remaining Data from the first image..." endo;
    ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decoded, out);
    out << "Freeing memory.." endo;
    stbi_image_free(imgO);
    stbi_image_free(imgE);

    out << "Reading Data from the remaining Images..." endo;
    for (size_t i = 1; i < eFileList.size(); i++)
    {
        out << "Iteration:" << i << "/" << eFileList.size() endo;
        stringI = 0;
        bitI = 0;
        decodedBuffer.clear();

        imgO = stbi_load((iFoldername + "\\" + oFileList[i]).c_str(), &w, &h, &channels, 3);
        imgE = stbi_load((mFoldername + "\\" + eFileList[i]).c_str(), &w, &h, &channels, 3);

        ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decodedBuffer, out);
        decoded.insert(decoded.end(), decodedBuffer.begin(), decodedBuffer.end());

        stbi_image_free(imgO);
        stbi_image_free(imgE);
    }
    
    out << "Size of Read data:" << decoded.size() endo;
    out << "Writing to File...";
    WriteBitsToFile(filename, decoded);
    
    out << "-> DecFolder: Complete" endo;
    return 0;
}

/* =========================================================5. LOW-LEVEL WAV LOGIC========================================================= */

void ReadDataFromWavC(float *mSampleData, float *iSampleData, int &bitI, int stringI, vector<bool> &decoded, ostream &out)
{
    out << "-> WavC: Read F32" endo;
    bool end = false;
    while (!end)
    {
        if (std::abs(iSampleData[bitI]) <= 0.9998f && iSampleData[bitI] != 0)
        {
            if (mSampleData[bitI] == iSampleData[bitI] - 0.0001f)
            {
                decoded.push_back(false);
                stringI++;
            }
            else if (mSampleData[bitI] == iSampleData[bitI] + 0.0001f)
            {
                decoded.push_back(true);
                stringI++;
            }
            else
            {
                end = true;
            }
        }
    }
    out << "-> WavC: F32 Done" endo;
}

string ReadFilenameFromWavC(vector<short> &mbuffer, vector<short> &ibuffer, int& bitI, int& stringI, ostream &out)
{
    out << "-> WavC: Extract Filename" endo;
    bool end = false;
    vector<bool> decoded;
    decoded.reserve(200);

    while (!end)
    {
        if (std::abs(ibuffer[bitI]) < 32767 && ibuffer[bitI] != 0)
        {
            if (mbuffer[bitI] == ibuffer[bitI] - 1)
            {
                decoded.push_back(false);
                stringI++;
            }
            else if (mbuffer[bitI] == ibuffer[bitI] + 1)
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

        if (decoded.size() % 8 == 0 && decoded.size() > 0)
        {
            if (BitsToAscii(decoded).find('|') != std::string::npos)
            {
                end = true;
                out << "-> WavC: Found '|' delimiter" endo;
            }
        }
    }
    if (decoded.size() == 0)
        InvalidInputMessage("Encoded Filename is not readable.");
    string s = BitsToAscii(decoded);
    out << "-> WavC: Filename Extracted" endo;
    return s.erase(s.size() - 1, 1);
}

void ReadDataFromWavC(vector<short>& mbuffer, vector<short>& ibuffer, int &bitI, int& stringI, vector<bool>& decoded, ostream &out)
{
    out << "-> WavC: Read I16 Data" endo;
    bool end = false;
    while (!end)
    {
        if (std::abs(ibuffer[bitI]) < 32767 && ibuffer[bitI] != 0)
        {
            if (mbuffer[bitI] == ibuffer[bitI] - 1)
            {
                decoded.push_back(false);
                stringI++;
            }
            else if (mbuffer[bitI] == ibuffer[bitI] + 1)
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
    out << "-> WavC: I16 Done" endo;
}

/* =========================================================6. HIGH-LEVEL WAV OPERATIONS =========================================================== */

bool EncodeWav(const string &ifilename, const string &ffilename, ostream &out)
{
    out << "-> EncWav: Init" endo;
    string mfilename = ifilename;
    mfilename.insert(mfilename.length() - 4, "M");
    int bitI = 0;
    int stringI = 0;
    sf_count_t totalSamples;
    sf_count_t originalFrames;
    sf_count_t framesRead;
    sf_count_t framesWritten;
    vector<short> buffer;
    vector<bool> fromarray;
    SF_INFO sfinfo;
    sfinfo.format = 0;

    out << "-> EncWav: Open Input" endo;
    SNDFILE *infline = sf_open(ifilename.c_str(), SFM_READ, &sfinfo);
    originalFrames = sfinfo.frames;
    if (!infline)
        InvalidInputMessage("Coudnt open file input file");
        
    out << "Details:" endo;
    out << "Sample Rate:" << sfinfo.samplerate << "Hz" endo;
    out << "Channels   :" << sfinfo.channels endo;
    out << "Frames     :" << sfinfo.frames endo;
    
    totalSamples = sfinfo.frames * sfinfo.channels;
    out << "-> EncWav: Resize Buffer" endo;
    buffer.resize(totalSamples);
    
    out << "-> EncWav: Read SF" endo;
    framesRead = sf_readf_short(infline, buffer.data(), sfinfo.frames);
    if (framesRead != sfinfo.frames)
        InvalidInputMessage("Read Frame Count does not match expected Frame Count\nRead Frame Count" + ts(framesRead) + "\nExpected:" + ts(sfinfo.frames));
    sf_close(infline);
    
    out << "-> EncWav: Read Payload" endo;
    ReadFileToArray(ffilename, fromarray, out);
    if (fromarray.size() >= totalSamples)
        InvalidInputMessage("\nThe File specified does not contain enough space to encode\nCapacity:" + ts(totalSamples) + "\nFilesize:" + ts(fromarray.size()));
        
    out << "Writing... (memory)" endo;
    while (bitI < totalSamples && stringI < fromarray.size())
    {
        if (std::abs(buffer[bitI]) < 32767 && buffer[bitI] != 0)
        {
            if (fromarray[stringI] == false)
            {
                buffer[bitI] = buffer[bitI] - 1;
            }
            else
            {
                buffer[bitI] = buffer[bitI] + 1;
            }
            stringI++;
        }
        bitI++;
    }
    
    out << "-> EncWav: Open Output" endo;
    SNDFILE *outfile = sf_open(mfilename.c_str(), SFM_WRITE, &sfinfo);
    if (!outfile)
        InvalidInputMessage("Coudnt open output File");
        
    out << "Wrtining ... (disk)" endo;
    framesWritten = sf_writef_short(outfile, buffer.data(), originalFrames);
    if (framesWritten != originalFrames)
        InvalidInputMessage("Written Frame Count does not match expected Frame Count");
    sf_close(outfile);
    
    out << "-> EncWav: Complete" endo;
    return 0;
}

bool DecodeWav(const string &mFilename, const string &iFilename, ostream &out)
{
    out << "-> DecWav: Init" endo;
    int bitI = 0;
    int stringI = 0;
    string ffilename;
    vector<short> mbuffer;
    vector<short> ibuffer;
    vector<bool> decoded;
    SF_INFO msfInfo;
    SF_INFO isfInfo;
    
    out << "-> DecWav: Open Mod" endo;
    SNDFILE *mfile = sf_open(mFilename.c_str(), SFM_READ, &msfInfo);
    out << "-> DecWav: Open Orig" endo;
    SNDFILE *ifile = sf_open(iFilename.c_str(), SFM_READ, &isfInfo);
    if (!mfile || !ifile) InvalidInputMessage("Coudnt Open File");
    
    out << "-> DecWav: Resize Buffers" endo;
    mbuffer.resize(msfInfo.channels * msfInfo.frames);
    ibuffer.resize(isfInfo.channels * isfInfo.frames);
    
    out << "-> DecWav: Read SF" endo;
    sf_readf_short(mfile, mbuffer.data(), msfInfo.frames);
    sf_readf_short(ifile, ibuffer.data(), isfInfo.frames);
    if (mbuffer.empty() || ibuffer.empty()) InvalidInputMessage("Coudnt read Files to Memory");
    out << "Read Files successfully into Memory" endo;
    
    out << "-> DecWav: Validate Sizes" endo;
    if (msfInfo.frames * msfInfo.channels != isfInfo.frames * isfInfo.channels)
        InvalidInputMessage("Sample amounts do not match, indicating file corruption or wrong file selection");
    out << "Sample sizes match:\nmSamplesize:" << msfInfo.frames * msfInfo.channels << "\niSampleSize:" << isfInfo.frames * isfInfo.channels endo;

    out << "-> DecWav: Find Non-Zero" endo;
    for (; bitI < mbuffer.size(); ++bitI) {
        if (mbuffer[bitI] != 0) {
            break; 
        }
    }
    out << "First non zero Sample at:" << bitI endo;
    
    out << "First ten samples:" endo;
    for (size_t i = bitI; i < 10 + bitI; i++)
    {
        out << "n:" << i << " | mbuffer:" << mbuffer[i] << " | ibuffer:" << ibuffer[i] endo;
    }
    
    out << "-> DecWav: Read Filename" endo;
    ffilename = ReadFilenameFromWavC(mbuffer, ibuffer, bitI, stringI, out);
    out << "Decoded filename:" << ffilename endo;
    
    out << "-> DecWav: Read Data" endo;
    ReadDataFromWavC(mbuffer, ibuffer, bitI, stringI, decoded, out);
    
    out << "Writing to file" endo;
    WriteBitsToFile(ffilename, decoded);
    
    out << "-> DecWav: Complete" endo;
    return 0;
}