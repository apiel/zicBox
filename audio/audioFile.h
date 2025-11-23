/** Description:
This piece of software code acts as a specialized digital audio file cutter or segment extractor. It is built primarily around libraries designed for reading and writing various sound file formats.

The central task of this code is defined in one key operation: copying a defined portion of an audio file.

**How it Works:**

1.  **Setup and Inspection:** The function is given the path to the original sound file, the path for the new clip, and specific start and end points (measured in audio frames, which are tiny blocks of sound data).
2.  **Validation:** It first attempts to open the source file. If the file cannot be accessed or read, the process stops immediately, and an error is recorded using the built-in logging system.
3.  **Seeking:** Once the original file is confirmed, the program jumps directly to the designated starting point within the audio data.
4.  **Copying:** It opens the new destination file, ensuring it uses the exact same format and quality settings as the original. It then transfers the audio data in small, efficient blocks, reading a chunk from the source and writing it immediately to the destination.
5.  **Completion:** This block-by-block transfer continues until the required number of frames (up to the end point) has been copied. Finally, both files are closed, confirming the new, partial audio clip has been successfully created.

In essence, it guarantees a safe, precise transfer of a selected audio segment from one file to another.

sha: e42331bd66c3faad0ad0800bbde68f53c02f14ef3209de83241a8daaa0d35ff3 
*/
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
