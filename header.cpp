
#pragma warning(disable : 4996)
#define endo << "\n"
#define ts(x) std::to_string(x)
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
#include "header.h"
#include <algorithm>
#include <dr_wav.h>
namespace fs = std::filesystem;

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

/* =========================================================1. BASIC UTILITIES========================================================= */

void InvalidInputMessage(const string &details)
{
	cout << "Your Input was Invalid: " << details;
	exit(1);
}

bool createFolder(const string &name)
{
	return !fs::create_directory(name);
}

bool checkEx(const std::string &path)
{
	return fs::exists(path);
}

vector<string> GetFilenamesFromFolder(string path)

{
	vector<string> PathV;
	for (const auto &entry : fs::directory_iterator(path))
	{
		PathV.push_back(entry.path().filename().string());
	}
	/*	cout << "Unsorted Vector:\n";
		for (auto e : PathV)
		{
			cout << e endo;
		}
			*/
	std::sort(PathV.begin(), PathV.end(), [](const std::string &a, const std::string &b)
			  {
		auto getNumber = [](const std::string& s) {
			size_t pos = 0;
			while (pos < s.size() && isdigit(s[pos])) pos++;
			return std::stoi(s.substr(0, pos));
			};
		return getNumber(a) < getNumber(b); });
	/*
cout << "Sorted Vector:\n";
for (auto e : PathV)
{
	cout << e endo;
}
	*/
	return PathV;
}

void ReccomendActionFilelistMismatch(const vector<string> &FileList1ORIGINAL, const vector<string> &FileList2ORIGINAL, ostream &out)
{
	vector<string> FileList1 = FileList1ORIGINAL;
	vector<string> FileList2 = FileList2ORIGINAL;
	out << "Size Diffrence:\nFilelist1:" << FileList1.size() << "\nFilelist2:" << FileList2.size() endo;
	for (size_t i = 0; i < FileList1.size(); i++)
	{
		for (size_t z = 0; z < FileList2.size(); z++)
		{
			string modified = FileList1[i];
			modified.insert(modified.length() - 4, 1, 'M');
			if (modified == FileList2[z])
			{
				FileList1.erase(FileList1.begin() + i);
				FileList2.erase(FileList2.begin() + z);

				--i;   // re-check new element at i
				break; // stop using z, it's now invalid context
			}
		}
	}
	out << "Following images were NOT found:\nFilelist1:";
	for (auto i : FileList1)
	{
		out << i endo;
	}
	out << "Filelist2:";
	for (auto i : FileList2)
	{
		out << i endo;
	}
}

void CheckFilelists(const vector<string> &FileList1, const vector<string> &FileList2, ostream &out)
{
	if (FileList1.size() != FileList2.size())
	{
		ReccomendActionFilelistMismatch(FileList1, FileList2, out);
		InvalidInputMessage("Filelist size mismatch.");
	}
	for (size_t i = 0; i < FileList1.size(); i++)
	{
		out << "Current mapping:Filelist1:" << FileList1[i] << "->" << FileList2[i] endo;
		string modified = FileList1[i];
		modified.insert(modified.length() - 4, 1, 'M');
		if (modified != FileList2[i])
		{
			ReccomendActionFilelistMismatch(FileList1, FileList2, out);
			InvalidInputMessage("A Filelist Item does not match its pair. Details:\nN(Null Initialised):" + ts(i) + "\nName in Filelist 1:" + FileList1[i] + "Name in Filelist 2:" + FileList2[i]);
		}
	}
}

/* =========================================================2. BIT / FILE UTILITIES========================================================= */

std::vector<bool> TextToAsciiB(const std::string &s)
{
	std::vector<bool> result;
	result.reserve(s.size() * 8);

	for (unsigned char c : s)
	{
		std::bitset<8> b(c);
		for (int i = 7; i >= 0; --i)
		{
			result.push_back(b[i]);
		}
	}

	return result;
}

string BitsToAscii(const vector<bool> &bits)
{
	if (bits.size() % 8 != 0)
		throw std::runtime_error("Bit vector length must be a multiple of 8");

	std::string result;
	result.reserve(bits.size() / 8);

	for (size_t i = 0; i < bits.size(); i += 8)
	{
		unsigned char value = 0;

		for (int b = 0; b < 8; ++b)
		{
			value <<= 1;
			value |= bits[i + b] ? 1 : 0;
		}

		result.push_back(static_cast<char>(value));
	}

	return result;
}

