
#pragma warning(disable : 4996)
#define ts(x) std::to_string(x)
#include "../include/CLI11/CLI11.hpp"
#include "img/imageHLL.h"
#include "wav/wavHLL.h"
#include "core/utils.h"
#include "core/defs.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <windows.h>

const char *COMPILE_VERSION = "V(" __DATE__ " " __TIME__ ")\n";
int main(int argc, char *argv[])
{
    stateClass state;
    state.verbose=1;
    std::cout << "Build Version:" << COMPILE_VERSION;
    char mediatype;
    std::string ifilefoldername;
    std::string ffilename;
    std::string mfilefoldername;
    CLI::App app{"P99:Media Encoder/Decoder"};
    auto *media_group = app.add_option_group("Media Into Type");
    app.set_help_all_flag("--help-all", "Expand and show all subcommand options");
    app.add_flag("-v,--verbose", state.verbose, "Enable verbose output");
    auto *audio = media_group->add_flag("-a,--audio", "Into File or all Files in the Folder is/are Audio File");
    auto *image = media_group->add_flag("-m,--image", "Into File or all Files in the Folder is/are image");
    media_group->require_option(1);
    auto encode = app.add_subcommand("encode", "Encode a File into a image or folder containing images");
    encode->add_option("-i,--into", ifilefoldername, "The file or folder containing images to encode into")->required()->check(CLI::ExistingPath);
    encode->add_option("-f,--from", ffilename, "The file to encode from")->required()->check(CLI::ExistingFile);
    encode->add_flag("-o,--overflow",state.deleteOverflow,"Delete Overflow Files out of from folder to make decoding easier.");
    encode->callback([&]()
                     {
if(state.verbose>=3)system("systeminfo");
                         if(std::filesystem::is_directory(ifilefoldername)) {
                            
                               if(*audio)
                            {
                               EncodeWavFolder(ifilefoldername,ffilename,state);
                            }
                             if (*image)
                            {
                             EncodeImageFolder(ifilefoldername, ffilename, state);
                            }
                            } else if (std::filesystem::is_regular_file(ifilefoldername)) {
                          if(*audio)
                            {
                               EncodeWav(ifilefoldername,ffilename,state);
                            }
                            else if (*image)
                            {
                                EncodeImage(ifilefoldername, ffilename, state);
                        } } });

    auto decode = app.add_subcommand("decode", "Decode a File from a image or folder containing images");
    decode->add_option("-m,--modified", mfilefoldername, "The image or folder containing images to decode from (modified)")->required()->check(CLI::ExistingPath);
    decode->add_option("-i,--into,-o,--original", ifilefoldername, "The file or folder containing images to decode from (original)")->required()->check(CLI::ExistingPath);
    decode->callback([&]()
                     {
                if(state.verbose>=3)system("systeminfo");       

     if(std::filesystem::is_directory(mfilefoldername)) {
        if(*audio)
        {
         DecodeWavFolder(mfilefoldername,ifilefoldername,state);
        }
        else if (*image)
        {DecodeImageFolder(mfilefoldername, ifilefoldername, state);}
        
     } else if (std::filesystem::is_regular_file(mfilefoldername)) {
        if(*audio)
        {
DecodeWav(mfilefoldername,ifilefoldername,state);
        }
        else if(*image)
        {
        DecodeImage(mfilefoldername, ifilefoldername, state);
        }
     } });

     auto check = app.add_subcommand("check","Check capacity of a Media File (currently only supportes images)");
     check->add_option("-i,--into", ifilefoldername, "The file or folder containing images to encode into")->required()->check(CLI::ExistingPath);

     check->callback([&]()
     {
        if(state.verbose>=3)system("systeminfo");
        state.out("Running Check callback",4);
 if(std::filesystem::is_directory(ifilefoldername)) {
        if(*audio)
        {
            state.out("Audio Files are currently unsuppored",1);
        }
        else if (*image)
        {
            state.out("Checking image dir",4);
checkImageFolderCapacityMidEnd(ifilefoldername,state);
        }
        
     } else if (std::filesystem::is_regular_file(ifilefoldername)) {
        if(*audio)
        {
    
           state.out("Audio Files are currently unsuppored",1);
        }
        else if(*image)
        {
            state.out("Checking image file",4);
checkImageFileCapacity(ifilefoldername,state);
        }

 } });
    app.callback([&]()
                 {
                    

                     if (app.get_subcommands().empty()) 
                     {
                         InvalidInputMessage("Interactive Menu is yet to be implemented. Apologies for the inconvenience.");
                     } });
    CLI11_PARSE(app, argc, argv);
}