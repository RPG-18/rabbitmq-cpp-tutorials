set(PROJECT amqp-tutorial)
project(${PROJECT} C CXX)
cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wextra -pedantic")

include_directories(3rdparty/AMQP-CPP-2.1.4
                    3rdparty/
)

add_subdirectory(3rdparty/AMQP-CPP-2.1.4)
add_subdirectory(src)