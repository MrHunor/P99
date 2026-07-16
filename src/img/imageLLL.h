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


size_t checkImageCapacityBackend(unsigned char *img, size_t imgSize, stateClass &state);
bool WriteToImage(unsigned char *img, size_t imgSize, const std::vector<bool> &s, stateClass &state, int &bitI, int &stringI);
void ReadDataFromImageC(unsigned char *imgC, unsigned char *imgR, int size, int &bitI, int &stringI, std::vector<bool> &decoded, stateClass &state);
