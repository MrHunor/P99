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
#include "src/core/defs.h"
#include "src/core/utils.h"
#include "src/wav/wavLLL.h"
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