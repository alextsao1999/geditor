cmake_minimum_required(VERSION 3.1.1)
project(GEditor)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})

if (MSVC)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS)
    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE)
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
        string(APPEND ${CompilerFlag} " /Zc:threadSafeInit-")
    endforeach()
endif()

add_definitions(-DUNICODE -D_UNICODE)

set(BUILD_SHARED ON)
add_subdirectory(third_party)

include_directories(src/)
file(GLOB_RECURSE headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.h)
aux_source_directory(src/ sources)
source_group("Header Files" FILES ${headers})
source_group("Source Files" FILES ${sources})
add_library(geditor STATIC ${sources} ${headers})
target_link_libraries(geditor PUBLIC skia ecode lsp lalr)

add_executable(main example/main.cpp)
target_link_libraries(main geditor)

add_library(editor
        SHARED
        example/dll.h
        example/dll.cpp)
target_link_libraries(editor geditor)

add_subdirectory(platform/qt)

add_executable(test
        example/test.cpp)
target_link_libraries(test lalr)
