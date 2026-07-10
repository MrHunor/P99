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
#include "../img/imageLLL.h"
#include "imageHLL.h"
// You might ask yourself why this function is "MidEnd" well thats because it returns a function (which is caracteristic of backend functions of this type) but also prints the size (which is caracteristic of frontend functions), is this stupid? yes. do i have time to change this now? no. Have fun dealing with this, future me
int checkImageFolderCapacityMidEnd(const std::string &ifoldername, stateClass &state)
{

    std::vector<std::string> ifolderList = GetFilenamesFromFolder(ifoldername, state);
    int w = 0;
    int h = 0;
    int channels = 0;
    std::string fullPath;
    int imgSize;
    uint64_t counter = 0;
    int diff;
    for (size_t i = 0; i < ifolderList.size(); i++)
    {
        state.out("Iteration:" + std::to_string(i) + "/" + std::to_string(ifolderList.size() - 1), 4);
        fullPath = ifoldername + "\\" + ifolderList[i];
        unsigned char *img = stbi_load(fullPath.c_str(), &w, &h, &channels, 0);
        imgSize = w * h * channels;
        diff = counter;
        counter = counter + checkImageCapacityBackend(img, imgSize, state);
        diff = counter - diff;
        state.out("File:" + fullPath + " has a capacity of:" + returnSpaceBitsAsSensefulValue(std::round(diff / 8)) + ", which contributes to the full Capacity of the Folder which is currently measure to be:" + returnSpaceBitsAsSensefulValue(std::round(counter / 8)) + "", 2);
        state.out("Freeing Memory...", 4);
        stbi_image_free(img);
    }
    state.out("Final Capacity of:" + ifoldername + ":" + returnSpaceBitsAsSensefulValue(std::round(counter / 8)), 1);
    state.out("Finished", 4);
    return counter;
}

void checkImageFileCapacity(const std::string &ifilename, stateClass &state)
{
    state.out("Starting...", 4);
    int w = 0;
    int h = 0;
    int channels = 0;
    int bitCounter = 0;
    size_t imgSize;
    unsigned char *img = stbi_load(ifilename.c_str(), &w, &h, &channels, 0);
    imgSize = w * h * channels;
    state.out("Available size :" + ts(checkImageCapacityBackend(img, imgSize, state)) + "bits", 0);
}

std::string ReadFilenameFromImageC(unsigned char *imgC, unsigned char *imgR, int &bitI, int &stringI, stateClass &state)
{
    state.out("Starting...", 4);
    bool end = false;
    std::vector<bool> decoded;
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
    if (decoded.empty())
        InvalidInputMessage("Encoded Filename is not readable.");
    std::string s = BitsToAscii(decoded);
    state.out("Filename Extracted", 4);
    return s.erase(s.size() - 1, 1);
}

bool EncodeImage(const std::string &ifilename, const std::string &ffilename_, stateClass &state)
{
    state.out("Starting...", 4);
    std::string ffilename = ffilename_;
    int w{}, h{}, channels{}, imgSize{}, bitI{}, stringI{};
    std::vector<bool> array;
    std::string mfilename=ifilename;
    state.out("Loading image '" + ifilename + "...", 1);
    unsigned char *img = stbi_load(ifilename.c_str(), &w, &h, &channels, 0);
    imgSize = w * h * channels;
    int capacity=0;
    if (!img)
        InvalidInputMessage("Failed to load image context.");

    state.out("User Input Check", 4);
    if (ffilename == "")
    {
      state.out("Input filename:",1);
        std::cin >> ffilename;
    }

    ReadFileToArray(ffilename, array, state);

    state.out("Checking capacity", 4);
    capacity= checkImageCapacityBackend(img, imgSize, state);
    if (array.size() > capacity)
    {
        InvalidInputMessage("Given File is too big to encode. Encoding File size=" + std::to_string(array.size()) + "; imageSize" + std::to_string(imgSize));
        stbi_image_free(img);
        return 1;
    }

    state.out("Writing to Image (Memory)...", 1);
    state.out("ARRAY Size:" + ts(array.size()) + "bit/" + ts(array.size() / 8) + "Bytes", 4);
    WriteToImage(img, capacity, array, state, bitI, stringI);

    state.out("Writing to Image (Disk)...", 1);
    mfilename =  mfilename.insert(mfilename.length() - 4, 1, 'M');
    stbi_write_png(mfilename.c_str(), w, h, channels, img, w*channels);
    state.out("Freeing Memory...", 4);
    stbi_image_free(img);

    state.out("Finished", 4);
    return 0;
}

