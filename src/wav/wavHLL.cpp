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
#include <algorithm>
#include <sndfile.h>
#include <stacktrace>
#include <source_location>
#include <cmath>
#include <chrono>
#include <iomanip>
#include "../core/defs.h"
#include "../core/utils.h"
#include "../wav/wavLLL.h"
bool EncodeWav(const std::string &ifilename, const std::string &ffilename, stateClass &state)
{
    state.out("Init", 1);
    std::string mfilename = ifilename;
    mfilename.insert(mfilename.length() - 4, "M");
    int bitI = 0;
    int stringI = 0;
    sf_count_t totalSamples;
    sf_count_t originalFrames;
    sf_count_t framesRead;
    sf_count_t framesWritten;
    std::vector<short> buffer;
    std::vector<bool> fromarray;
    SF_INFO sfinfo;
    sfinfo.format = 0;

    state.out("Open Input", 1);
    SNDFILE *infline = sf_open(ifilename.c_str(), SFM_READ, &sfinfo);
    originalFrames = sfinfo.frames;
    if (!infline)
        InvalidInputMessage("Could not open input file");

    state.out("Details:", 4);
    state.out("Sample Rate:" + std::to_string(sfinfo.samplerate) + "Hz", 4);
    state.out("Channels   :" + std::to_string(sfinfo.channels), 4);
    state.out("Frames     :" + std::to_string(sfinfo.frames), 4);

    totalSamples = sfinfo.frames * sfinfo.channels;
    state.out("Resize Buffer", 4);
    buffer.resize(totalSamples);

    state.out("Read SF", 1);
    framesRead = sf_readf_short(infline, buffer.data(), sfinfo.frames);
    if (framesRead != sfinfo.frames)
        InvalidInputMessage("Read Frame Count does not match expected Frame Count\nRead Frame Count" + ts(framesRead) + "\nExpected:" + ts(sfinfo.frames));
    sf_close(infline);

    state.out("Read Payload", 1);
    ReadFileToArray(ffilename, fromarray, state);
    if (fromarray.size() >= totalSamples)
        InvalidInputMessage("\nThe File specified does not contain enough space to encode\nCapacity:" + ts(totalSamples) + "\nFilesize:" + ts(fromarray.size()));

    state.out("Writing... (memory)", 1);
    while (bitI < totalSamples && stringI < fromarray.size())
    {
        if (std::abs(buffer[bitI]) < 32767 && buffer[bitI] != 0)
        {
            if (fromarray[stringI] == false)
            {
                buffer[bitI] = buffer[bitI] - 1;
            }
            else
            {
                buffer[bitI] = buffer[bitI] + 1;
            }
            stringI++;
        }
        bitI++;
    }

    state.out("Open Output", 1);
    SNDFILE *outfile = sf_open(mfilename.c_str(), SFM_WRITE, &sfinfo);
    if (!outfile)
        InvalidInputMessage("Could not open output File");

    state.out("Writing ... (disk)", 1);
    framesWritten = sf_writef_short(outfile, buffer.data(), originalFrames);
    if (framesWritten != originalFrames)
        InvalidInputMessage("Written Frame Count does not match expected Frame Count");
    sf_close(outfile);

    state.out("Complete", 1);
    return 0;
}

bool DecodeWav(const std::string &mFilename, const std::string &iFilename, stateClass &state)
{
    state.out("Init", 1);
    int bitI = 0;
    int stringI = 0;
    std::string ffilename;
    std::vector<short> mbuffer;
    std::vector<short> ibuffer;
    std::vector<bool> decoded;
    SF_INFO msfInfo;
    SF_INFO isfInfo;

    state.out("Open Mod", 1);
    SNDFILE *mfile = sf_open(mFilename.c_str(), SFM_READ, &msfInfo);
    state.out("Open Orig", 1);
    SNDFILE *ifile = sf_open(iFilename.c_str(), SFM_READ, &isfInfo);
    if (!mfile || !ifile)
        InvalidInputMessage("Could not open files.");

    state.out("Resize Buffers", 4);
    mbuffer.resize(msfInfo.channels * msfInfo.frames);
    ibuffer.resize(isfInfo.channels * isfInfo.frames);

    state.out("Read SF", 1);
    sf_readf_short(mfile, mbuffer.data(), msfInfo.frames);
    sf_readf_short(ifile, ibuffer.data(), isfInfo.frames);
    if (mbuffer.empty() || ibuffer.empty())
        InvalidInputMessage("Could not read Files to Memory");
    state.out("Read Files successfully into Memory", 1);

    state.out("Validate Sizes", 1);
    if (msfInfo.frames * msfInfo.channels != isfInfo.frames * isfInfo.channels)
        InvalidInputMessage("Sample amounts do not match, indicating file corruption or wrong file selection");
    state.out("Sample sizes match:\nmSamplesize:" + std::to_string(msfInfo.frames * msfInfo.channels) + "\niSampleSize:" + std::to_string(isfInfo.frames * isfInfo.channels), 4);

    state.out("Find Non-Zero", 4);
    for (; bitI < mbuffer.size(); ++bitI)
    {
        if (mbuffer[bitI] != 0)
        {
            break;
        }
    }
    state.out("First non zero Sample at:" + std::to_string(bitI), 4);

    state.out("First ten samples:", 4);
    for (size_t i = bitI; i < 10 + bitI; i++)
    {
        state.out("n:" + std::to_string(i) + " | mbuffer:" + std::to_string(mbuffer[i]) + " | ibuffer:" + std::to_string(ibuffer[i]), 4);
    }

    state.out("Read Filename", 1);
    ffilename = ReadFilenameFromWavC(mbuffer, ibuffer, bitI, stringI, state);
    state.out("Decoded filename:" + ffilename, 1);

    state.out("Read Data", 1);
    ReadDataFromWavC(mbuffer, ibuffer, bitI, stringI, decoded, state);

    state.out("Writing to file", 1);
    WriteBitsToFile(ffilename, decoded, state);

    state.out("Complete", 1);
    return 0;
}

