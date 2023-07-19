cmake_minimum_required(VERSION 3.16)
project(ptz-camera-client LANGUAGES CXX)

set(FFMPEG_KIT_PATH /home/user/ffmpeg-kit)

set(CMAKE_AUTOMOC ON)
set(ENV{QT_MEDIA_BACKEND} "ffmpeg")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia Qml Quick WebSockets)

qt_add_executable(ptz-camera-client
    src/main.cpp
    src/client.h
    src/client.cpp
    src/trackingobjectmodel.h
    src/trackingobjectmodel.cpp
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
    Qt::WebSockets
)

# Resources:
set(ptz-camera-client_resource_files
    "qml/main.qml"
    "qml/ControlScreen.qml"
    "qml/ConnectionScreen.qml"
    "qml/Style.qml"
    "qml/RecordButton.qml"
    "qml/qmldir"
    "res/logo.png"
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

set(ANDROID_FFMPEG_EXTRAS_DIR ${FFMPEG_KIT_PATH}/prebuilt/android-${ANDROID_ABI}/ffmpeg/lib)

set_property(TARGET ptz-camera-client PROPERTY QT_ANDROID_EXTRA_LIBS
    ${ANDROID_FFMPEG_EXTRAS_DIR}/libavformat.so
    ${ANDROID_FFMPEG_EXTRAS_DIR}/libavcodec.so
    ${ANDROID_FFMPEG_EXTRAS_DIR}/libswresample.so
    ${ANDROID_FFMPEG_EXTRAS_DIR}/libswscale.so
    ${ANDROID_FFMPEG_EXTRAS_DIR}/libavutil.so
)
