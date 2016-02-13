cmake_minimum_required(VERSION 3.3)
project(huffman)

set(CMAKE_C_FLAGS "-std=c99")

set(SOURCE_FILES huffman.c list_2cell.c tree_3cell.c prioqueue.c bitset.c)
add_executable(huffman ${SOURCE_FILES} huffman.c)