bool EncodeWavFolder(const std::string &ifoldername, const std::string &ffilename, stateClass &state)
{
    state.out("Starting...", 4);
    std::string absolutePath;
    std::string mfoldername = ifoldername + "M";
    std::string command = "mkdir " + mfoldername;
    state.out("trying:'" + command + "'", 4);
    system(command.c_str());
    std::vector<std::string> FileList = GetFilenamesFromFolder(ifoldername, state);
    const std::vector<std::string> FileListOriginal = GetFilenamesFromFolder(ifoldername, state);
    std::vector<bool> farray;
    std::vector<bool> aChunk;
    std::vector<short> buffer;
    int NIL;
    int bitcounter = 0;
    int stringI = 0;
    int bitI;
    sf_count_t originalFrames;
    sf_count_t totalSamples;
    sf_count_t framesRead;
    sf_count_t framesWritten;
    ReadFileToArray(ffilename, farray, state);
    SF_INFO sfinfo;
    sfinfo.format = 0;
    SNDFILE *infline = nullptr;
    state.out("Calculating NIL...", 4);
    for (NIL = 0; bitcounter < farray.size(); NIL++)
    {
        state.out("Iteration:" + ts(NIL) + "/" + ts(FileList.size()) + "(MAX)", 4);

        absolutePath = ifoldername + "\\" + FileList[NIL];
        state.out("Opening file:" + absolutePath, 1);
        infline = sf_open(absolutePath.c_str(), SFM_READ, &sfinfo);
        originalFrames = sfinfo.frames;
        if (!infline)
            InvalidInputMessage("Could not open input file");

        state.out("Details:", 4);
        state.out("Sample Rate:" + std::to_string(sfinfo.samplerate) + "Hz", 4);
        state.out("Channels   :" + std::to_string(sfinfo.channels), 4);
        state.out("Frames     :" + std::to_string(sfinfo.frames), 4);

        totalSamples = sfinfo.frames * sfinfo.channels;
        state.out("Resizing buffer to:" + ts(totalSamples), 4);
        buffer.resize(totalSamples);
        state.out("Size of buffer:" + ts(buffer.size()), 4);
        state.out("Reading file to memory", 1);
        framesRead = sf_readf_short(infline, buffer.data(), sfinfo.frames);
        if (framesRead != sfinfo.frames)
            InvalidInputMessage("Read Frame Count does not match expected Frame Count\nRead Frame Count" + ts(framesRead) + "\nExpected:" + ts(sfinfo.frames));
        sf_close(infline);
        state.out("Sucessfully read file to memory", 4);
        bitcounter = bitcounter + CheckWavFileCapacityBackend(buffer, state);
        state.out("Bitcounter:" + ts(bitcounter), 4);
        state.out("From File size (in bits:)" + ts(farray.size()), 4);
    }

    for (int i = 0; i < NIL; i++)
    {
        absolutePath = ifoldername + "\\" + FileList[i];
        state.out("Iteration:" + ts(i) + "/" + ts(NIL), 4);
        state.out("StringI:" + ts(stringI), 4);
        state.out("Opening file:" + absolutePath, 1);
        infline = sf_open(absolutePath.c_str(), SFM_READ, &sfinfo);
        originalFrames = sfinfo.frames;
        if (!infline)
            InvalidInputMessage("Could not open input file");

        state.out("Details:", 4);
        state.out("Sample Rate:" + std::to_string(sfinfo.samplerate) + "Hz", 4);
        state.out("Channels   :" + std::to_string(sfinfo.channels), 4);
        state.out("Frames     :" + std::to_string(sfinfo.frames), 4);

        totalSamples = sfinfo.frames * sfinfo.channels;
        state.out("Resizing buffer...", 4);
        buffer.resize(totalSamples);

        state.out("Read File to memory", 1);
        framesRead = sf_readf_short(infline, buffer.data(), sfinfo.frames);
        if (framesRead != sfinfo.frames)
            InvalidInputMessage("Read Frame Count does not match expected Frame Count\nRead Frame Count" + ts(framesRead) + "\nExpected:" + ts(sfinfo.frames));
        sf_close(infline);
        state.out("Writing to file (memory)...", 4);
        state.out("stringI:" + ts(stringI), 4);
        state.out("buffer size:" + ts(buffer.size()), 4);
        state.out("farray size:" + ts(farray.size()), 4);
        for (bitI = 0; bitI < buffer.size() && stringI < farray.size(); bitI++)
        {
            if (std::abs(buffer[bitI]) < 32767 && buffer[bitI] != 0)
            {
                if (farray[stringI] == false)
                {
                    buffer[bitI] = buffer[bitI] - 1;
                }
                else
                {
                    buffer[bitI] = buffer[bitI] + 1;
                }
                stringI++;
            }
        }
        absolutePath = mfoldername + "\\" + FileList[i].insert(FileList[i].length() - 4, 1, 'M');
        command = "type NUL > " + absolutePath;
        system(command.c_str());
        state.out("Opening file:" + absolutePath, 4);
        SNDFILE *outfile = sf_open(absolutePath.c_str(), SFM_WRITE, &sfinfo);
        if (!outfile)
            InvalidInputMessage("Could not open output File. sf_strerror:" + std::string(sf_strerror(NULL)));

        state.out("Writing ... (disk)", 1);
        framesWritten = sf_writef_short(outfile, buffer.data(), originalFrames);
        if (framesWritten != originalFrames)
            InvalidInputMessage("Written Frame Count does not match expected Frame Count");
        sf_close(outfile);
    }

    if (state.deleteOverflow)
    {
        for (int i = NIL; i < FileListOriginal.size(); i++)
        {

            absolutePath = ifoldername + "\\" + FileListOriginal[i];
            state.out("Deleting file:" + absolutePath, 1);
            if (remove(absolutePath.c_str()) != 0)
                InvalidInputMessage("Failed to delete file");
        }
    }
    return 0;
}

