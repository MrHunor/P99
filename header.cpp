#pragma warning(disable : 4996)
#define endo <<"\n"
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

bool checkEx(const std::string& path)
{
    return fs::exists(path);
}

vector<string> GetFilenamesFromFolder(string path)
{
    vector<string> PathV;
    for (const auto& entry : fs::directory_iterator(path)) {
        PathV.push_back(entry.path().filename().string());
    }
    cout << "Unsorted Vector:\n";
    for (auto e : PathV) { cout << e endo; }
    
    std::sort(PathV.begin(), PathV.end(), [](const std::string& a, const std::string& b) {
        auto getNumber = [](const std::string& s) {
            size_t pos = 0;
            while (pos < s.size() && isdigit(s[pos])) pos++;
            return (pos == 0) ? 0 : std::stoi(s.substr(0, pos));
        };
        return getNumber(a) < getNumber(b);
    });
    
    cout << "Sorted Vector:\n";
    for (auto e : PathV) { cout << e endo; }
    return PathV;
}

void ReccomendActionFilelistMismatch(const vector<string>& FileList1ORIGINAL, const vector<string>& FileList2ORIGINAL, ostream& out)
{
    vector<string> FileList1 = FileList1ORIGINAL;
    vector<string> FileList2 = FileList2ORIGINAL;
    out << "Size Diffrence:\nFilelist1:" << FileList1.size() << "\nFilelist2:" << FileList2.size() endo;
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
    out << "Following images were NOT found:\nFilelist1:";
    for (auto i : FileList1) { out << i endo; }
    out << "Filelist2:";
    for (auto i : FileList2) { out << i endo; }
}

void CheckFilelists(const vector<string>& FileList1, const vector<string>& FileList2, ostream& out)
{
    if (FileList1.size() != FileList2.size())
    {
        ReccomendActionFilelistMismatch(FileList1, FileList2, out);
        InvalidInputMessage("Filelist size mismatch.");
    }
    for (size_t i = 0; i < FileList1.size(); i++)
    {
        out << "Current mapping:Filelist1:" << FileList1[i] << "->" << FileList2[i] endo;
        string modified = FileList1[i];
        modified.insert(modified.length() - 4, 1, 'M');
        if (modified != FileList2[i])
        {
            ReccomendActionFilelistMismatch(FileList1, FileList2, out);
            InvalidInputMessage("A Filelist Item does not match its pair. Details:\nN:" + ts(i) + "\n1:" + FileList1[i] + "\n2:" + FileList2[i]);
        }
    }
}

/* =========================================================2. BIT / FILE UTILITIES========================================================= */

std::vector<bool> TextToAsciiB(const std::string& s)
{
    std::vector<bool> result;
    result.reserve(s.size() * 8);
    for (unsigned char c : s) {
        std::bitset<8> b(c);
        for (int i = 7; i >= 0; --i) result.push_back(b[i]);
    }
    return result;
}

string BitsToAscii(const vector<bool>& bits)
{
    if (bits.size() % 8 != 0) throw std::runtime_error("Bit vector length must be a multiple of 8");
    std::string result;
    result.reserve(bits.size() / 8);
    for (size_t i = 0; i < bits.size(); i += 8) {
        unsigned char value = 0;
        for (int b = 0; b < 8; ++b) {
            value <<= 1;
            value |= bits[i + b] ? 1 : 0;
        }
        result.push_back(static_cast<char>(value));
    }
    return result;
}

void ReadFileToArray(const std::string& filename, std::vector<bool>& array, ostream& out)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Failed to open file");
    file.seekg(0, std::ios::end);
    std::streampos end = file.tellg();
    size_t size = static_cast<size_t>(end);
    file.seekg(0, std::ios::beg);
    vector<char> buffer(size);
    file.read(buffer.data(), size);
    std::vector<bool> header = TextToAsciiB(filename + "|");
    array.clear();
    array.reserve(header.size() + (size * 8));
    array.insert(array.end(), header.begin(), header.end());
    for (unsigned char c : buffer) {
        for (int bit = 7; bit >= 0; --bit) array.push_back((c >> bit) & 1);
    }
}

