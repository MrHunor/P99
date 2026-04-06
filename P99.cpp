#pragma warning(disable : 4996)
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>

#include <stb_image.h>
#include <stb_image_write.h>

#include "Header.h"

using std::cout;
using std::endl;
using std::cin;

//finish improve performace,finish argv
int main(int argc, char* argv[])
{
	string placeholder;
	std::ostream* out = nullptr;
	std::ofstream file("output.txt");
	NullStream nullout;

	stbi_write_png_compression_level = 9;

	switch (argc)
	{
	case 1:
	{
		cout << "Verbose (V), Fileoutput(F) or None (N):";
		cin >> placeholder;

		if (placeholder == "V") {
			out = &std::cout;
			file.close();
			remove("output.txt");
		}
		else if (placeholder == "F") {
			file.open("output.txt");
			out = &file;
		}
		else if (placeholder == "N") {
			out = &nullout;
			file.close();
			remove("output.txt");
		}
		else {
			InvalidInputMessage();
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
					EncodeFolder(placeholder, *out);
				else
					EncodeImage(placeholder, *out);
			}
			else InvalidInputMessage("Your input is neiter a File nor a folder");
		}
		else if (placeholder == "D")
		{
			cout << "Enter Encoded Filename or Foldername:";
			cin >> placeholder;
			if (checkEx(placeholder))
			{
				if (placeholder[placeholder.length() - 4] != '.')
				{
					DecodeFolder(placeholder, *out);
				}
				else
				{
					DecodeImage(placeholder, *out);
				}
			}
			else InvalidInputMessage();
		}
	}

	break;
	//arg ("console") version
	default:
		if (argv[1] == "E" || argv[1] == "e")
		{
			//second arg is file/foldername to encode to
			if (argv[2][strlen(argv[2]) - 4] != '.')
			{
				//Folder
			}
			else
			{
				//File
			}

			return 0;
		}
		if (argv[1] == "D" || argv[1] == "d")
		{
			return 0;
		}
		InvalidInputMessage();
	}
	cout << "Finished";
}