cmake_minimum_required(VERSION 3.16)
project(raw_eth)

set(CMAKE_CXX_STANDARD 14)

add_executable(raw_eth main.cpp InetAddress.cpp InetAddress.h InetAddress.cpp InetAddress.h arpspoof.h.cpp)