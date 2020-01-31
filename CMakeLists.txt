cmake_minimum_required(VERSION 3.1.1)
project(Geditor)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})

add_definitions(-DUNICODE -D_UNICODE)

add_subdirectory(third_party/skia)
add_subdirectory(third_party/ecode)
include_directories(${skia_include_dirs})
include_directories(${ecode_include_dirs})

include_directories(src/)
file(GLOB_RECURSE headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.h)
aux_source_directory(src/ sources)
source_group("Header Files" FILES ${headers})
source_group("Source Files" FILES ${sources})

add_library(geditor STATIC ${sources} ${headers})

set(LINK_STATIC_LIB opengl32.lib usp10.lib skia geditor ecode)

add_executable(main example/main.cpp)
target_link_libraries(main ${LINK_STATIC_LIB})

add_library(editor
        SHARED
        example/dll.h
        example/dll.cpp)
target_link_libraries(editor ${LINK_STATIC_LIB})
