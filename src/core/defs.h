#pragma once
#define endo << "\n"
#define ts(x) std::to_string(x)
#include <filesystem>
#include <iostream>
#include <source_location>
namespace fs = std::filesystem;
class stateClass
{
public:
    int verbose;
    bool deleteOverflow;
    void out(const std::string &output, int importance, std::source_location location = std::source_location::current())
    {
        if (importance <= verbose && verbose >= 4)
            std::cout << location.function_name() << "->";
        if (importance <= verbose)
            std::cout << output;
        if (verbose >= 4)
        {
            auto now = std::chrono::system_clock::now();
            std::cout << "@" << std::format("{}",now);
        }
        if (importance <= verbose)
            std::cout << std::endl<<std::flush;
    }
};
