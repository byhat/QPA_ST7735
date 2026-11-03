cmake_minimum_required(VERSION 3.16)
project(qpa_st7735 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

include(GNUInstallDirs)

add_subdirectory(lib)

find_package(Qt6 6.4 COMPONENTS Gui)
if(Qt6_FOUND)
    add_subdirectory(src)
    option(QPA_ST7735_BUILD_QML "Build the QML demo application" ON)
    if(QPA_ST7735_BUILD_QML)
        add_subdirectory(qml-app)
    endif()
else()
    message(STATUS "Qt6 не найден: строятся только библиотека и пример")
endif()