bool DecodeImage(const std::string &mFilename, const std::string &ffilename_, stateClass &state)
{
    state.out("Starting...", 1);
    std::string ffilename = ffilename_;
    int w{}, h{}, channels{}, bitI{}, stringI{};
    std::string Filename{};
    std::vector<bool> decoded;

    state.out("Load Mod Img", 1);
    unsigned char *imgR = stbi_load(mFilename.c_str(), &w, &h, &channels, 0);
    if (!imgR)
        return 1;

    state.out("User Input Check", 4);
    if (ffilename == "")
    {
        std::cout << "Enter Original Filename:";
        std::cin >> ffilename;
    }
    if (!checkEx(ffilename, state))
        InvalidInputMessage("Target missing.");

    state.out("Load Orig Img", 1);
    unsigned char *imgC = stbi_load(ffilename.c_str(), &w, &h, &channels, 0); 
    if (!imgC)
        return 1;

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

bool EncodeImageFolder(const std::string &ifoldername, const std::string &ffilename_, stateClass &state)
{
    state.out("Init", 4);
    std::vector<std::string> FileList = GetFilenamesFromFolder(ifoldername, state);
    std::vector<bool> array, aChunk;
    std::string fullPath{}, mfoldername{};
    std::string ffilename = ffilename_;
    int w{}, inputSize{}, h{}, channels{}, bitcounter{}, NIL{}, bitI{},stringI{}, offset{};
    int chunkSize = 0;
    int capacity = 0;
    unsigned char *img = nullptr;
    mfoldername = ifoldername + "M";
    state.out("Create M-Dir", 1);
    createFolder(mfoldername, state);

    if (ffilename == "")
    {
       std::cout << "Enter Encoding Filename:";
       std::cin >> ffilename;
    }
    if (!checkEx(ffilename, state))
        InvalidInputMessage("The File you specified does not exist or could not be found");

    state.out("Read File", 1);
    ReadFileToArray(ffilename, array, state);
    inputSize = array.size();
    state.out("ARRAY Size:" + std::to_string(array.size()) + "bit/" + std::to_string(array.size() / 8) + "Bytes", 4);

    state.out("Calculating NIL...", 1);
    for (int i = 0; bitcounter < inputSize; i++)
    {

        if (i >= FileList.size())
            InvalidInputMessage("Not enough images to store data.\n Bitcounter:" + std::to_string(bitcounter) + ".\ninputSize:" + std::to_string(inputSize));

        fullPath = ifoldername + "\\" + FileList[i];
        img = stbi_load(fullPath.c_str(), &w, &h, &channels, 0);
        if(img == nullptr)InvalidInputMessage("Failed to load image");
        bitcounter += checkImageCapacityBackend(img, w*h*channels,state);
        NIL = i + 1;
    }
    state.out("Expected NIL" + std::to_string(NIL) + "/" + std::to_string(FileList.size()), 4);

    state.out("Loop Files", 1);
    for (size_t i = 0; i < NIL; i++)
    {
        state.out("Iteration:" + std::to_string(i) + "/" + std::to_string(NIL), 4);
        fullPath = ifoldername + "\\" + FileList[i];
        img = stbi_load(fullPath.c_str(), &w, &h, &channels, 0);

        capacity=checkImageCapacityBackend(img,w*h*channels,state);
        state.out("Assigning Chunk of Size:" + ts(capacity), 4);
        
        chunkSize = std::min((int)array.size() - offset, capacity);
        aChunk.assign(array.begin() + offset, array.begin() + chunkSize + offset);

        bitI = 0;
       stringI = 0;
        state.out("Writing to Image (Memory)...", 1);
        WriteToImage(img, capacity, aChunk, state, bitI, stringI);
        state.out("Actual Chunk Size:" + std::to_string(stringI)+"(If this varies from the previous message, something is seriously wrong)", 4);
        offset = offset + stringI;

        fullPath = mfoldername + "\\" + FileList[i].insert(FileList[i].length() - 4, 1, 'M');
        state.out("Writing to Image (Disk)...", 1);
        stbi_write_png(fullPath.c_str(), w, h, channels, img, channels * w);

        state.out("Freeing Memory...", 4);
        stbi_image_free(img);
    }
    if (state.deleteOverflow)
    {
        for (int i = NIL; i < FileList.size(); i++)
        {
            state.out("Deleting file:" + FileList[i], 1);
            fullPath = ifoldername + "\\" + FileList[i];
            if (remove(fullPath.c_str()) != 0)
                InvalidInputMessage("Failed to delete file");
        }
    }
    state.out("Complete", 1);
    return 0;
}

bool DecodeImageFolder(const std::string &mFoldername, const std::string &iFoldername_, stateClass &state)
{
    state.out("Init", 1);
    unsigned char *imgO{};
    unsigned char *imgE{};
    int w{}, h{}, channels{}, stringI{}, bitI{};
    std::string iFoldername = iFoldername_;
    std::string fullPath{}, filename{};
    std::vector<std::string> eFileList{}, oFileList{};
    std::vector<bool> decoded{}, decodedBuffer{};

    if (iFoldername == "")
    {
        std::cout << "Enter Original Foldername:";
        std::cin >> iFoldername;
    }
    if (!checkEx(iFoldername, state))
        InvalidInputMessage("Original source folder missing.");

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