void ReadFileToArray(const std::string &filename, std::vector<bool> &array, ostream &out)
{
	// No idea how this work
	//  --- Read file safely ---
	std::ifstream file(filename, std::ios::binary);
	if (!file)
		throw std::runtime_error("Failed to open file");

	file.seekg(0, std::ios::end);
	std::streampos end = file.tellg();
	if (end < 0)
		throw std::runtime_error("tellg() failed");

	size_t size = static_cast<size_t>(end);
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	if (!file.read(buffer.data(), size))
		throw std::runtime_error("Failed to read file");

	// --- Prepare metadata ---
	std::vector<bool> header = TextToAsciiB(filename + "|");

	// --- Build final result cleanly ---
	array.clear();
	out << "Reserving array for Size:" << header.size() + size << endl;
	array.reserve(header.size() + size);
	out << "Reserving finished" endo;
	// insert header first
	out << "Inserting header..." endo;
	array.insert(array.end(), header.begin(), header.end());
	out << "Appending file data..." endo;
	// then append file data
	for (unsigned char c : buffer)
	{
		for (int bit = 7; bit >= 0; --bit)
		{
			array.push_back((c >> bit) & 1);
		}
	}
}

void WriteBitsToFile(const std::string &filename, const std::vector<bool> &bits)
{
	std::ofstream out(filename, std::ios::binary);
	if (!out)
	{
		throw std::runtime_error("Failed to open file");
	}

	std::vector<uint8_t> bytes;
	bytes.reserve((bits.size() + 7) / 8);

	for (size_t i = 0; i < bits.size(); i += 8)
	{
		uint8_t byte = 0;

		for (size_t j = 0; j < 8 && (i + j) < bits.size(); ++j)
		{
			byte <<= 1;
			byte |= bits[i + j] ? 1 : 0;
		}

		bytes.push_back(byte);
	}

	out.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
}

std::uintmax_t ReadbSizeFromFile(const std::string &filename){
	return std::filesystem::file_size(filename) * 8;
}
bool FileIs(const std::string& filename, const std::string& extension) {
    return std::filesystem::path(filename).extension() == extension;
}
/* =========================================================3. LOW-LEVEL IMAGE LOGIC========================================================= */

bool WriteToImage(unsigned char *img, size_t imgSize, const vector<bool> &s, ostream &out, int &bitI, int &stringI)
{
	auto sLength = s.size();
	while (stringI < sLength)
	{
		if (bitI >= imgSize)
			return 1;

		if (img[bitI] > 0 && img[bitI] < 255)
		{
			if (s[stringI] == 0)
			{
				img[bitI]--;
				stringI++;
			}
			else if (s[stringI] == 1)
			{
				img[bitI]++;
				stringI++;
			}
		}
		bitI++;
	}
	return 0;
}

