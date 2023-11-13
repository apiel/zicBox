#ifndef _FS_DIRECTORY_LIST_H_
#define _FS_DIRECTORY_LIST_H_

#include <filesystem>

#include <vector>
#include <algorithm>

std::vector<std::filesystem::path> getDirectoryList(std::filesystem::path folder)
{
    std::vector<std::filesystem::path> list;
    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (entry.is_directory()) {
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