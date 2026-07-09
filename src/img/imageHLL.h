#pragma once
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

int checkImageFolderCapacityMidEnd(const std::string &ifoldername, stateClass &state);
void checkImageFileCapacity(const std::string &ifilename, stateClass &state);
std::string ReadFilenameFromImageC(unsigned char *imgC, unsigned char *imgR, int &bitI, int &stringI, stateClass &state);
bool EncodeImage(const std::string &ifilename, const std::string &ffilename_, stateClass &state);
bool DecodeImage(const std::string &mFilename, const std::string &ffilename_, stateClass &state);
bool EncodeImageFolder(const std::string &ifoldername, const std::string &ffilename_, stateClass &state);
bool DecodeImageFolder(const std::string &mFoldername, const std::string &iFoldername_, stateClass &state);