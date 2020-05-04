cmake_minimum_required(VERSION 3.14)
project(ParserBuilder)
set(CMAKE_CXX_STANDARD 14)

set(include_dirs .)
foreach(include_dir ${include_dirs})
    include_directories(${include_dir})
    aux_source_directory(${include_dir} sources)
endforeach()

add_executable(builder ${sources})
target_link_libraries(builder lsp)