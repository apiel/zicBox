#pragma once

#include <iostream>
#include <sndfile.h>
#include <string>

#include "log.h"

bool copyPartialAudioFile(const std::string& src, const std::string& dest, sf_count_t start, sf_count_t end)
{
    // Open the source file
    SF_INFO sfinfo;
    SNDFILE* infile = sf_open(src.c_str(), SFM_READ, &sfinfo);
    if (!infile) {
        logError("Could not open input file '%s' (%s)", src.c_str(), sf_strerror(nullptr));
        return false;
    }

    if (start < 0) {
        start = 0;
    }
    if (end <= start) {
        end = sfinfo.frames;
    }
    sf_count_t frames = end - start;
    sf_seek(infile, start, SEEK_SET);
    printf("copyPartialAudioFile start %ld end %ld frames %ld\n", start, end, frames);

    // Open the output file
    SNDFILE* outfile = sf_open(dest.c_str(), SFM_WRITE, &sfinfo);
    if (!outfile) {
        logError("Could not open output file '%s' (%s)", dest.c_str(), sf_strerror(nullptr));
        sf_close(infile);
        return false;
    }
    const size_t bufferSize = 1024;
    float buffer[bufferSize];
    while (frames > 0) {
        sf_count_t count = sf_read_float(infile, buffer, bufferSize);
        if (count <= 0) {
            break;
        }
        sf_write_float(outfile, buffer, count);
        frames -= count;
    }

    sf_close(infile);
    sf_close(outfile);
    return true;
}
