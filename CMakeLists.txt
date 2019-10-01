cmake_minimum_required(VERSION 3.1.1)
project(Geditor)

set(CMAKE_CXX_STANDARD 14)

set(inc_dirs
        animator;config;core;device/xps;effects;gpu;gpu/gl;images;doc;pathops;pdf;pipe;ports;
        record;svg;utils;utils/win;utils/ios;utils/mac;views;views/android;views/animated;views/unix;xml)
foreach(inc_dir ${inc_dirs})
    include_directories("third_party/skia/include/${inc_dir}")
endforeach()
add_subdirectory(third_party/skia)

add_definitions(-DUNICODE -D_UNICODE)
include_directories(src/)
file(GLOB_RECURSE headers RELATIVE ${CMAKE_SOURCE_DIR} src/*.h)
aux_source_directory(src/ sources)
source_group("Header Files" FILES ${headers})
source_group("Source Files" FILES ${sources})
add_executable(geditor ${sources})

set(LINK_STATIC_LIB opengl32.lib usp10.lib ${CMAKE_BINARY_DIR}/lib/skia.lib)
target_link_libraries(geditor ${LINK_STATIC_LIB})
