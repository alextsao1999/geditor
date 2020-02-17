# Created by Alex
set(CMAKE_CXX_STANDARD 14)
set(libpng_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/ CACHE INTERNAL "")
aux_source_directory(${libpng_include_dirs} sources)
include_directories(${libpng_include_dirs})
add_library(png STATIC ${sources})
target_include_directories(png PUBLIC ${libpng_include_dirs})
