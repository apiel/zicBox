#ifndef _FS_DIRECTORY_SET_H_
#define _FS_DIRECTORY_SET_H_

#include <filesystem>
#include <set>

std::set<std::filesystem::path> getDirectorySet(std::filesystem::path folder)
{
    std::set<std::filesystem::path> set;
    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (entry.is_directory()) {
            continue;
        }

        if (entry.path().filename() == ".keep") {
            continue;
        }

        set.insert(entry.path().filename());
    }
    return set;
}

#endif