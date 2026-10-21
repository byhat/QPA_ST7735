find_package(Qt6 6.4 REQUIRED COMPONENTS Quick)

add_executable(qml-st7735 main.cpp)
target_link_libraries(qml-st7735 PRIVATE Qt6::Quick)

if(QPA_ST7735_BUILD_QML_INSTALL)
    install(TARGETS qml-st7735 DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES Main.qml DESTINATION ${CMAKE_INSTALL_DATADIR}/qpast7735)
endif()
