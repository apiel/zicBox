cmake_minimum_required(VERSION 3.16)

# 2. ADD THE CUSTOM COMPONENT DIRECTORY (Before including project.cmake)
# This tells the build system to search "esp/zicBlock" for components (like 'main')
set(EXTRA_COMPONENT_DIRS "esp/zicBlock")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
idf_build_set_property(MINIMAL_BUILD ON)
project(zicBlock)