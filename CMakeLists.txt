cmake_minimum_required(VERSION 3.3)
project(huffman)

set(CMAKE_C_FLAGS "-std=c99")

set(SOURCE_FILES main.c tree_3cell.c tree_3cell.h bitset.c bitset.h)
add_executable(huffman ${SOURCE_FILES} main.c)