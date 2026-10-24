add_library(qpast7735 SHARED
    Plugin/Plugin.cpp
    Platform/Integration.cpp
    Platform/Screen.cpp
    Platform/Window.cpp
    Platform/Cursor.cpp
    Platform/FramePresenter.cpp
    Platform/BackingStore.cpp
    Input/MouseDevice.cpp
    Input/KeyboardDevice.cpp
    Config/Config.cpp
)

target_include_directories(qpast7735 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Plugin
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform
    ${CMAKE_CURRENT_SOURCE_DIR}/Input
    ${CMAKE_CURRENT_SOURCE_DIR}/Config
)

target_link_libraries(qpast7735 PRIVATE Qt6::Gui Qt6::GuiPrivate st7735)

if(QT6_INSTALL_PLUGINS)
    install(TARGETS qpast7735 DESTINATION ${QT6_INSTALL_PLUGINS}/platforms)
    install(FILES ${CMAKE_SOURCE_DIR}/config/qpa_st7735.json
            DESTINATION ${CMAKE_INSTALL_DATADIR}/qpa_st7735/examples)
endif()
