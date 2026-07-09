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
#include "src/core/defs.h"
#include "src/core/utils.h"
void ReadDataFromWavC(float *mSampleData, float *iSampleData, int &bitI, int stringI, std::vector<bool> &decoded, stateClass &state)
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

std::string ReadFilenameFromWavC(std::vector<short> &mbuffer, std::vector<short> &ibuffer, int &bitI, int &stringI, stateClass &state)
{
    state.out("Extract Filename", 1);
    bool end = false;
    std::vector<bool> decoded;
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
    if (decoded.empty())
        InvalidInputMessage("Encoded Filename is not readable.");
    std::string s = BitsToAscii(decoded);
    state.out("Filename Extracted", 4);
    return s.erase(s.size() - 1, 1);
}

void ReadDataFromWavC(std::vector<short> &mbuffer, std::vector<short> &ibuffer, int &bitI, int &stringI, std::vector<bool> &decoded, stateClass &state)
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