void ReadDataFromImageC(unsigned char *imgC, unsigned char *imgR, int size, int &bitI, int &stringI, vector<bool> &decoded, ostream &out)
{
	bool end = false;
	while (!end && bitI < size)
	{
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
}

void ReadDataFromWavC( float *mSampleData, float *iSampleData,int &bitI,int stringI,vector<bool> &decoded,ostream &out)
{
	bool end=false;
	while(!end)
	{
		if (std::abs(iSampleData[bitI]) <= 0.9998f&&iSampleData[bitI]!=0)
		{
			if (mSampleData[bitI]==iSampleData[bitI]-0.0001f)
			{
				decoded.push_back(false);
				stringI++;
			}
			else if (mSampleData[bitI]==iSampleData[bitI]+0.0001f)
			{
				decoded.push_back(true);
				stringI++;
			}
			else
			{
				end = true;
			}
		}
		}
}

string ReadFilenameFromImageC(unsigned char *imgC, unsigned char *imgR, int &bitI, int &stringI, ostream &out)
{
	bool end = false;
	vector<bool> decoded;
	decoded.reserve(200);

	while (!end)
	{
		out << "BitI:" << bitI endo;
		out << "StringI" << stringI endo;
		out << "Decoded size:" << decoded.size() endo;
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

		if (decoded.size() % 8 == 0)
		{
			out << "Checking for End Symbol..." endo;
			if (BitsToAscii(decoded).find('|') != std::string::npos)
			{
				end = true;
				out << "End Symbol found!" endo;
			}
		}
	}
	if (decoded.size() == 0)
		InvalidInputMessage("Encoded Filename is not readable.");
	string s = BitsToAscii(decoded);
	return s.erase(s.size() - 1, 1);
}

string ReadFilenameFromWavC( float *mSampleData, float *iSampleData,int &bitI,int stringI,ostream &out)
{
bool end = false;
vector<bool> decoded;
decoded.reserve(200);
const float epsilon = 0.00001f; 

while (!end)
	{
	/*
	Useful for debugging
	out << "BitI:" << bitI<<"\nmSampleData:"<<mSampleData[bitI]<<"\niSampleData:"<<iSampleData[bitI] endo;
		out << "StringI" << stringI endo;
		out << "Decoded size:" << decoded.size() endo;
		out <<"array so far:\n";
		for(auto i : decoded)
		{
        out<<i endo;
		}
	*/
		//check if valid bitI
		
if (std::abs(iSampleData[bitI]) <= 0.9998f && iSampleData[bitI] != 0.0f)
{
	out<<"Found valid bit" endo;
    float diff = mSampleData[bitI] - iSampleData[bitI];

    // Is the difference almost exactly -0.0001f?
    if (std::abs(diff - (-0.0001f)) < epsilon)
    {
		out<<"found false"endo;
        decoded.push_back(false);
        stringI++;
    }
    // Is the difference almost exactly +0.0001f?
    else if (std::abs(diff - 0.0001f) < epsilon)
    {
        decoded.push_back(true);
        stringI++;
    }
    else
    {
      end=true;
    }
}

		bitI++;

		if (decoded.size() % 8 == 0&&!decoded.empty())
		{
			//out << "Checking for End Symbol... Size is:"<<decoded.size() endo;
			out<<"Data so far:"<<BitsToAscii(decoded)endo;
			if (BitsToAscii(decoded).find('|') != std::string::npos)
			{
				end = true;
				out << "End Symbol found!" endo;
			}
		}
	}
	if (decoded.size() == 0)
		InvalidInputMessage("Encoded Filename is not readable.");
	string s = BitsToAscii(decoded);
	return s.erase(s.size() - 1, 1);

}
/* =========================================================4. IMAGE OPERATIONS========================================================= */
// i is into and f is from m is modified, o and c /e are old formats which are now depreciated
bool EncodeImage(const string &ifilename, const string &ffilename_, ostream &out)
{
	string ffilename = ffilename_;
	int w{}, h{}, channels{}, imgSize{}, bitI{}, stringI{};
	vector<bool> array;
	out << "\nLoading image '" << ifilename << "'...\n";
	unsigned char *img = stbi_load(ifilename.c_str(), &w, &h, &channels, 3);
	imgSize = w * h * 3;

	if (!img)InvalidInputMessage();
	if (ffilename == "")
	{
		cout << "Enter the containing Filename:";
		cin >> ffilename;
	}
	if (!checkEx(ffilename))
		InvalidInputMessage("This file does not exist");
	out << "Reserving array for Size:" << ReadbSizeFromFile(ffilename) << endl;
	ReadFileToArray(ffilename, array, out);

	if (array.size() > imgSize)
	{
		InvalidInputMessage("Given File is too big to encode. Encoding File size=" + std::to_string(array.size()) + "; imageSize" + std::to_string(imgSize));
		stbi_image_free(img);
		return 1;
	}
	out << "Writing to Image (Memory)..." endo;
	out << "ARRAY Size:" << array.size() << "bit/" << array.size() / 8 << "Bytes" endo;
	WriteToImage(img, imgSize, array, out, bitI, stringI);
	out << "Writing to Image (Disk)..." endo;
	stbi_write_png("output.png", w, h, 3, img, 3 * w);
	out << "Freeing Memory..." endo;
	stbi_image_free(img);
	return 0;
}

bool DecodeImage(const string &mFilename, const string &ffilename_, ostream &out)
{
	string ffilename = ffilename_;
	int w{}, h{}, channels{}, bitI{}, stringI{};
	string Filename{};
	vector<bool> decoded;

	unsigned char *imgR = stbi_load(mFilename.c_str(), &w, &h, &channels, 3); // FIXED
	if (!imgR)
		return 1;
	if (ffilename == "")
	{
		cout << "Enter Original Filename:";
		cin >> ffilename;
	}
	if (!checkEx(ffilename))
		InvalidInputMessage("");

	unsigned char *imgC = stbi_load(ffilename.c_str(), &w, &h, &channels, 3);
	if (!imgC)
		return 1;

	Filename = ReadFilenameFromImageC(imgC, imgR, bitI, stringI, out);
	out << "Read Filename:" << Filename endo;
	out << "Channels:" << channels endo;
	ReadDataFromImageC(imgC, imgR, (w * h * channels), bitI, stringI, decoded, out);

	WriteBitsToFile(Filename, decoded);

	stbi_image_free(imgR);
	stbi_image_free(imgC);
	return 0;
}

/* =========================================================5. FOLDER OPERATIONS========================================================= */
// i is into and f is from m is modified, o and c /e are old formats which are now depreciated
bool EncodeImageFolder(const string &ifoldername, const string &ffilename_, ostream &out)
{
	vector<string> FileList = GetFilenamesFromFolder(ifoldername);
	vector<bool> array, aChunk;
	string fullPath{}, mfoldername{};
	string ffilename = ffilename_;
	int w{}, inputSize{}, h{}, channels{}, bitcounter{}, NIL{}, bitI{}, stringI{}, offset{};

	mfoldername = ifoldername + "M";
	createFolder(mfoldername);
	if (ffilename == "")
	{
		cout << "Enter Encoding Filename:";
		cin >> ffilename;
	}
	if (!checkEx(ffilename))
		InvalidInputMessage("The File you specified does not exist or could not be found");
	ReadFileToArray(ffilename, array, out);
	inputSize = array.size();
	out << "ARRAY Size:" << array.size() << "bit/" << array.size() / 8 << "Bytes" endo;
	/*
	cout << "Filelist:";
	for (auto i : FileList)
	{
		cout << i endo;
	}
		*/
	out << "Calculating NIL..." endo; // NIL = Needed Image Load
	for (int i = 0; bitcounter < inputSize; i++)
	{

		if (i >= FileList.size())
			InvalidInputMessage("Not enough images to store data.\n Bitcounter:" + std::to_string(bitcounter) + ".\ninputSize:" + std::to_string(inputSize));

		fullPath = ifoldername + "\\" + FileList[i];
		stbi_info(fullPath.c_str(), &w, &h, &channels);

		bitcounter += (w * h * channels);
		NIL = i + 1;
	}
	out << "Expected NIL" << NIL << "/" << FileList.size() endo;
	for (size_t i = 0; i < NIL; i++)
	{
		out << "Iteration:" << i << "/" << NIL endo;
		fullPath = ifoldername + "\\" + FileList[i];
		unsigned char *img = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);
		out << "Assigning Chunk of Size:" << (w * h * channels) endo;
		int capacity = w * h * channels;
		int chunkSize = std::min((int)array.size() - offset, capacity);
		aChunk.assign(array.begin() + offset, array.begin() + chunkSize + offset);
		bitI = 0;
		stringI = 0;
		out << "Writing to Image (Memory)..." endo;
		WriteToImage(img, (w * h * channels), aChunk, out, bitI, stringI);
		out << "Actual Chunk Size:" << stringI endo;
		offset = offset + stringI;
		fullPath = mfoldername + "\\" + FileList[i].insert(FileList[i].length() - 4, 1, 'M');
		out << "Writing to Image (Disk)..." endo;
		stbi_write_png(fullPath.c_str(), w, h, channels, img, channels * w);
		out << "Freeing Memory..." endo;
		stbi_image_free(img);
	}

	return 0;
}

