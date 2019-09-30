cmake_minimum_required(VERSION 3.1.1)
project(Geditor)

set(CMAKE_CXX_STANDARD 14)

set(inc_dirs
        animator;config;core;device/xps;effects;gpu;gpu/gl;images;doc;pathops;pdf;pipe;ports;
        record;svg;utils;utils/win;utils/ios;utils/mac;views;views/android;views/animated;views/unix;xml)
foreach(inc_dir ${inc_dirs})
    include_directories("third_party/skia/include/${inc_dir}")
endforeach()
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

include_directories(src/)

aux_source_directory(src/ sources)
add_executable(geditor ${sources})

add_subdirectory(third_party/skia)

find_library(LIB_GL Opengl32.lib OpenGl opengl32 opengl)
find_library(LIB_USP10 Usp10.lib usp10.lib usp10)
target_link_libraries(geditor ${LIB_USP10} ${LIB_GL} ${CMAKE_BINARY_DIR}/lib/skiad.lib)