bool DecodeWavFolder(const std::string &mfoldername, const std::string ifoldername, stateClass &state)
{
    state.out("Starting...", 4);
    // Verify files
    std::vector<std::string> mFileList = GetFilenamesFromFolder(mfoldername, state);
    std::vector<std::string> iFileList = GetFilenamesFromFolder(ifoldername, state);
    CheckFilelists(iFileList, mFileList, state);

    // Start actual reading
    int bitI = 0;
    int stringI = 0;
    std::string ffilename;
    std::vector<short> mbuffer;
    std::vector<short> ibuffer;
    std::vector<bool> decoded;
    std::string filename;
    std::string absolutePath;
    SF_INFO msfInfo;
    SF_INFO isfInfo;
    SNDFILE *mfile = nullptr;
    SNDFILE *ifile = nullptr;
    // readFilename
    absolutePath=mfoldername+"\\"+mFileList[0];
    mfile = sf_open(absolutePath.c_str(), SFM_READ, &msfInfo);
    absolutePath=ifoldername+"\\"+iFileList[0];
    ifile = sf_open(absolutePath.c_str(), SFM_READ, &isfInfo);
    mbuffer.resize(msfInfo.channels * msfInfo.frames);
    ibuffer.resize(isfInfo.channels * isfInfo.frames);
    sf_readf_short(mfile, mbuffer.data(), msfInfo.frames);
    sf_readf_short(ifile, ibuffer.data(), isfInfo.frames);
    if (mbuffer.empty() || ibuffer.empty())
        InvalidInputMessage("Could not read Files to Memory");
    state.out("Validate Sizes", 1);
    if (msfInfo.frames * msfInfo.channels != isfInfo.frames * isfInfo.channels)
        InvalidInputMessage("Sample amounts do not match, indicating file corruption or wrong file selection");
        
    
    filename = ReadFilenameFromWavC(mbuffer, ibuffer, bitI, stringI, state);
    // read Rest of the remainign first file
    ReadDataFromWavC(mbuffer, ibuffer, bitI, stringI, decoded, state);

    // loop the rest of the files
    for (int i = 1; i <= iFileList.size(); i++)
    {
        state.out("Interation:"+ts(i)+"/"+ts(iFileList.size()),4);
        mfile = sf_open(mFileList[i].c_str(), SFM_READ, &msfInfo);
        ifile = sf_open(iFileList[i].c_str(), SFM_READ, &isfInfo);
             mbuffer.resize(msfInfo.channels * msfInfo.frames);
        ibuffer.resize(isfInfo.channels * isfInfo.frames);
        sf_readf_short(mfile, mbuffer.data(), msfInfo.frames);
        sf_readf_short(ifile, ibuffer.data(), isfInfo.frames);
        if (mbuffer.empty() || ibuffer.empty())
        InvalidInputMessage("Could not read Files to Memory");
        state.out("Validate Sizes", 1);
        if (msfInfo.frames * msfInfo.channels != isfInfo.frames * isfInfo.channels)
            InvalidInputMessage("Sample amounts do not match, indicating file corruption or wrong file selection");
   
        ReadDataFromWavC(mbuffer, ibuffer, bitI, stringI, decoded, state);
    }

  
    state.out("Writing to file", 1);
    WriteBitsToFile(ffilename, decoded, state);

    state.out("Complete", 1);
    return 0;
}