bool DecodeImageFolder(const string &mFoldername, const string &iFoldername_, ostream &out)
{
	unsigned char *imgO{};
	unsigned char *imgE{};
	int w{}, h{}, channels{}, stringI{}, bitI{};
	string iFoldername = iFoldername_;
	string fullPath{}, filename{};
	vector<string> eFileList{}, oFileList{};
	vector<bool> decoded{}, decodedBuffer{};
	if (iFoldername == "")
	{
		cout << "Enter Original Foldername:";
		cin >> iFoldername;
	}
	if (!checkEx(iFoldername))
		InvalidInputMessage("");

	eFileList = GetFilenamesFromFolder(mFoldername);
	oFileList = GetFilenamesFromFolder(iFoldername);

	CheckFilelists(oFileList, eFileList, out);

	fullPath = iFoldername + "\\" + oFileList[0];
	imgO = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);

	fullPath = mFoldername + "\\" + eFileList[0];
	imgE = stbi_load(fullPath.c_str(), &w, &h, &channels, 3);

	filename = ReadFilenameFromImageC(imgO, imgE, bitI, stringI, out);
	out << "Filename:" << filename endo;
	out << "Reading remaining Data from the first image..." endo;
	ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decoded, out);
	out << "Freeing memory.." endo;
	stbi_image_free(imgO);
	stbi_image_free(imgE);

	out << "Reading Data from the remaining Images..." endo;
	for (int i = 1; i < eFileList.size(); i++)
	{
		out << "Iteration:" << i << "/" << eFileList.size() endo;
		stringI = 0;
		bitI = 0;
		decodedBuffer.clear();

		imgO = stbi_load((iFoldername + "\\" + oFileList[i]).c_str(), &w, &h, &channels, 3);
		imgE = stbi_load((mFoldername + "\\" + eFileList[i]).c_str(), &w, &h, &channels, 3);

		ReadDataFromImageC(imgO, imgE, (w * h * channels), bitI, stringI, decodedBuffer, out);
		decoded.insert(decoded.end(), decodedBuffer.begin(), decodedBuffer.end());

		stbi_image_free(imgO);
		stbi_image_free(imgE);
	}
	out << "Size of Read data:" << decoded.size() endo;
	out << "Writing to File...";
	WriteBitsToFile(filename, decoded);
	return 0;
}

