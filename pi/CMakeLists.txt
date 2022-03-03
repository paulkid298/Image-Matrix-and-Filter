cmake_minimum_required(VERSION 2.8)
project( Server )
find_package( OpenCV REQUIRED )
find_package (Threads)
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( main main.cpp )


ADD_LIBRARY(LibsModule 
    client.cpp
    utils_singlethread.cpp
    utils.cpp
    image_filter_sequential.cpp
    image_filter_parallel.cpp
    global.cpp
)

target_link_libraries (main ${CMAKE_THREAD_LIBS_INIT} ${OpenCV_LIBS} LibsModule)

#set(CMAKE_BUILD_TYPE Debug)

