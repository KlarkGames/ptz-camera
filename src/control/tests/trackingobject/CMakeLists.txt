enable_testing()

add_executable(trackingobject
    testtrackingobject.h
    testtrackingobject.cpp
    ../../src/trackingobject.h
    ../../src/trackingobject.cpp
)

add_test(NAME trackingobject COMMAND trackingobject)

target_include_directories(trackingobject PRIVATE
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(trackingobject PRIVATE
    ${OpenCV_LIBRARY_DIRS}
)

target_link_libraries(trackingobject PRIVATE
    Qt::Test
    ${OpenCV_LIBRARIES}
)
