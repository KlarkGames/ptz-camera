cmake_minimum_required(VERSION 3.16)
project(ptz-camera LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Test Gui Multimedia Qml Quick SerialPort WebSockets)
find_package(OpenCV 4.5.1 REQUIRED)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../models/MobileNetV2_modified.onnx
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/models)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../models/yolov8s.onnx
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/models)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../models/yolov5n.onnx
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/models)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../models/coco.names
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/utils)

qt_add_executable(ptz-camera
    src/main.cpp
    src/processor.h
    src/processor.cpp
    src/mountdriver.h
    src/mountdriver.cpp
    src/server.h
    src/server.cpp
    src/settingsconductor.h
    src/streamer.h
    src/streamer.cpp
    src/trackingobject.h
    src/trackingobject.cpp
    src/deepsort.h
    src/deepsort.cpp
    src/hungarian.h
    src/hungarian.cpp
    src/nets/objectdetectionnet.h
    src/nets/yolo5.cpp
    src/nets/yolo5.h
    src/utils.h
    src/arduinocommand.h
    src/arduinocommand.cpp
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTLIBS REQUIRED
    gstreamer-1.0>=1.22
    gstreamer-video-1.0>=1.22
    gstreamer-app-1.0>=1.22
    gstreamer-base-1.0>=1.22
)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set_target_properties(ptz-camera PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_include_directories(ptz-camera PUBLIC
    ../shared
    src
    ${GSTLIBS_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

target_link_directories(ptz-camera PUBLIC
    ${GSTLIBS_LIBRARY_DIRS}
    ${OpenCV_LIBRARY_DIRS}
)

target_link_libraries(ptz-camera PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Multimedia
    Qt::Qml
    Qt::Quick
    Qt::SerialPort
    Qt::Test
    Qt::WebSockets
    ${GSTLIBS_LIBRARIES}
    Qt6::Widgets
    ${OpenCV_LIBRARIES}
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

add_subdirectory(tests)

install(TARGETS ptz-camera
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
