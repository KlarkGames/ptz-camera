cmake_minimum_required(VERSION 3.16)
project(ptz-camera LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia Qml Quick SerialPort)

qt_add_executable(ptz-camera
    src/mail.cpp
    src/processor.h
    src/processor.cpp
    src/mountdriver.h
    src/mountdriver.cpp
)

set_target_properties(ptz-camera PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
)

target_include_directories(ptz-camera PUBLIC
    ../shared
    src
)

target_link_libraries(ptz-camera PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Multimedia
    Qt::Qml
    Qt::Quick
    Qt::SerialPort
)

# Resources:
set(ptz-camera_resource_files
    "qml/main.qml"
    "qml/qmldir"
    "qml/MediaList.qml"
    "qml/AudioInputSelect.qml"
    "qml/CameraSelect.qml"
    "qml/MountSelect.qml"
    "qml/RecordButton.qml"
    "qml/Controls.qml"
    "qml/StyleParameter.qml"
    "qml/StyleRectangle.qml"
    "qml/SettingsMetaData.qml"
    "qml/SettingsEncoder.qml"
    "qml/StyleSlider.qml"
    "qml/Style.qml"
    "qml/Playback.qml"
)

qt_add_resources(ptz-camera "ptz-camera"
    PREFIX
        "/"
    FILES
        ${ptz-camera_resource_files}
)

qt6_add_qml_module(ptz-camera
    URI com.basilevs.multimedia
    VERSION 1.0
    SOURCES
        src/processor.h
)

install(TARGETS ptz-camera
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
