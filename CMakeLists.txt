cmake_minimum_required(VERSION 3.16)
project(qpa_st7735 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

include(GNUInstallDirs)

add_subdirectory(lib)

find_package(Qt6 6.4 REQUIRED COMPONENTS Gui)
add_subdirectory(src)