/* =========================================================6. WAV OPERATIONS =========================================================== */
bool EncodeWav(const string ifilename, const string &ffilename, ostream &out)
{
	// variables
	string mfilename = ifilename;
	mfilename.insert(mfilename.length() - 4, "M");
	int stringI;
	unsigned int channels;
	unsigned int sampleRate;
	drwav_uint64 totalPCMFrameCount;
	vector<bool> array;
	// load file to memory
	float *SampleData = drwav_open_file_and_read_pcm_frames_f32(ifilename.c_str(), &channels, &sampleRate, &totalPCMFrameCount, NULL);
	// Read File to array
	ReadFileToArray(ffilename, array, out);
	if (array.size() > totalPCMFrameCount * channels)
	{
		drwav_free(SampleData, NULL);
		InvalidInputMessage("The .wav does not have enough Space to store the File you specified.(Not enough Samples)");
	}
	out << "Writing to file (memory)" endo;
	for (size_t bitI = 0; stringI < array.size() && bitI < totalPCMFrameCount * channels; bitI++)
	{
		// check if the current bit is not a maximum Value or zero (adding or subtracting would generate a unwanted effect)
		if (std::abs(SampleData[bitI]) <= 0.9998f&&SampleData[bitI]!=0)
		{
			if (array[stringI] == 0)
				SampleData[bitI] = SampleData[bitI] - 0.0001f;
			else
				SampleData[bitI] = SampleData[bitI] + 0.0001f;
			stringI++;
		}
	}

	// Check if the file fitted
	if (stringI < array.size())
	{
		drwav_free(SampleData, NULL);
		InvalidInputMessage("The .wav does not have enough Space to store the File you specified.(Too many loud or quiet samples) ");
	}
	out << "Writing to File (disk)" endo;
	// write file
	drwav pWav;
	drwav_data_format format;
	format.container = drwav_container_riff;
	format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
	format.channels = channels;
	format.sampleRate = sampleRate;
	format.bitsPerSample = 32;
	drwav_init_file_write(&pWav, mfilename.c_str(), &format, NULL);
	if (&pWav != NULL)
	{
		drwav_write_pcm_frames(&pWav, totalPCMFrameCount, SampleData);
		drwav_free(SampleData, NULL);
		return 0;
	}
	else
	{
		drwav_free(SampleData, NULL);
		InvalidInputMessage("Coudnt write to Output File (" + mfilename + ")");
	}
	return 0;
}

bool DecodeWav(const string& mfilename, const string& ifilename, ostream& out)
{
	//variables
	string ffilename;
    unsigned int mchannels,ichannels;
	unsigned int mSampleRate,iSampleRate;
	drwav_uint64 mTotalPCMFrameCount,iTotalPCMFrameCount;
	vector<bool> array;
	array.clear();
	int bitI=0;
	int stringI=0;
	// load file to memory
	out<<"Loading files into memory" endo;
	float *mSampleData = drwav_open_file_and_read_pcm_frames_f32(mfilename.c_str(), &mchannels, &mSampleRate, &mTotalPCMFrameCount, NULL);
	float *iSampleData = drwav_open_file_and_read_pcm_frames_f32(ifilename.c_str(), &ichannels, &iSampleRate, &iTotalPCMFrameCount, NULL);
    if(mTotalPCMFrameCount!=iTotalPCMFrameCount)
	{
		drwav_free(mSampleData,NULL);
		drwav_free(iSampleData,NULL);
		InvalidInputMessage("Frame Count Does not Match, suggesting diffrent or corrupted Files\n modifiedTotalPCmFrameCount:"+ts(mTotalPCMFrameCount)+"\ninto/originalTotalPCmFramCount:"+ts(iTotalPCMFrameCount));
	}
	out<<"Decoding filename" endo;
    ffilename=ReadFilenameFromWavC(mSampleData,iSampleData,bitI,stringI,out);
	out<<"Decoding rest" endo;
	ReadDataFromWavC(mSampleData,iSampleData,bitI,stringI,array,out);
	out<<"Writing to file (disk)" endo;
	WriteBitsToFile(ffilename,array);
	return 0;
}