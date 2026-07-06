cmake_minimum_required(VERSION 3.16)
project(lnos)

set(CMAKE_CXX_STANDARD 20)

add_executable(lnos-server
        server/main.cpp
        server/main.h
)

add_executable(lnos-agent
        agent/main.cpp
        agent/main.h
)