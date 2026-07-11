#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <source_location>
#include "defs.h" 

/* =========================================================
   1. BASIC UTILITIES
   ========================================================= */

// The default argument belongs ONLY here in the header definition
void InvalidInputMessage(const std::string &details, std::source_location location = std::source_location::current());

bool createFolder(const std::string &name, stateClass &state);

bool checkEx(const std::string &path, stateClass &state);

std::vector<std::string> GetFilenamesFromFolder(std::string path, stateClass &state);

void ReccomendActionFilelistMismatch(const std::vector<std::string> &FileList1ORIGINAL, const std::vector<std::string> &FileList2ORIGINAL, stateClass &state);

void CheckFilelists(const std::vector<std::string> &FileList1, const std::vector<std::string> &FileList2, stateClass &state);

std::string returnSpaceBitsAsSensefulValue(int value);

/* =========================================================
   2. BIT / FILE UTILITIES
   ========================================================= */

std::vector<bool> TextToAsciiB(const std::string &s);

std::string BitsToAscii(const std::vector<bool> &bits);

void ReadFileToArray(const std::string &filename, std::vector<bool> &array, stateClass &state);

void WriteBitsToFile(const std::string &filename, const std::vector<bool> &bits, stateClass &state);

std::uintmax_t ReadbSizeFromFile(const std::string &filename);

bool FileIs(const std::string &filename, const std::string &extension);