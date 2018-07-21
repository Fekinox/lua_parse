cmake_minimum_required(VERSION 2.8)
project(luaparse)

find_library(LUA lua53.dll PATHS "../lib")

set(SOURCES main.cpp)
set(LIBS ${LUA})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

include_directories("C:/MinGW/include" "../include")
add_executable(luaparse ${SOURCES})

target_link_libraries(luaparse ${LIBS})