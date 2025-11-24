/** Description:
This file serves as a utility for interacting with the computer's file system, specifically designed to read and organize the contents of a directory (folder). It acts like a customizable scanning tool embedded within a larger software application.

### Key Functionality

The core mechanism involves scanning a specified folder and generating an organized list of the files and sub-folders found inside.

### Customizable Filtering

The process uses a set of `Options` to allow the user to control exactly what appears in the final output list. These options act as simple filters or switches:

1.  **Skip Folders:** Excludes any sub-directories found during the scan.
2.  **Skip Files:** Excludes all individual files.
3.  **Skip Hidden Items:** Ignores operating system items usually marked as hidden (often those starting with a dot or period).

### The Scanning Process

The main function receives the starting folder and the chosen options. It systematically checks every item in that folder against the rules:

*   It applies the customizable filters (e.g., if you choose to skip files, those are ignored).
*   It automatically ignores a specific placeholder file named `.keep`.

Any file or folder that successfully passes all the required filters is collected. Once the collection is complete, the entire list is sorted alphabetically to ensure a consistent and predictable order before it is returned to the main program.

sha: 43936b9508e78549376886d9d9869a130cab0574d671e6595b7e23d1aaa90ff1 
*/
#ifndef _FS_DIRECTORY_LIST_H_
#define _FS_DIRECTORY_LIST_H_

#include <filesystem>

#include <vector>
#include <algorithm>

struct DirectoryListOptions {
    bool skipFolder = false;
    bool skipFiles = false;
    bool skipHidden = false;
};

std::vector<std::filesystem::path> getDirectoryList(std::filesystem::path folder, DirectoryListOptions options = {})
{
    std::vector<std::filesystem::path> list;
    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (options.skipFolder && entry.is_directory()) {
            continue;
        }

        if (options.skipFiles && !entry.is_directory()) {
            continue;
        }

        if (options.skipHidden && entry.path().filename().string().at(0) == '.') {
            continue;
        }

        if (entry.path().filename() == ".keep") {
            continue;
        }

        list.push_back(entry.path());
    }

    std::sort(list.begin(), list.end());

    return list;
}

// #include <set>
//
// std::set<std::filesystem::path> getDirectorySet(std::filesystem::path folder)
// {
//     std::set<std::filesystem::path> set;
//     for (const auto& entry : std::filesystem::directory_iterator(folder)) {
//         if (entry.is_directory()) {
//             continue;
//         }
//
//         if (entry.path().filename() == ".keep") {
//             continue;
//         }
//
//         set.insert(entry.path().filename());
//     }
//     return set;
// }

#endif