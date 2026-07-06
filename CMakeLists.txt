cmake_minimum_required(VERSION 3.16)
project(lnos)

set(CMAKE_CXX_STANDARD 20)

add_executable(lnos-agent
        agent/main.cpp
        agent/ip.cpp
        agent/registry.cpp
)


target_include_directories(lnos-agent PRIVATE shared)

target_include_directories(lnos-agent PRIVATE agent)