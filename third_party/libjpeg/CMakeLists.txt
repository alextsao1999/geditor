# Created by Alex
set(CMAKE_CXX_STANDARD 14)
set(libjpeg_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/ CACHE INTERNAL "")
aux_source_directory(${libjpeg_include_dirs} sources)
add_library(jpeg STATIC ${sources})
target_include_directories(jpeg PUBLIC ${libjpeg_include_dirs})
