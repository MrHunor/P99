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
#include <cmath>
#include <chrono>
#include <iomanip>
#include "defs.h"
#include "utils.h"

//LINK ../../docs/core.md:13
void InvalidInputMessage(const std::string &details, std::source_location location)
{
    std::cout << std::stacktrace::current() << std::endl;
    std::cout << "Filename:" << location.file_name() << std::endl;
    std::cout << "Function:" << location.function_name() << std::endl;
    std::cout << "Line:" << location.line() << std::endl;
    std::cout << "Column:" << location.column() << std::endl;
    if (!details.empty())
    {
        std::cout << "Details provided:" << details << std::endl;
    }
    exit(1);
}

bool createFolder(const std::string &name, stateClass &state)
{
    state.out("FS: Create Dir " + name, 1);
    return !fs::create_directory(name);
}

bool checkEx(const std::string &path, stateClass &state)
{
    state.out("FS: Check Exists " + path, 1);
    return fs::exists(path);
}

std::vector<std::string> GetFilenamesFromFolder(std::string path, stateClass &state)
{
    state.out("Starting... " + path, 4);
    std::vector<std::string> PathV;
    for (const auto &entry : fs::directory_iterator(path))
    {
        PathV.push_back(entry.path().filename().string());
    }
    state.out("Sorting: " + ts(PathV.size()) + " items", 4);
    std::sort(PathV.begin(), PathV.end(), [](const std::string &a, const std::string &b)
              {
            auto getNumber = [](const std::string& s) {
                size_t pos = 0;
                while (pos < s.size() && isdigit(s[pos])) pos++;
                if (pos == 0) return 0;
                return std::stoi(s.substr(0, pos));
            };
            return getNumber(a) < getNumber(b); });
    state.out("Finished", 4);
    return PathV;
}

void ReccomendActionFilelistMismatch(const std::vector<std::string> &FileList1ORIGINAL, const std::vector<std::string> &FileList2ORIGINAL, stateClass &state)
{
    state.out("Starting...", 1);
    std::vector<std::string> FileList1 = FileList1ORIGINAL;
    std::vector<std::string> FileList2 = FileList2ORIGINAL;
    state.out("Size Difference:\nFilelist1:" + ts(FileList1.size()) + "\nFilelist2:" + ts(FileList2.size()), 1);

    state.out("Filter Match", 4);
    for (size_t i = 0; i < FileList1.size(); i++)
    {
        for (size_t z = 0; z < FileList2.size(); z++)
        {
            std::string modified = FileList1[i];
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
    for (const auto &i : FileList1)
    {
        state.out(i, 1);
    }
    state.out("Following images were NOT found:\nFilelist2:", 1);
    for (const auto &i : FileList2)
    {
        state.out(i, 1);
    }
    state.out("Finished", 4);
}

void CheckFilelists(const std::vector<std::string> &FileList1, const std::vector<std::string> &FileList2, stateClass &state)
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
        std::string modified = FileList1[i];
        modified.insert(modified.length() - 4, 1, 'M');
        state.out("Comparing modified:"+modified+"| Original:"+FileList2[i],4);
        if (modified != FileList2[i])
        {
            state.out("Pair Mismatch", 1);
            ReccomendActionFilelistMismatch(FileList1, FileList2, state);
            InvalidInputMessage("A Filelist Item does not match its pair. Details:\nN(Null Initialised):" + ts(i) + "\nName in Filelist 1:" + FileList1[i] + "Name in Filelist 2:" + FileList2[i]);
        }
    }
    state.out("Finished", 4);
}

std::string returnSpaceBytesAsSensefulValue(int value)
{
    float floatvalue = value;
    if (value > 1073741824)
        return ts(floatvalue / 1073741824) + "GB";
    if (value > 1048576)
        return ts(floatvalue / 1048576) + "MB";
    if (value > 1024)
        return ts(floatvalue / 1024) + "KB";
    return ts(floatvalue)+ "Bytes";
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

std::string BitsToAscii(const std::vector<bool> &bits)
{
    if (bits.size() % 8 != 0)
        throw std::runtime_error("Bit std::vector length must be a multiple of 8");

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

void ReadFileToArray(const std::string &filename, std::vector<bool> &array, stateClass &state)
{
    state.out("Starting..", 4);
    std::ifstream file(filename, std::ios::binary);
    if (!file)
        InvalidInputMessage("Failed to open file");

    state.out("Seeking end..", 4);
    file.seekg(0, std::ios::end);
    std::streampos end = file.tellg();
    if (end < 0)
        InvalidInputMessage("tellg failed");

    size_t size = static_cast<size_t>(end);
    file.seekg(0, std::ios::beg);

    state.out("Reading File to Buffer...", 4);
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
        InvalidInputMessage("Failed to read File to Buffer");

    state.out("Preping Metadata...", 4);
    std::vector<bool> header = TextToAsciiB(filename + "|");

    array.clear();
    state.out("Reserving array for Size: " + ts(header.size()) + " + " + ts(size*8), 4);
    array.reserve(header.size() + size*8);
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

void WriteBitsToFile(const std::string &filename, const std::vector<bool> &bits, stateClass &state)
{
    state.out("Writing Bits to file:" + filename, 1);
    std::ofstream out(filename, std::ios::binary);
    if (!out)
    {
      InvalidInputMessage("Failed to open file");
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
