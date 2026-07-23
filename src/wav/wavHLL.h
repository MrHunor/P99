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
#include "../wav/wavLLL.h"
bool EncodeWav(const std::string &ifilename, const std::string &ffilename, stateClass &state);
bool DecodeWav(const std::string &mFilename, const std::string &iFilename, stateClass &state);
bool EncodeWavFolder(const std::string &ifoldername, const std::string &ffilename, stateClass &state);
bool DecodeWavFolder(const std::string &mfoldername, const std::string ifoldername, stateClass &state);
void CheckWavFileCapacityFrontEnd(const std::string &ifilename, stateClass& state);