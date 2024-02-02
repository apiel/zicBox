#ifndef FILE_BROWSER_H
#define FILE_BROWSER_H

#include "../../helpers/range.h"
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>

class FileBrowser {
protected:
    std::string folder;
    std::vector<std::string> files;

public:
    uint16_t position = 0;
    uint16_t count = 0;

    FileBrowser(std::string _folder)
    {
        openFolder(_folder);
    }

    FileBrowser()
    {
    }

    void openFolder(std::string _folder)
    {
        folder = _folder;
        files.clear();
        DIR* dir = opendir(folder.c_str());
        if (dir != NULL) {
            struct dirent* directory;
            count = 0;
            while ((directory = readdir(dir)) != NULL) {
                if (directory->d_name[0] != '.') { // Ignore all file starting with '.'
                    files.push_back(directory->d_name);
                    count++;
                }
            }
            closedir(dir);

            std::sort(files.begin(), files.end());
        }
    }

    std::string getFilePath(uint16_t pos)
    {
        return folder + "/" + getFile(pos);
    }

    std::string getFile(uint16_t pos)
    {
        position = range(pos, 0, count - 1);
        return files[position];
    }
};

#endif