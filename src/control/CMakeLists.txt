cmake_minimum_required(VERSION 3.16)
project(ptz-camera LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia Qml Quick SerialPort WebSockets)

qt_add_executable(ptz-camera
    src/main.cpp
    src/processor.h
    src/processor.cpp
    src/mountdriver.h
    src/mountdriver.cpp
    src/server.h
    src/server.cpp
    src/streamer.h
    src/streamer.cpp
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTLIBS REQUIRED
    gstreamer-1.0>=1.22
    gstreamer-video-1.0>=1.22
    gstreamer-app-1.0>=1.22
    gstreamer-base-1.0>=1.22
)

set_target_properties(ptz-camera PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../mobile/iOS/Info.plist.in
)

target_include_directories(ptz-camera PUBLIC
    ../shared
    src
    ${GSTLIBS_INCLUDE_DIRS}
)

target_link_directories(ptz-camera PUBLIC
    ${GSTLIBS_LIBRARY_DIRS}
)

target_link_libraries(ptz-camera PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Multimedia
    Qt::Qml
    Qt::Quick
    Qt::SerialPort
    Qt::WebSockets
    ${GSTLIBS_LIBRARIES}
)

install(TARGETS ptz-camera
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
