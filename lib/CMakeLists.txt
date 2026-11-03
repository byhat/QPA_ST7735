# Мини-либа драйвера ST7735: собирается и отдельно, без Qt
# (cmake -B build-lib -S lib), и как часть проекта с плагином.
cmake_minimum_required(VERSION 3.16)
project(st7735 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

# WiringPi / WiringOP: заголовок и библиотека ставятся из репозитория
# библиотеки под конкретную плату (make install кладёт в /usr/local)
find_path(WIRINGPI_INCLUDE_DIR wiringPi.h
    PATHS /usr/local/include /usr/include)
find_library(WIRINGPI_LIBRARY wiringPi
    PATHS /usr/local/lib /usr/lib)
if(NOT WIRINGPI_INCLUDE_DIR OR NOT WIRINGPI_LIBRARY)
    message(FATAL_ERROR "wiringPi/WiringOP не найден: установите библиотеку для вашей платы")
endif()

add_library(st7735 STATIC
    hal/SpidevBus.cpp
    hal/WiringGpioLine.cpp
    display/St7735Driver.cpp
)
set_property(TARGET st7735 PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(st7735 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/hal
    ${CMAKE_CURRENT_SOURCE_DIR}/display
)
target_link_libraries(st7735 PUBLIC Threads::Threads ${WIRINGPI_LIBRARY})
target_include_directories(st7735 PUBLIC ${WIRINGPI_INCLUDE_DIR})

add_subdirectory(example)
