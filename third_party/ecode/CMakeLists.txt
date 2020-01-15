# Created By ≤‹À≥
set(CMAKE_CXX_STANDARD 14)

set(DIRS .)

include_directories(${DIRS})

foreach (DIR ${DIRS})
    aux_source_directory(${DIR} FILES)
endforeach ()

add_library(parser STATIC ${FILES})
