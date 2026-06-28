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
#include <algorithm>
#include <sndfile.h>
#include <stacktrace>
#include <source_location>
#include <chrono>
#include "header.h"
namespace fs = std::filesystem;

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;



/* =========================================================1. BASIC UTILITIES========================================================= */

void InvalidInputMessage(const string &details, std::source_location location)
{
    cout << std::stacktrace::current() << endl;
    cout << "Filename:" << location.file_name() << endl;
    cout << "Function:" << location.function_name() << endl;
    cout << "Line:" << location.line() << endl;
    cout << "Column:" << location.column() << endl;
    if (!details.empty()) 
    {
        cout << "Details provided:" << details << endl;
    }
    exit(1);
}

bool createFolder(const string &name, stateClass& state)
{
    state.out("FS: Create Dir " + name, 1);
    return !fs::create_directory(name);
}

bool checkEx(const std::string &path, stateClass& state)
{
    state.out("FS: Check Exists " + path, 1);
    return fs::exists(path);
}

vector<string> GetFilenamesFromFolder(string path, stateClass& state)
{
    state.out("Dir: Scan " + path, 1);
    vector<string> PathV;
    for (const auto &entry : fs::directory_iterator(path))
    {
        PathV.push_back(entry.path().filename().string());
    }
    state.out("Dir: Sort " + ts(PathV.size()) + " items", 1);
    std::sort(PathV.begin(), PathV.end(), [](const std::string &a, const std::string &b)
        {
            auto getNumber = [](const std::string& s) {
                size_t pos = 0;
                while (pos < s.size() && isdigit(s[pos])) pos++;
                if (pos == 0) return 0;
                return std::stoi(s.substr(0, pos));
            };
            return getNumber(a) < getNumber(b); 
        });
    state.out("Dir: Done", 1);
    return PathV;
}

void ReccomendActionFilelistMismatch(const vector<string> &FileList1ORIGINAL, const vector<string> &FileList2ORIGINAL, stateClass& state)
{
    state.out("Starting...", 1);
    vector<string> FileList1 = FileList1ORIGINAL;
    vector<string> FileList2 = FileList2ORIGINAL;
    state.out("Size Difference:\nFilelist1:" + ts(FileList1.size()) + "\nFilelist2:" + ts(FileList2.size()), 1);
    
    state.out("Filter Match", 4);
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
                --i;  
                break; 
            }
        }
    }
    state.out("Following images were NOT found:\nFilelist1:", 1);
    for (const auto& i : FileList1)
    {
       state.out(i, 1);  
    }
    state.out("Following images were NOT found:\nFilelist2:", 1);
    for (const auto& i : FileList2)
    {
      state.out(i, 1); 
    }
    state.out("Finished", 4);
}

void CheckFilelists(const vector<string> &FileList1, const vector<string> &FileList2, stateClass& state)
{
    state.out("Starting", 4);
    if (FileList1.size() != FileList2.size())
    {
        state.out("Size Mismatch", 1);
        ReccomendActionFilelistMismatch(FileList1, FileList2, state);
        InvalidInputMessage("Filelist size mismatch.");
    }
    
    state.out("Looping Items", 4);
    for (size_t i = 0; i < FileList1.size(); i++)
    {
        state.out("Current mapping:Filelist1:" + FileList1[i] + "->" + FileList2[i], 4);
        string modified = FileList1[i];
        modified.insert(modified.length() - 4, 1, 'M');
        if (modified != FileList2[i])
        {
            state.out("Pair Mismatch", 1);
            ReccomendActionFilelistMismatch(FileList1, FileList2, state);
            InvalidInputMessage("A Filelist Item does not match its pair. Details:\nN(Null Initialised):" + ts(i) + "\nName in Filelist 1:" + FileList1[i] + "Name in Filelist 2:" + FileList2[i]);
        }
    }
    state.out("Finished", 4);
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

void ReadFileToArray(const std::string &filename, std::vector<bool> &array, stateClass& state)
{
    state.out("Starting..", 4);
    std::ifstream file(filename, std::ios::binary);
    if (!file) InvalidInputMessage("Failed to open file");

    state.out("Seeking end..", 4);
    file.seekg(0, std::ios::end);
    std::streampos end = file.tellg();
    if (end < 0) InvalidInputMessage("tellg failed");

    size_t size = static_cast<size_t>(end);
    file.seekg(0, std::ios::beg);

    state.out("Reading File to Buffer...", 4);
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) InvalidInputMessage("Failed to read File to Buffer");

    state.out("Preping Metadata...", 4);
    std::vector<bool> header = TextToAsciiB(filename + "|");

    array.clear();
    state.out("Reserving array for Size: " + ts(header.size()) + " + " + ts(size), 4);
    array.reserve(header.size() + size);
    state.out("Reserving finished", 4);
    
    state.out("Inserting header...", 4);
    array.insert(array.end(), header.begin(), header.end());
    
    state.out("Appending file data...", 4);
    for (unsigned char c : buffer)
    {
        for (int bit = 7; bit >= 0; --bit)
        {
            array.push_back((c >> bit) & 1);
        }
    }
    state.out("Done", 4);
}

