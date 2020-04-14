# Created By Alex Tsao
cmake_minimum_required(VERSION 3.14)
project(third_party)

add_subdirectory(lsp)
add_subdirectory(libjpeg)
add_subdirectory(skia)
add_subdirectory(ecode)

add_library(lalr STATIC)
target_include_directories(lalr PUBLIC lalr/lalr/)
aux_source_directory(lalr/lalr/lalr/ sources)
target_sources(lalr PUBLIC ${sources})
