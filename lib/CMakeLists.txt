# Мини-либа драйвера ST7735: собирается и отдельно, без Qt
# (cmake -B build-lib -S lib), и как часть проекта с плагином.
cmake_minimum_required(VERSION 3.16)
project(st7735 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_library(st7735 STATIC
    hal/SpidevBus.cpp
    hal/SysfsGpioLine.cpp
    display/St7735Driver.cpp
)
set_property(TARGET st7735 PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(st7735 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/hal
    ${CMAKE_CURRENT_SOURCE_DIR}/display
)
target_link_libraries(st7735 PUBLIC Threads::Threads)

add_subdirectory(example)