void WriteBitsToFile(const std::string &filename, const std::vector<bool> &bits, stateClass& state)
{
    state.out("File: Write Bits " + filename, 1);
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
    state.out("File: Done", 1);
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

bool WriteToImage(unsigned char *img, size_t imgSize, const vector<bool> &s, stateClass& state, int &bitI, int &stringI)
{
    state.out("Starting...", 4);
    auto sLength = s.size();
    state.out("Starting loop...",4);
    while (stringI < sLength)
    {
   // state.out("\nimgSize:"+ts(imgSize)+"\nbitI:"+ts(bitI)+"\nstringI"+ts(stringI)+"\n",4);
        if (bitI >= imgSize)
        {
            state.out("Image Capacity overflow\nimgSize:"+ts(imgSize)+"\nBitI:"+ts(bitI)+"\nstringI:"+ts(stringI)+"\nMeaning that there were only "+ts(stringI)+"/"+ts(bitI)+" bits of the image written too",4);
            return 0;
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
    state.out("Finished", 4);
    return 0;
}

void ReadDataFromImageC(unsigned char *imgC, unsigned char *imgR, int size, int &bitI, int &stringI, vector<bool> &decoded, stateClass& state)
{
    state.out("Starting...", 4);
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
    state.out("Finished", 4);
}

string ReadFilenameFromImageC(unsigned char *imgC, unsigned char *imgR, int &bitI, int &stringI, stateClass& state)
{
    state.out("Starting...", 4);
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

        if (decoded.size() % 8 == 0 && !decoded.empty())
        {
            if (BitsToAscii(decoded).find('|') != std::string::npos)
            {
                end = true;
                state.out("Found '|' delimiter", 4);
            }
        }
    }
    if (decoded.empty()) InvalidInputMessage("Encoded Filename is not readable.");
    string s = BitsToAscii(decoded);
    state.out("Filename Extracted", 4);
    return s.erase(s.size() - 1, 1);
}

/* =========================================================4. HIGH-LEVEL IMAGE & FOLDER OPERATIONS========================================================= */

bool EncodeImage(const string &ifilename, const string &ffilename_, stateClass& state)
{
   state.out("Starting...", 4);
   string ffilename = ffilename_;
   int w{}, h{}, channels{}, imgSize{}, bitI{}, stringI{};
   vector<bool> array;
    
   state.out("Loading image '" + ifilename + "...", 1);
   unsigned char *img = stbi_load(ifilename.c_str(), &w, &h, &channels,0);
   imgSize = w * h * 3;

   if (!img) InvalidInputMessage("Failed to load image context.");
        
   state.out("User Input Check", 4);
   if (ffilename == "")
   {
       cout << "Enter the containing Filename:";
       cin >> ffilename;
   }
    
   ReadFileToArray(ffilename, array, state);

   state.out("Checking capacity", 4);
   if (array.size() > imgSize)
   {
       InvalidInputMessage("Given File is too big to encode. Encoding File size=" + std::to_string(array.size()) + "; imageSize" + std::to_string(imgSize));
       stbi_image_free(img);
       return 1;
   }
    
   state.out("Writing to Image (Memory)...", 1);
   state.out("ARRAY Size:" + ts(array.size()) + "bit/" + ts(array.size() / 8) + "Bytes", 4);
   WriteToImage(img, imgSize, array, state, bitI, stringI);
    
   state.out("Writing to Image (Disk)...", 1);
   stbi_write_png("output.png", w, h, 3, img, 3 * w);
    
   state.out("Freeing Memory...", 4);
   stbi_image_free(img);
    
   state.out("Finished", 4);
   return 0;
}

bool DecodeImage(const string &mFilename, const string &ffilename_, stateClass& state)
{
    state.out("Starting...", 1);
    string ffilename = ffilename_;
    int w{}, h{}, channels{}, bitI{}, stringI{};
    string Filename{};
    vector<bool> decoded;

    state.out("Load Mod Img", 1);
    unsigned char *imgR = stbi_load(mFilename.c_str(), &w, &h, &channels, 0); 
    if (!imgR) return 1;
        
    state.out("User Input Check", 4);
    if (ffilename == "")
    {
        cout << "Enter Original Filename:";
        cin >> ffilename;
    }
    if (!checkEx(ffilename, state)) InvalidInputMessage("Target missing.");

    state.out("Load Orig Img", 1);
    unsigned char *imgC = stbi_load(ffilename.c_str(), &w, &h, &channels, 3);
    if (!imgC) return 1;

    state.out("Extract Filename", 1);
    Filename = ReadFilenameFromImageC(imgC, imgR, bitI, stringI, state);
    state.out("Read Filename:" + Filename, 1);
    state.out("Channels:" + std::to_string(channels), 4);
    
    state.out("Read Bytes", 1);
    ReadDataFromImageC(imgC, imgR, (w * h * channels), bitI, stringI, decoded, state);

    state.out("Write Output", 1);
    WriteBitsToFile(Filename, decoded, state);

    state.out("Free Mem", 4);
    stbi_image_free(imgR);
    stbi_image_free(imgC);
    
    state.out("Complete", 1);
    return 0;
}

bool EncodeImageFolder(const string &ifoldername, const string &ffilename_, stateClass& state)
{
    state.out("Init", 1);
    vector<string> FileList = GetFilenamesFromFolder(ifoldername, state);
    vector<bool> array, aChunk;
    string fullPath{}, mfoldername{};
    string ffilename = ffilename_;
    int w{}, inputSize{}, h{}, channels{}, bitcounter{}, NIL{}, bitI{}, stringI{}, offset{};

    mfoldername = ifoldername + "M";
    state.out("Create M-Dir", 1);
    createFolder(mfoldername, state);
    
    if (ffilename == "")
    {
        cout << "Enter Encoding Filename:";
        cin >> ffilename;
    }
    if (!checkEx(ffilename, state)) InvalidInputMessage("The File you specified does not exist or could not be found");
        
    state.out("Read File", 1);
    ReadFileToArray(ffilename, array, state);
    inputSize = array.size();
    state.out("ARRAY Size:" + std::to_string(array.size()) + "bit/" + std::to_string(array.size() / 8) + "Bytes", 4);

    state.out("Calculating NIL...", 1); 
    for (int i = 0; bitcounter < inputSize; i++)
    {
        if (i >= FileList.size()) InvalidInputMessage("Not enough images to store data.\n Bitcounter:" + std::to_string(bitcounter) + ".\ninputSize:" + std::to_string(inputSize));

        fullPath = ifoldername + "\\" + FileList[i];
        stbi_info(fullPath.c_str(), &w, &h, &channels);

        bitcounter += (w * h * channels);
        NIL = i + 1;
    }
    state.out("Expected NIL" + std::to_string(NIL) + "/" + std::to_string(FileList.size()), 4);
    
    state.out("Loop Files", 1);
    for (size_t i = 0; i < NIL; i++)
    {
        state.out("Iteration:" + std::to_string(i) + "/" + std::to_string(NIL), 4);
        fullPath = ifoldername + "\\" + FileList[i];
        unsigned char *img = stbi_load(fullPath.c_str(), &w, &h, &channels, 0);
        
        state.out("Assigning Chunk of Size:" + std::to_string(w * h * channels), 4);
        int capacity = w * h * channels;
        int chunkSize = std::min((int)array.size() - offset, capacity);
        aChunk.assign(array.begin() + offset, array.begin() + chunkSize + offset);
        
        bitI = 0;
        stringI = 0;
        state.out("Writing to Image (Memory)...", 1);
        WriteToImage(img, (w * h * channels), aChunk, state, bitI, stringI);
        state.out("Actual Chunk Size:" + std::to_string(stringI), 4);
        offset = offset + stringI;
        
        fullPath = mfoldername + "\\" + FileList[i].insert(FileList[i].length() - 4, 1, 'M');
        state.out("Writing to Image (Disk)...", 1);
        stbi_write_png(fullPath.c_str(), w, h, channels, img, channels * w);
        
        state.out("Freeing Memory...", 4);
        stbi_image_free(img);
    }
    if(state.deleteOverflow)
    {
    for(int i = NIL; i< FileList.size(); i++)
    {
    remove(FileList[i].c_str());
    }

    }
    state.out("Complete", 1);
    return 0;
}

bool DecodeImageFolder(const string &mFoldername, const string &iFoldername_, stateClass& state)
{
    state.out("Init", 1);
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
    if (!checkEx(iFoldername, state)) InvalidInputMessage("Original source folder missing.");

    state.out("Get Lists", 1);
    eFileList = GetFilenamesFromFolder(mFoldername, state);
    oFileList = GetFilenamesFromFolder(iFoldername, state);

    CheckFilelists(oFileList, eFileList, state);

    state.out("Load Primary", 1);
    fullPath = iFoldername + "\\" + oFileList[0];
    imgO = stbi_load(fullPath.c_str(), &w, &h, &channels, 0);

    fullPath = mFoldername + "\\" + eFileList[0];
    imgE = stbi_load(fullPath.c_str(), &w, &h, &channels, 0);

    state.out("Extract Name", 1);
    filename = ReadFilenameFromImageC(imgO, imgE, bitI, stringI, state);
    state.out("Filename:" + filename, 1);
    
    state.out("Reading remaining Data from the first image...", 4);
    ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decoded, state);
    state.out("Freeing memory..", 4);
    stbi_image_free(imgO);
    stbi_image_free(imgE);

    state.out("Reading Data from the remaining Images...", 4);
    for (size_t i = 1; i < eFileList.size(); i++)
    {
        state.out("Iteration:" + std::to_string(i) + "/" + std::to_string(eFileList.size()), 4);
        stringI = 0;
        bitI = 0;
        decodedBuffer.clear();

        imgO = stbi_load((iFoldername + "\\" + oFileList[i]).c_str(), &w, &h, &channels, 3);
        imgE = stbi_load((mFoldername + "\\" + eFileList[i]).c_str(), &w, &h, &channels, 3);

        ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decodedBuffer, state);
        decoded.insert(decoded.end(), decodedBuffer.begin(), decodedBuffer.end());

        stbi_image_free(imgO);
        stbi_image_free(imgE);
    }
    
    state.out("Size of Read data:" + std::to_string(decoded.size()), 4);
    state.out("Writing to File...", 1);
    WriteBitsToFile(filename, decoded, state);
    
    state.out("Complete", 1);
    return 0;
}

