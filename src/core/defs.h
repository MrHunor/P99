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
    //LINK ../../docs/core.md:6
    void out(const std::string &output, int importance, std::source_location location = std::source_location::current())
    {
        if(importance>verbose) return;
        std::string message = "";

        if ( verbose >= 4 ) message += std::format("{}->",location.function_name());
        message+=output;
        if (verbose >= 4)
        {
            auto now = std::chrono::system_clock::now();
            //This may look weird but this just pads the message to x characters so the timestamps are cleanly aligned
            std::cout<< std::format("{:<150}    @{}",message,now);
        }
        else std::cout<<message;

        std::cout<<std::endl<<std::flush;
    
    }
};
