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