void WriteBitsToFile(const std::string& filename, const std::vector<bool>& bits)
{
    std::ofstream out(filename, std::ios::binary);
    if (!out) throw std::runtime_error("Failed to open file");
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

std::uintmax_t ReadbSizeFromFile(const std::string& filename) { return std::filesystem::file_size(filename) * 8; }

/* =========================================================3. LOW-LEVEL IMAGE LOGIC========================================================= */

bool WriteToImage(unsigned char* img, size_t imgSize, const vector<bool>& s, ostream& out, int& bitI, int& stringI)
{
    auto sLength = s.size();
    while (stringI < sLength && bitI < imgSize) {
        if (img[bitI] > 0 && img[bitI] < 255) {
            if (s[stringI] == 0) img[bitI]--;
            else img[bitI]++;
            stringI++;
        }
        bitI++;
    }
    return (stringI < sLength);
}

void ReadDataFromImageC(unsigned char* imgC, unsigned char* imgR, int size, int& bitI, int& stringI, vector<bool>& decoded, ostream& out)
{
    bool end = false;
    while (!end && bitI < size) {
        if (imgC[bitI] > 0 && imgC[bitI] < 255) {
            if (imgR[bitI] == imgC[bitI] - 1) decoded.push_back(false);
            else if (imgR[bitI] == imgC[bitI] + 1) decoded.push_back(true);
            else end = true;
            stringI++;
        }
        bitI++;
    }
}

string ReadFilenameFromImageC(unsigned char* imgC, unsigned char* imgR, int& bitI, int& stringI, ostream& out)
{
    bool end = false;
    vector<bool> decoded;
    while (!end && bitI < 100000) {
        if (imgC[bitI] > 0 && imgC[bitI] < 255) {
            if (imgR[bitI] == imgC[bitI] - 1) decoded.push_back(false);
            else if (imgR[bitI] == imgC[bitI] + 1) decoded.push_back(true);
            else end = true;
            stringI++;
        }
        bitI++;
        if (decoded.size() >= 8 && decoded.size() % 8 == 0) {
            if (BitsToAscii(decoded).find('|') != std::string::npos) end = true;
        }
    }
    if (decoded.size() == 0) InvalidInputMessage("Encoded Filename is not readable.");
    string s = BitsToAscii(decoded);
    return s.substr(0, s.find('|'));
}

/* =========================================================4. IMAGE OPERATIONS========================================================= */

bool EncodeImage(const string& filename, ostream& out)
{
    string placeholder{};
    int w{}, h{}, channels{}, bitI{}, stringI{};
    vector<bool> array;
    unsigned char* img = stbi_load(filename.c_str(), &w, &h, &channels, 3);
    if (!img) return 1;
    cout << "Enter the containing Filename:"; cin >> placeholder;
    if (!checkEx(placeholder)) InvalidInputMessage("File '" + placeholder + "' does not exist.");
    ReadFileToArray(placeholder, array, out);
    if (array.size() > (w * h * 3)) {
        InvalidInputMessage("File too big.");
        stbi_image_free(img); return 1;
    }
    WriteToImage(img, w * h * 3, array, out, bitI, stringI);
    stbi_write_png("output.png", w, h, 3, img, 3 * w);
    stbi_image_free(img);
    return 0;
}

bool DecodeImage(const string& eFilename, ostream& out)
{
    string placeholder;
    int w{}, h{}, channels{}, bitI{}, stringI{};
    vector<bool> decoded;
    unsigned char* imgR = stbi_load(eFilename.c_str(), &w, &h, &channels, 3);
    if (!imgR) return 1;
    cout << "Enter Original Filename:"; cin >> placeholder;
    if (!checkEx(placeholder)) InvalidInputMessage("Original file '" + placeholder + "' not found.");
    unsigned char* imgC = stbi_load(placeholder.c_str(), &w, &h, &channels, 3);
    if (!imgC) return 1;
    string Filename = ReadFilenameFromImageC(imgC, imgR, bitI, stringI, out);
    ReadDataFromImageC(imgC, imgR, (w * h * channels), bitI, stringI, decoded, out);
    WriteBitsToFile(Filename, decoded);
    stbi_image_free(imgR); stbi_image_free(imgC);
    return 0;
}

/* =========================================================5. FOLDER OPERATIONS========================================================= */

bool EncodeFolder(const string& ofoldername, ostream& out)
{
    vector<string> FileList = GetFilenamesFromFolder(ofoldername);
    vector<bool> array, aChunk;
    string placeholder{}, fullPath{}, efoldername = ofoldername + "M";
    createFolder(efoldername);
    cout << "Enter Encoding Filename:"; cin >> placeholder;
    if (!checkEx(placeholder)) InvalidInputMessage("Target file '" + placeholder + "' not found.");
    ReadFileToArray(placeholder, array, out);
    int bitcounter = 0, NIL = 0, offset = 0, w, h, channels;
    for(int i = 0; bitcounter < (int)array.size(); i++) {
        if (i >= (int)FileList.size()) InvalidInputMessage("Not enough images in folder.");
        stbi_info((ofoldername + "\\" + FileList[i]).c_str(), &w, &h, &channels);
        bitcounter += (w * h * channels);
        NIL = i + 1;
    }
    for (size_t i = 0; i < NIL; i++) {
        unsigned char* img = stbi_load((ofoldername + "\\" + FileList[i]).c_str(), &w, &h, &channels, 3);
        int capacity = w * h * channels;
        int chunkSize = std::min((int)array.size() - offset, capacity);
        aChunk.assign(array.begin() + offset, array.begin() + chunkSize + offset);
        int bitI = 0, stringI = 0;
        WriteToImage(img, capacity, aChunk, out, bitI, stringI);
        offset += stringI;
        string outPath = efoldername + "\\" + FileList[i];
        outPath.insert(outPath.length() - 4, "M");
        stbi_write_png(outPath.c_str(), w, h, channels, img, channels * w);
        stbi_image_free(img);
    }
    return 0;
}

bool DecodeFolder(const string& eFoldername, const string& oFoldername, ostream& out)
{
    if (!checkEx(oFoldername)) InvalidInputMessage("Original folder '" + oFoldername + "' not found.");
    vector<string> eFileList = GetFilenamesFromFolder(eFoldername);
    vector<string> oFileList = GetFilenamesFromFolder(oFoldername);
    CheckFilelists(oFileList, eFileList, out);
    vector<bool> decoded;
    int w, h, channels, bitI = 0, stringI = 0;
    
    // Process first image for filename
    unsigned char* imgO = stbi_load((oFoldername + "\\" + oFileList[0]).c_str(), &w, &h, &channels, 3);
    unsigned char* imgE = stbi_load((eFoldername + "\\" + eFileList[0]).c_str(), &w, &h, &channels, 3);
    string filename = ReadFilenameFromImageC(imgO, imgE, bitI, stringI, out);
    ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decoded, out);
    stbi_image_free(imgO); stbi_image_free(imgE);

    // Process remaining
    for (size_t i = 1; i < eFileList.size(); i++) {
        imgO = stbi_load((oFoldername + "\\" + oFileList[i]).c_str(), &w, &h, &channels, 3);
        imgE = stbi_load((eFoldername + "\\" + eFileList[i]).c_str(), &w, &h, &channels, 3);
        bitI = 0; stringI = 0;
        ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decoded, out);
        stbi_image_free(imgO); stbi_image_free(imgE);
    }
    WriteBitsToFile(filename, decoded);
    return 0;
}