cmake_minimum_required(VERSION 3.13)
project(vttytester)

# set C++17
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")
set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp serialport.cpp)

target_link_libraries(${PROJECT_NAME} boost_system boost_thread)
