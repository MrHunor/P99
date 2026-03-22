#pragma warning(disable : 4996)
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <iostream>
#include <stb_image.h>
#include <stb_image_write.h>
#include <algorithm>
#include <string>
#include <vector>

#include "Header.h"
using std::cout;
using std::endl;
// implement readi9ng how many chanels the image has and using those instead of forcing 3, impleent reading of folder images
int main(int argc, char* argv[])
{
	stbi_write_png_compression_level = 9;
	switch (argc)
	{
	case 1:
	{
		bool end = false;

		vector<bool> decoded;
		decoded.clear();
		string placeholder;
		string foldername;
		string Filename;
		int w, h, channels;
		int imgChannels;
		int stringI = 0;
		int bitI = 0;
		int imgSize;
		string eFoldername;
		string oFoldername;
		uint64_t inputSize;
		int bitcounter = 0;
		int NIL = 0;//Needed Image Load (how many Images are needed to encode the input)
		string sChunk;
		string fullPath;
		cout << "Encode (E) or Decode (D):";
		cin >> placeholder;

		if (placeholder == "E")
		{
			cout << "Enter Imagename or Foldername:";
			cin >> placeholder;
			if (placeholder[placeholder.length() - 4] != '.')
			{
				foldername = placeholder;
				vector<string> FileList;
				FileList = GetFilenamesFromFolder(foldername);
				cout << "Files in Folder:" << endl;
				for (auto entry : FileList)
				{
					cout << entry << endl;
				}
				cout << "Enter Filename(Files Bigger then 1GB might cause issues due to RAM overload):";
				cin >> placeholder;
				string s = ReadFileToBString(placeholder);
				inputSize = s.size();
				if (inputSize < 5000)
				{
					cout << "\nRaw Binary:" << s << endl;
				}
				cout << "Size of text:" << inputSize << endl;
				cout << "Evaluating Image Load..." << endl;
				for (int i = 1; bitcounter < inputSize; i++)
				{
					if (i > FileList.size())
					{
						cout << "Input too big to encode into availabe Images.\n";
						cout << "Finished.";
						return -1;
					}
					fullPath = foldername + "\\" + FileList[i];
					stbi_info(fullPath.c_str(), &w, &h, &channels);
					cout << w << " " << h << " " << channels << endl;
					bitcounter += (w * h * channels);
					NIL = i;
				}
				cout << "NIL:" << NIL << "/" << FileList.size() << endl;
				cout << "Encoding...";
				for (size_t i = 0; i < NIL; i++)
				{
					cout << "Loading image '" << FileList[i] << "'..\n";
					fullPath = foldername + "\\" + FileList[i];
					cout << "Loading:" << fullPath << endl;
					unsigned char* img = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);
					sChunk = s.substr(0, (w * h * channels));
					cout << "Selected Chunk size:" << sChunk.length();
					cout << "Encoding..." << endl;
					bitI = 0;
					stringI = 0;
					WriteToImage(img, (w * h * channels), sChunk, false, bitI, stringI);
					s.erase(0, stringI);
					cout << "Encoded Chunck size:" << stringI;
					cout << "Writing file..." << endl;
					fullPath = foldername + "\\" + FileList[i].insert(FileList[i].length() - 4, 1, 'M');
					cout << "Writing to:" << fullPath << endl;
					stbi_write_png(fullPath.c_str(), w, h, channels, img, channels * w);
					stbi_image_free(img);
				}
			}

			else
			{
				cout << "\nLoading image '" << placeholder << "'...\n";
				unsigned char* img = stbi_load(placeholder.c_str(), &w, &h, &channels, 4);
				imgSize = w * h * 4;
				if (img != nullptr)cout << "Loaded image.\n";
				else
				{
					cout << "Failed to load image.";
					return 0;
				}
				cout << "Enter Filename:";
				cin >> placeholder;
				string s = ReadFileToBString(placeholder);
				cout << "\nRaw Binary:" << s << endl;
				cout << "Size of text:" << s.length() << endl;
				if (s.length() > imgSize)
				{
					cout << "Input too Large." << endl;
					stbi_image_free(img);
					cout << "Finished.\n";
					return -1;
				}
				cout << "Encoding...\n";
				WriteToImage(img, imgSize, s, true, bitI, stringI);
				cout << "Writing file 'output.png'...\n";
				stbi_write_png("output.png", w, h, 4, img, 4 * w);
				stbi_image_free(img);
				cout << "Finished.\n";
			}
		}
		else if (placeholder == "D")
		{
			cout << "Enter Encoded Filename or Foldername:";
			cin >> placeholder;
			if (placeholder[placeholder.length() - 4] != '.')
			{
				eFoldername = placeholder;
				cout << "Enter Original Foldername:" << endl;
				cin >> oFoldername;
			}
			else
			{
				unsigned char* imgR = stbi_load(placeholder.c_str(), &w, &h, &channels, 4);
				cout << "Loading image '" << placeholder << "'...";
				if (imgR == nullptr)
				{
					cout << "\nFailed to load image, exiting.";
					return 0;
				}
				cout << "\nLoaded image.\n";
				cout << "Enter Original Filename:";
				cin >> placeholder;
				unsigned char* imgC = stbi_load(placeholder.c_str(), &w, &h, &channels, 4);
				if (imgC == nullptr)
				{
					cout << "\nFailed to load image, exiting.";
					return 0;
				}
				cout << "\nLoaded image.\n";
				//Pre Processing
				Filename = ReadFilenameFromImageC(imgC, imgR, bitI, stringI);
				cout << "Filename:" << Filename << endl;
				//Actual Processing
				ReadDataFromImageC(imgC, imgR, bitI, stringI, decoded);
				cout << "Read following binary:";
				for (bool i : decoded)
				{
					if (i == false)cout << "0";
					if (i == true)cout << "1";
				}
				cout << "\nWhich in ascci is:\n" << BitsToAscii(decoded) << endl;
				cout << "Writing to file:'" << Filename << "'..." << endl;
				WriteBitsToFile(Filename, decoded);
				cout << "Finished writing.\nFreeing memory..." << endl;
				stbi_image_free(imgR);
				stbi_image_free(imgC);
				cout << "Finished";
			}
		}
	}

	break;
	/*default:
	{
		bool end = false;
		//argument checking

		if (string(argv[1]) != "E" && string(argv[1]) != "D")end = true;
		if (string(argv[1]) == "E" && (argc > 3 || argc < 2))end = true;
		if (string(argv[1]) == "D" && (argc > 4 || argc < 3))end = true;
		if (end)
		{
			cout << "Wrong command entered.\nP99 Syntax:\n";
			cout << "P99.exe E/D (inputFileName)/(EncodedFileName OriginalFileName) (FILE)/()\n";
			cout << "Given Syntax: P99.exe ";
			for (int i = 0; i < argc; i++)
			{
				cout << argv[i] << " ";
			}
			return 0;
		}
		vector<bool> decoded;
		string placeholder;
		int w, h, channels;
		int stringI = 0;
		int bitI = 0;

		if (argv[0] == "E")
		{
			unsigned char* img = stbi_load(string(argv[2]).c_str(), &w, &h, &channels, 4);
			if (img != nullptr)cout << "Loaded image.\n";
			else
			{
				cout << "Failed to load image.";
				return 0;
			}

			string s = TextToAsciiB(string(argv[3]));
			cout << "\nText to AscciBinary:" << s << endl;
			cout << "Size of text:" << s.length() << endl;
			cout << "Encoding...\n";
			//Magik
			while (true)
			{
				cout << "bitI:" << bitI << endl;
				cout << "stringI:" << stringI << endl;
				if (img[bitI] > 0 && img[bitI] < 255)
				{
					if (s[stringI] == '0')
					{
						img[bitI]--;
						stringI++;
					}
					else if (s[stringI] == '1')
					{
						img[bitI]++;
						stringI++;
					}
				}
				bitI++;
				if (stringI == s.length()) {
					break;
				}
			}
			cout << "Writing file 'output.png'...\n";
			stbi_write_png_compression_level = 9;
			stbi_write_png("output.png", w, h, 4, img, 4 * w);
			stbi_image_free(img);
			cout << "Finished.\n";
		}
		else if (placeholder == "D" || argv[0] == "D")
		{
			unsigned char* imgR = stbi_load(string(argv[2]).c_str(), &w, &h, &channels, 4);
			cout << "Loading image '" << placeholder << "'...";
			if (imgR == nullptr)
			{
				cout << "\nFailed to load image, exiting.";
				return 0;
			}
			cout << "\nLoaded image.\n";
			unsigned char* imgC = stbi_load(string(argv[3]).c_str(), &w, &h, &channels, 4);
			if (imgC == nullptr)
			{
				cout << "\nFailed to load image, exiting.";
				return 0;
			}
			cout << "\nLoaded image.\n";
			while (!end)
			{
				cout << "bitI:" << bitI << endl;
				cout << "stringI:" << stringI << endl;
				if (imgC[bitI] > 0 && imgC[bitI] < 255)
				{
					if (imgR[bitI] == imgC[bitI] - 1)
					{
						decoded.push_back(false);
						stringI++;
					}
					else if (imgR[bitI] == imgC[bitI] + 1)
					{
						decoded.push_back(true);
						stringI++;
					}
					else
					{
						end = true;
					}
				}
				bitI++;
			}
			cout << "Read following binary:";
			for (bool i : decoded)
			{
				if (i == false)cout << "0";
				if (i == true)cout << "1";
			}
			cout << "\nWhich in ascci is:\n" << BitsToAscii(decoded) << endl;
			stbi_image_free(imgR);
			stbi_image_free(imgC);
			cout << "Finished";
		}
	}
	break;
	*/
	}
	return 0;
}