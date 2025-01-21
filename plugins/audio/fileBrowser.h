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
        position = range(pos, 0, count - 1);
        return files[position];
    }

    std::string getFile(uint16_t pos)
    {
        position = range(pos, 0, count - 1);
        return files[position].filename();
    }

    std::string getFileWithoutExtension(uint16_t pos)
    {
        std::string file = getFile(pos);
        return file.substr(0, file.find_last_of("."));
    }
};

#endif