/* =========================================================5. LOW-LEVEL WAV LOGIC========================================================= */

void ReadDataFromWavC(float *mSampleData, float *iSampleData, int &bitI, int stringI, vector<bool> &decoded, stateClass& state)
{
    state.out("Read F32", 1);
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
    state.out("F32 Done", 4);
}

string ReadFilenameFromWavC(vector<short> &mbuffer, vector<short> &ibuffer, int& bitI, int& stringI, stateClass& state)
{
    state.out("Extract Filename", 1);
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

        if (decoded.size() % 8 == 0 && !decoded.empty())
        {
            if (BitsToAscii(decoded).find('|') != std::string::npos)
            {
                end = true;
                state.out("Found '|' delimiter", 4);
            }
        }
    }
    if (decoded.empty()) InvalidInputMessage("Encoded Filename is not readable.");
    string s = BitsToAscii(decoded);
    state.out("Filename Extracted", 4);
    return s.erase(s.size() - 1, 1);
}

void ReadDataFromWavC(vector<short>& mbuffer, vector<short>& ibuffer, int &bitI, int& stringI, vector<bool>& decoded, stateClass& state)
{
    state.out("Read I16 Data", 1);
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
    state.out("I16 Done", 4);
}

/* =========================================================6. HIGH-LEVEL WAV OPERATIONS =========================================================== */

