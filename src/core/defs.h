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
    // LINK ../../docs/core.md:6
    void out(const std::string &output, int importance, std::source_location location = std::source_location::current())
    {
        if (importance > verbose)
            return;
        std::string message = "";
        if (verbose >= 4)
        {
            auto now = std::chrono::system_clock::now();
            message += std::format("@{} -> ", now);
        }
        if (verbose >= 4)message += std::format("{}->", location.function_name());
        message += output;
         std::cout << message;
        std::cout << std::endl
                  << std::flush;
    }
};
