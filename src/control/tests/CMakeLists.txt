enable_testing()

add_executable(hungarian
    testhungarian.h
    testhungarian.cpp
    ../src/hungarian.h
    ../src/hungarian.cpp
)

add_test(NAME hungarian COMMAND hungarian)

target_include_directories(hungarian PRIVATE
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(hungarian PRIVATE
    ${OpenCV_LIBRARY_DIRS}
)

target_link_libraries(hungarian PRIVATE
    Qt::Test
    ${OpenCV_LIBRARIES}
)
