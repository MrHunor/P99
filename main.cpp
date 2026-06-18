#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#pragma warning(disable : 4996)
#define ts(x) std::to_string(x)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "CLI11.hpp"
#include "header.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

const char *COMPILE_VERSION = "V(" __DATE__ " " __TIME__ ")\n";
int main(int argc, char *argv[])
{
    std::cout << "Build Version:" << COMPILE_VERSION;
    NullStream nullout;
    std::ostream *out = &nullout;
    bool verbose = false;
    std::string ifilefoldername;
    std::string ffilename;
    std::string mfilefoldername;
    CLI::App app{"P99:Image Encoder/Decoder"};
    app.set_help_all_flag("--help-all", "Expand and show all subcommand options");
    app.add_flag("-v,--verbose", verbose, "Enable verbose output");

    auto encode = app.add_subcommand("encode", "Encode a File into a image or folder containing images");
    encode->add_option("-i,--into", ifilefoldername, "The file or folder containing imagesto encode into")->required()->check(CLI::ExistingPath);
    encode->add_option("-f,--from", ffilename, "The file to encode from")->required()->check(CLI::ExistingFile);
    encode->callback([&]()
                     {
                           if (verbose) 
                     {
                         out = &std::cout; 
                     }

                         if(std::filesystem::is_directory(ifilefoldername)) {
                             EncodeFolder(ifilefoldername, ffilename, *out);
                         } else if (std::filesystem::is_regular_file(ifilefoldername)) {
                          EncodeImage(ifilefoldername, ffilename, *out);
                         } });

    auto decode = app.add_subcommand("decode", "Decode a File from a image or folder containing images");
    decode->add_option("-m,--modified", mfilefoldername, "The image or folder containing images to decode from (modified)")->required()->check(CLI::ExistingPath);
    decode->add_option("-f,--from,-o,--original", ffilename, "The file or folder containing images to decode from (original)")->required()->check(CLI::ExistingPath);
    decode->callback([&]()
                     {
                           if (verbose) 
                     {
                         out = &std::cout; 
                     }

     if(std::filesystem::is_directory(mfilefoldername)) {
        DecodeFolder(mfilefoldername, ffilename, *out);
     } else if (std::filesystem::is_regular_file(mfilefoldername)) {
        DecodeImage(mfilefoldername, ffilename, *out);
     } });

  app.callback([&]()
                 {
                     if (verbose) 
                     {
                         out = &std::cout; 
                     }

                     if (app.get_subcommands().empty()) 
                     {
                         InvalidInputMessage("Interactive Menu is yet to be implemented. Apologies for the inconvenience.");
                     }
                 });
    CLI11_PARSE(app, argc, argv);
}