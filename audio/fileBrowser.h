/** Description:
This code defines a specialized utility class named `FileBrowser`. Its primary function is to manage and navigate the contents of a specific folder on the file system, providing a structured way for an application to access files.

**How It Works:**

1.  **Initialization:** When you create a `FileBrowser`, you typically provide a folder path. The tool then immediately scans that directory using internal helper functions.
2.  **Directory Reading:** By default, when it scans the folder, it intelligently skips over any sub-folders and hidden files, creating a clean, internal list of only the usable files. It also tracks the total number of files found.
3.  **Safe Access:** The class ensures safe access to the file list. If you request the file at position 10, but the folder only contains 5 files, the system automatically adjusts the request to prevent errors.
4.  **Retrieval Functions:** You can easily retrieve files based on their numbered position in the list. Available options include getting the full file path, just the filename, or the filename stripped of its extension (like removing `.png` or `.txt`).
5.  **Navigation:** The browser includes search capabilities to locate a file by its name and return its exact list position. It can also calculate the position of the file immediately before or after a currently selected file, making it useful for simple sequence navigation in applications like media players or asset loaders.

In essence, the `FileBrowser` acts as a robust and efficient index card system for a directory, allowing programs to interact with file lists predictably and safely.

sha: 71743422d7d17e318c874a75da19a263a6b4d0203814e3f2f30ff0ca342c4cb1 
*/
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
