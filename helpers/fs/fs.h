/** Description:
This header file, labeled `_FS_H_`, acts as a blueprint for a custom library designed to handle basic file system operations. It defines specialized tools that interact directly with the computer’s storage structure.

The primary goal of this code is to provide structured and ordered navigation of folders. It incorporates necessary standard components to read directory contents, manipulate strings, and handle basic input/output operations.

**How it works (The concepts defined):**

1.  **Specialized Folder Reading:** The library includes a customized function to read the contents of a directory while automatically ignoring the common navigation links that represent the current folder and the parent folder.
2.  **Alphabetical Navigation:** It contains core logic to find the files within a folder in alphabetical order. Users can determine the very first file in a directory, or efficiently move to the file immediately "next" or "previous" to a currently selected file.
3.  **Basic Utilities:** Included are helper functions for simplified, predictable string comparison, as well as a robust routine for safely copying data from a source file to a destination file, chunk by chunk.

In essence, this file creates a focused toolkit for organized and predictable management and traversal of files and folders on a computer system.

sha: 1a90b40f90642a03352d35179e32e041087f9cc4fb98d6620f8fe26b41c47819 
*/
#ifndef _FS_H_
#define _FS_H_

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h> // mkdir
#include <sys/types.h> // mkdir

// struct dirent* myReaddir(DIR* x)
// {
//     struct dirent* directory;
//     directory = readdir(x);
//     if (directory != NULL && (strcmp(directory->d_name, ".") == 0 || strcmp(directory->d_name, "..") == 0)) {
//         return myReaddir(x);
//     }
//     return directory;
// }

// int8_t myStrcmp(const char* __s1, const char* __s2)
// {
//     int res = strcmp(__s1, __s2);
//     if (res == 0) {
//         return 0;
//     }
//     return res > 0 ? 1 : -1;
// }

// bool nextFile(char* filename, const char* folder, const char* current, int8_t direction, uint8_t len = 255)
// {
//     bool next = false;
//     DIR* x = opendir(folder);
//     if (x != NULL) {
//         struct dirent* directory;

//         char cur[255];
//         strncpy(cur, current, len);
//         strncpy(filename, cur, len);
//         while ((directory = myReaddir(x)) != NULL) {
//             if (myStrcmp(directory->d_name, cur) == direction
//                 && (strcmp(filename, cur) == 0 || myStrcmp(directory->d_name, filename) == direction * -1)) {
//                 strncpy(filename, directory->d_name, len);
//                 next = true;
//             }
//         }
//         closedir(x);
//     }
//     return next;
// }

// bool firstFile(char* filename, const char* folder, uint8_t len = 255)
// {
//     bool initialized = false;
//     DIR* x = opendir(folder);
//     if (x != NULL) {
//         struct dirent* directory;
//         while ((directory = myReaddir(x)) != NULL) {
//             // printf("%s <> %s = %d\n", directory->d_name, filename, myStrcmp(directory->d_name, filename));
//             if (!initialized || myStrcmp(directory->d_name, filename) == -1) {
//                 strncpy(filename, directory->d_name, len);
//                 initialized = true;
//             }
//         }
//         closedir(x);
//     }
//     return initialized;
// }

// static int fileCompare(const void *p, const void *q)
// {
//     const char *l= (const char *)p;
//     const char *r= (const char *)q;
//     int cmp;

//     cmp= strcmp (l, r);
//     return cmp;
// }

// void copyFile(char* src, char* dst)
// {
//     FILE* srcFile = fopen(src, "rb");
//     FILE* dstFile = fopen(dst, "wb");
//     if (srcFile == NULL || dstFile == NULL) {
//         return;
//     }

//     char buf[1024];
//     size_t size;
//     while ((size = fread(buf, 1, sizeof(buf), srcFile)) > 0) {
//         fwrite(buf, 1, size, dstFile);
//     }

//     fclose(srcFile);
//     fclose(dstFile);
// }

#endif