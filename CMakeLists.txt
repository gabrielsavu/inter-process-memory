cmake_minimum_required(VERSION 3.15)
project(memoryalloc C)

set(CMAKE_C_STANDARD 99)

add_executable(memoryalloc main.c src/memory.c src/memory.h src/internals.h)
TARGET_LINK_LIBRARIES(memoryalloc -lrt -pthread)