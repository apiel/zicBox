/** Description:
This code defines a comprehensive digital tool called a `FileBrowser`, designed to manage and navigate the contents of a specific folder on a computer system.

**Core Functionality:**
The File Browser acts like a specialized librarian for a directory. When instructed to `openFolder`, it reads all the files within that location, creating an organized, numbered list. By default, it typically ignores sub-folders and hidden system files, focusing purely on accessible data files.

**Managing Files:**
The tool stores this list of files internally, tracking both the total number of files (`count`) and the current selected file position.

**Access and Retrieval:**
Users interact with this tool by asking for files based on their position number. For example, you can request the file at position 3. The underlying logic automatically validates this number to ensure it stays within the list's boundaries, preventing errors.

The File Browser offers several ways to retrieve information:
1. The complete location (path) of the file.
2. Just the file name (e.g., `image.jpg`).
3. The file name without its extension (e.g., `image`).

**Advanced Navigation:**
It includes powerful search capabilities. You can search for a specific filename to instantly find its position in the list. Additionally, navigation features allow calculating the position of the file immediately preceding or following a known file, streamlining sequential browsing. Features like `peek` allow checking file details without affecting the tool’s internal tracked position.

Tags: File Management, Directory Navigation, Path Handling, System Utility, File Listing
sha: 14467d8a99820f916e1e4cdb3b576799b72f2791b3d3df4e5ebe217a1e9150f1 
*/
#pragma once

#include "helpers/clamp.h"
#include "helpers/fs/directoryList.h"

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

    std::filesystem::path peek(uint16_t pos) const
    {
        if (pos < 1 || pos > files.size())
            return {};
        return files[pos - 1];
    }

    std::string peekFile(uint16_t pos) const
    {
        auto p = peek(pos);
        return p.empty() ? "" : p.filename().string();
    }

    std::string peekFileWithoutExtension(uint16_t pos) const
    {
        std::string file = peekFile(pos);
        auto dot = file.find_last_of(".");
        return dot == std::string::npos ? file : file.substr(0, dot);
    }
};
