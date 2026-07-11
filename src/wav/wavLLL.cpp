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
#include "../core/defs.h"
#include "../core/utils.h"

int CheckWavFileCapacityBackend( std::vector<short> buffer, stateClass& state)
{
int counter=0;
int bufferSize=buffer.size();
for(int bitI = 0; bitI<=buffer.size();bitI++)
{
  if (std::abs(buffer[bitI]) < 32767 && buffer[bitI] != 0)counter++;
}
return counter;
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
