cmake_minimum_required(VERSION 3.16)
project(lnos)

set(CMAKE_CXX_STANDARD 20)

add_executable(lnosd
        agent/main.cpp
        agent/ip.cpp
        agent/registry.cpp
)


target_include_directories(lnosd PRIVATE shared)

target_include_directories(lnosd PRIVATE agent)