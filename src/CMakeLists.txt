
include_directories(.)
set(sources
        main.cpp
        document.cpp
        paint_manager.cpp
        command_queue.cpp
        layout.cpp
        text_buffer.cpp
        geditor.cpp
        memory.cpp
        table.cpp
        utils.cpp
        )

add_executable(geditor ${sources})
