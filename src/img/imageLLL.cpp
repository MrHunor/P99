#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <iostream>
#include <string>
#include <bitset>
#include <vector>
#include <fstream>
#include <filesystem>

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
#include "imageLLL.h"
size_t checkImageCapacityBackend(unsigned char *img, size_t imgSize, stateClass &state)
{
    state.out("Starting...", 4);
    size_t counter = 0;
    int channelCounter=1;//carful this is 1 initialised not 0
    for (size_t i = 0; i < imgSize; i++)
    {
        if (img[i] > 0 && img[i] < 255&&channelCounter != 4)
        {
            counter++;
        }
        if(channelCounter == 4) channelCounter = 0;
        channelCounter++;
        
    }
    state.out("Capacity:"+ts(counter),4);
    state.out("Finished....", 4);
    return counter;
}

bool WriteToImage(unsigned char *img, size_t capacity, const std::vector<bool> &s, stateClass &state, int &bitI, int &stringI)
{
    state.out("Starting...", 4);
    auto sLength = s.size();
    state.out("Starting loop...", 4);
    int channelCounter=1;//careful this is 1 initialised not 0
    while (stringI < sLength)
    {
        // state.out("\nimgSize:"+ts(imgSize)+"\nbitI:"+ts(bitI)+"\nstringI"+ts(stringI)+"\n",4);
        if (bitI >= capacity)
        {
            state.out("Image Capacity overflow\ncapacity:" + ts(capacity) + "\nBitI:" + ts(bitI) + "\nstringI:" + ts(stringI), 4);
            return 0;
        }
        if (img[bitI] > 0 && img[bitI] < 255 &&channelCounter != 4)
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
        if(channelCounter == 4) channelCounter = 0;
        bitI++;
        channelCounter++;
    }
    state.out("Finished", 4);
    return 0;
}

void ReadDataFromImageC(unsigned char *imgC, unsigned char *imgR, int size, int &bitI, int &stringI, std::vector<bool> &decoded, stateClass &state)
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
