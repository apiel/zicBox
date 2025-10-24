#pragma once

#include "helpers/fs/directoryList.h"
#include "helpers/clamp.h"

class FileBrowser {
protected:
    std::string folder;
    std::vector<std::filesystem::path> files;

    std::filesystem::path get(uint16_t pos)
    {
        position = CLAMP(pos, 1, files.size());
        return files.at(position - 1);
    }

public:
    uint16_t position = 0; // Keep it to 0, so we know it haven't been initialized, this is used in many engines to compare  with current position in the patch.
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
        return get(pos);
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

    uint16_t find(std::string filename)
    {
        filename = std::filesystem::path(filename).filename();
        for (int i = 0; i < count; i++) {
            if (files[i].filename() == filename) {
                return i + 1;
            }
        }
        return 0;
    }

    uint16_t next(std::string filename, int8_t direction = 1)
    {
        filename = std::filesystem::path(filename).filename();
        uint16_t pos = find(filename);
        return pos ? pos + direction : 0;
    }
};
