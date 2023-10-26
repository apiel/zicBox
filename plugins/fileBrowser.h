#ifndef FILE_BROWSER_H
#define FILE_BROWSER_H

#include "../helpers/range.h"
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FILE_BROWSER_FILES_MAX 127
#define FILE_BROWSER_FILEPATH_LEN 258
#define FILE_BROWSER_FILENAME_LEN 255

class FileBrowser {
protected:
    const char* folder;
    char filepath[FILE_BROWSER_FILEPATH_LEN];
    char files[FILE_BROWSER_FILES_MAX][FILE_BROWSER_FILENAME_LEN];

    void sort()
    {
        uint8_t i, j;
        char tmp[FILE_BROWSER_FILENAME_LEN];
        for (i = 0; i < count - 1; i++) {
            for (j = i + 1; j < count; j++) {
                if (strcmp(files[i], files[j]) > 0) {
                    strncpy(tmp, files[i], FILE_BROWSER_FILENAME_LEN);
                    strncpy(files[i], files[j], FILE_BROWSER_FILENAME_LEN);
                    strncpy(files[j], tmp, FILE_BROWSER_FILENAME_LEN);
                }
            }
        }
    }

public:
    uint8_t position = 0;
    uint8_t count = 0;

    FileBrowser(const char* _folder)
    {
        openFolder(_folder);
    }

    FileBrowser()
    {
    }

    void openFolder(const char* _folder)
    {
        folder = _folder;
        DIR* dir = opendir(folder);
        if (dir != NULL) {
            struct dirent* directory;
            count = 0;
            while ((directory = readdir(dir)) != NULL && count < FILE_BROWSER_FILES_MAX) {
                if (strcmp(directory->d_name, ".") != 0 && strcmp(directory->d_name, "..") != 0) {
                    strncpy(files[count], directory->d_name, FILE_BROWSER_FILENAME_LEN);
                    count++;
                }
            }
            closedir(dir);
        }

        sort();

        // getFilePath(position);
    }

    char* getFilePath(uint8_t pos)
    {
        position = range(pos, 0, count - 1);
        snprintf(filepath, FILE_BROWSER_FILEPATH_LEN, "%s/%s", folder, files[position]);
        return filepath;
    }

    char* getFile(uint8_t pos)
    {
        position = range(pos, 0, count - 1);
        return files[position];
    }
};

#endif