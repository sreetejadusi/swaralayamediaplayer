cmake_minimum_required(VERSION 3.16)
project(SwaralayaMediaPlayer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find Qt 6
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Network OpenGL OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Network OpenGL OpenGLWidgets)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Source files
set(SOURCES
    src/main.cpp
    src/Player/PlayerController.cpp
    src/UI/MainWindow.cpp
    src/UI/VideoWidget.cpp
    src/Settings/Settings.cpp
    src/resources.qrc
)

set(HEADERS
    src/Player/PlayerController.h
    src/UI/MainWindow.h
    src/UI/VideoWidget.h
    src/Settings/Settings.h
)

if(WIN32)
    list(APPEND SOURCES src/resources.rc)
    # We expect mpv headers and lib to be in third_party/mpv
    set(MPV_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/mpv")
    set(MPV_LIBRARY "${CMAKE_SOURCE_DIR}/third_party/mpv/mpv.lib")
    include_directories(${MPV_INCLUDE_DIR})
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MPV REQUIRED mpv)
    include_directories(${MPV_INCLUDE_DIRS})
    link_directories(${MPV_LIBRARY_DIRS})
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
endif()


if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Network
        ${MPV_LIBRARY}
    )

    # Copy libmpv DLL to output directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/third_party/libmpv-2.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libmpv-2.dll"
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::OpenGL
        Qt${QT_VERSION_MAJOR}::OpenGLWidgets
        ${MPV_LIBRARIES}
    )

    # Installation instructions for Linux
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
    install(FILES ${CMAKE_SOURCE_DIR}/swaralayamediaplayer.desktop DESTINATION share/applications)
    install(FILES ${CMAKE_SOURCE_DIR}/src/app.png DESTINATION share/icons/hicolor/256x256/apps RENAME swaralayamediaplayer.png)

    # CPack setup for .deb
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "swaralayamediaplayer")
    set(CPACK_PACKAGE_VERSION "1.0.0")
    set(CPACK_PACKAGE_CONTACT "developer@example.com")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Swaralaya Media Player")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    
    # We will provide a postinst script to set it as default
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/postinst")

    include(CPack)
endif()