bool EncodeWav(const string &ifilename, const string &ffilename, stateClass& state)
{
    state.out("Init", 1);
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

    state.out("Open Input", 1);
    SNDFILE *infline = sf_open(ifilename.c_str(), SFM_READ, &sfinfo);
    originalFrames = sfinfo.frames;
    if (!infline) InvalidInputMessage("Could not open input file");
        
    state.out("Details:", 4);
    state.out("Sample Rate:" + std::to_string(sfinfo.samplerate) + "Hz", 4);
    state.out("Channels   :" + std::to_string(sfinfo.channels), 4);
    state.out("Frames     :" + std::to_string(sfinfo.frames), 4);
    
    totalSamples = sfinfo.frames * sfinfo.channels;
    state.out("Resize Buffer", 4);
    buffer.resize(totalSamples);
    
    state.out("Read SF", 1);
    framesRead = sf_readf_short(infline, buffer.data(), sfinfo.frames);
    if (framesRead != sfinfo.frames) InvalidInputMessage("Read Frame Count does not match expected Frame Count\nRead Frame Count" + ts(framesRead) + "\nExpected:" + ts(sfinfo.frames));
    sf_close(infline);
    
    state.out("Read Payload", 1);
    ReadFileToArray(ffilename, fromarray, state);
    if (fromarray.size() >= totalSamples) InvalidInputMessage("\nThe File specified does not contain enough space to encode\nCapacity:" + ts(totalSamples) + "\nFilesize:" + ts(fromarray.size()));
        
    state.out("Writing... (memory)", 1);
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
    
    state.out("Open Output", 1);
    SNDFILE *outfile = sf_open(mfilename.c_str(), SFM_WRITE, &sfinfo);
    if (!outfile) InvalidInputMessage("Could not open output File");
        
    state.out("Writing ... (disk)", 1);
    framesWritten = sf_writef_short(outfile, buffer.data(), originalFrames);
    if (framesWritten != originalFrames) InvalidInputMessage("Written Frame Count does not match expected Frame Count");
    sf_close(outfile);
    
    state.out("Complete", 1);
    return 0;
}

