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

add_subdirectory(example)
