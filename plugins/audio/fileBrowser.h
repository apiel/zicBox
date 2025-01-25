#ifndef FILE_BROWSER_H
#define FILE_BROWSER_H

#include "helpers/range.h"
#include "helpers/fs/directoryList.h"
// #include <dirent.h>
// #include <stdint.h>
// #include <stdio.h>
// #include <string.h>
// #include <vector>
// #include <algorithm>

class FileBrowser {
protected:
    std::string folder;
    std::vector<std::filesystem::path> files;

    std::filesystem::path get(uint16_t pos)
    {
        position = range(pos, 1, files.size());
        return files.at(position - 1);
    }

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

    void openFolder(std::string _folder, DirectoryListOptions options = { .skipFolder = true, .skipHidden = true })
    {
        folder = _folder;
        files = getDirectoryList(folder, options);
        count = files.size();
    }

    std::string getFilePath(uint16_t pos)
    {
        return get(position);
    }

    std::string getFile(uint16_t pos)
    {
        return get(pos).filename();
    }

    std::string getFileWithoutExtension(uint16_t pos)
    {
        std::string file = getFile(pos);
        return file.substr(0, file.find_last_of("."));
    }

    bool find(std::string filename)
    {
        for (int i = 0; i < count; i++) {
            if (files[i].filename() == filename) {
                position = i + 1;
                return true;
            }
        }
        position = 0;
        return false;
    }
};

#endif