bool DecodeWav(const string &mFilename, const string &iFilename, stateClass& state)
{
    state.out("Init", 1);
    int bitI = 0;
    int stringI = 0;
    string ffilename;
    vector<short> mbuffer;
    vector<short> ibuffer;
    vector<bool> decoded;
    SF_INFO msfInfo;
    SF_INFO isfInfo;
    
    state.out("Open Mod", 1);
    SNDFILE *mfile = sf_open(mFilename.c_str(), SFM_READ, &msfInfo);
    state.out("Open Orig", 1);
    SNDFILE *ifile = sf_open(iFilename.c_str(), SFM_READ, &isfInfo);
    if (!mfile || !ifile) InvalidInputMessage("Could not open files.");
    
    state.out("Resize Buffers", 4);
    mbuffer.resize(msfInfo.channels * msfInfo.frames);
    ibuffer.resize(isfInfo.channels * isfInfo.frames);
    
    state.out("Read SF", 1);
    sf_readf_short(mfile, mbuffer.data(), msfInfo.frames);
    sf_readf_short(ifile, ibuffer.data(), isfInfo.frames);
    if (mbuffer.empty() || ibuffer.empty()) InvalidInputMessage("Could not read Files to Memory");
    state.out("Read Files successfully into Memory", 1);
    
    state.out("Validate Sizes", 1);
    if (msfInfo.frames * msfInfo.channels != isfInfo.frames * isfInfo.channels) InvalidInputMessage("Sample amounts do not match, indicating file corruption or wrong file selection");
    state.out("Sample sizes match:\nmSamplesize:" + std::to_string(msfInfo.frames * msfInfo.channels) + "\niSampleSize:" + std::to_string(isfInfo.frames * isfInfo.channels), 4);

    state.out("Find Non-Zero", 4);
    for (; bitI < mbuffer.size(); ++bitI) {
        if (mbuffer[bitI] != 0) {
            break; 
        }
    }
    state.out("First non zero Sample at:" + std::to_string(bitI), 4);
    
    state.out("First ten samples:", 4);
    for (size_t i = bitI; i < 10 + bitI; i++)
    {
        state.out("n:" + std::to_string(i) + " | mbuffer:" + std::to_string(mbuffer[i]) + " | ibuffer:" + std::to_string(ibuffer[i]), 4);
    }
    
    state.out("Read Filename", 1);
    ffilename = ReadFilenameFromWavC(mbuffer, ibuffer, bitI, stringI, state);
    state.out("Decoded filename:" + ffilename, 1);
    
    state.out("Read Data", 1);
    ReadDataFromWavC(mbuffer, ibuffer, bitI, stringI, decoded, state);
    
    state.out("Writing to file", 1);
    WriteBitsToFile(ffilename, decoded, state);
    
    state.out("Complete", 1);
    return 0;
}