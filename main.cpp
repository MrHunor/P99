#pragma warning(disable : 4996)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define ts(x) std::to_string(x)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <string>

#include "header.h"

using std::cin;
using std::cout;
using std::endl;

const char *COMPILE_VERSION = "V(" __DATE__ " " __TIME__ ")\n";

int main(int argc, char *argv[])
{
	cout << "Build Version:" << COMPILE_VERSION;

	string placeholder;
	std::ostream *out = nullptr;
	std::ofstream file("output.txt");
	NullStream nullout;

	stbi_write_png_compression_level = 9;

	switch (argc)
	{
	case 1:
	{
		cout << "Display File Treee from current directory for easier File Navigation? (Y/N)" << endl;
		cin >> placeholder;
		if (placeholder == "Y")
		{
			system("tree");
		}
		cout << "Verbose (V), Fileoutput(F) or None (N):";
		cin >> placeholder;

		if (placeholder == "V")
		{
			out = &std::cout;
			file.close();
			remove("output.txt");
		}
		else if (placeholder == "F")
		{
			file.open("output.txt");
			out = &file;
		}
		else if (placeholder == "N")
		{
			out = &nullout;
			file.close();
			remove("output.txt");
		}
		else
		{
			InvalidInputMessage("This input is not valid. Please choose either V, F or N");
			return 0;
		}

		cout << "\nEncode (E) or Decode (D):";
		cin >> placeholder;
		if (placeholder == "E")
		{
			cout << "Enter Imagename or Foldername:";
			cin >> placeholder;
			if (checkEx(placeholder))
			{
				if (placeholder[placeholder.length() - 4] != '.')
					EncodeFolder(placeholder, "", *out);
				else
					EncodeImage(placeholder, "", *out);
			}
			else
				InvalidInputMessage("Your input is neiter a File nor a folder");
		}
		else if (placeholder == "D")
		{
			cout << "Enter Encoded Filename or Foldername:";
			cin >> placeholder;
			if (checkEx(placeholder))
			{
				if (placeholder[placeholder.length() - 4] != '.')
				{
					DecodeFolder(placeholder, "", *out);
				}
				else
				{
					DecodeImage(placeholder, "", *out);
				}
			}
			else
				InvalidInputMessage("The specified file or folder does not exist");
		}
	}

	break;
	// arg ("console") version
	default:
		/*
		for (size_t i = 0; i < argc; i++)
		{
		cout<<"Arg " << i << ":" << argv[i] << endl;
		}
		*/
		out = &std::cout;
		file.close();
		remove("output.txt");
		if (string(argv[1]) == "--help")
		{
			// yes this way ai, i have no social skills, also this is horrible need fixing,
			cout << "\nEncodes data into or decodes data from images and folders.\n\n";
			cout << "Usage:\n";
			cout << "    [interactive mode]\n";
			cout << "    [command] [action] [target] [source/modified]\n\n";
			cout << "Actions:\n";
			cout << "    encode    Hides data within a specified image or folder.\n";
			cout << "    decode    Extracts hidden data from a modified image or folder.\n\n";
			cout << "Examples:\n";
			cout << "    encode myfolder\\ outputfolder\\\n";
			cout << "    decode myfolder\\ outputfolder\\\n\n";
			cout << "If run with no arguments, the tool starts in interactive menu mode.\n";

			return 0;
		}
		if (string(argv[1]) == "encode")
		{
			// second arg is file/foldername to encode into
			if (string(argv[2])[string(argv[2]).length() - 4] != '.')
			{
				// third argument is file to encode from
				EncodeFolder(string(argv[2]), string(argv[3]), *out);
			}
			else
			{
				EncodeImage(string(argv[2]), string(argv[3]), *out);
				// File
			}

			return 0;
		}
		
		if (string(argv[1]) == "decode")
		{
			if (string(argv[2]).length() >= 4 && string(argv[2]).substr(string(argv[2]).length() - 4) != ".txt")
			{
				cout << "Decoding folder with original foldername:" << argv[2] << " and modified foldername:" << argv[3] << endl;
				// second argument is the original foldername and third argument is the modified foldername
				DecodeFolder(string(argv[2]), string(argv[3]), *out);
			}
			else
			{
				DecodeImage(string(argv[2]), string(argv[3]), *out);
			}

			return 0;
		}
		InvalidInputMessage("Invalid Options");
	}
}