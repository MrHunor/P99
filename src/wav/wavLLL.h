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
void ReadDataFromWavC(float *mSampleData, float *iSampleData, int &bitI, int stringI, std::vector<bool> &decoded, stateClass &state);
std::string ReadFilenameFromWavC(std::vector<short> &mbuffer, std::vector<short> &ibuffer, int &bitI, int &stringI, stateClass &state);
void ReadDataFromWavC(std::vector<short> &mbuffer, std::vector<short> &ibuffer, int &bitI, int &stringI, std::vector<bool> &decoded, stateClass &state);