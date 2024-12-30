/*md
> Find an example on how to use the host embedded in your own application: `use_host_demo.cpp`.
*/

// Build with: `g++ -o demo use_host_demo.cpp`

#include <dlfcn.h>
#include <stdio.h>

int main()
{
    const char* path = "./zicHost.x86.so";
    printf("Loading host from %s\n", path);
    void* handle = dlopen(path, RTLD_LAZY);
    if (handle == NULL) {
        printf("Failed to load %s: %s\n", path, dlerror());
        return 1;
    }
    printf("Loaded %s\n", path);
    dlclose(handle);
}