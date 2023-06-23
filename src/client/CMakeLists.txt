cmake_minimum_required(VERSION 3.16)
project(ptz-camera-client LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia Qml Quick SerialPort WebSockets)

qt_add_executable(ptz-camera-client
    src/main.cpp
    src/client.h
    src/client.cpp
)

set_target_properties(ptz-camera-client PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
)

target_include_directories(ptz-camera-client PUBLIC
    ../shared
    src
)

target_link_libraries(ptz-camera-client PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Multimedia
    Qt::Qml
    Qt::Quick
    Qt::SerialPort
    Qt::WebSockets
)

# Resources:
set(ptz-camera-client_resource_files
    "qml/main.qml"
    "qml/Style.qml"
    "qml/qmldir"
)

qt_add_resources(ptz-camera-client "ptz-camera-client"
    PREFIX
        "/"
    FILES
        ${ptz-camera-client_resource_files}
)

qt6_add_qml_module(ptz-camera-client
    URI com.basilevs.multimedia
    VERSION 1.0
    SOURCES
        src/client.h
)

install(TARGETS ptz-camera-client
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
