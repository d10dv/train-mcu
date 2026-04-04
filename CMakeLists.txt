# The following lines of boilerplate have to be in your project's CMakeLists.txt
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# Use absolute path for extra component directories
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(train-mcu)
