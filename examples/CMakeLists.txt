# Streamlined examples CMakeLists.txt for macOS whisper-realtime
# Only builds essential common libraries needed for audio capture

# dependencies
find_package(Threads REQUIRED)

# SDL2 for audio capture
if (WHISPER_SDL2)
    find_package(SDL2 REQUIRED)
    string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)
    message(STATUS "SDL2_INCLUDE_DIRS = ${SDL2_INCLUDE_DIRS}")
    message(STATUS "SDL2_LIBRARIES    = ${SDL2_LIBRARIES}")
endif()

# common utilities library
set(TARGET common)

add_library(${TARGET} STATIC
    common.h
    common.cpp
    common-ggml.h
    common-ggml.cpp
    # common-whisper.cpp removed - has file loading dependencies we don't need
    grammar-parser.h
    grammar-parser.cpp
    )

include(DefaultTargetOptions)

target_link_libraries(${TARGET} PRIVATE whisper ${CMAKE_DL_LIBS})

set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(${TARGET} PROPERTIES FOLDER "libs")

# SDL2 audio capture library  
if (WHISPER_SDL2)
    set(TARGET common-sdl)

    add_library(${TARGET} STATIC
        common-sdl.h
        common-sdl.cpp
        )

    include(DefaultTargetOptions)

    target_include_directories(${TARGET} PUBLIC  ${SDL2_INCLUDE_DIRS})
    target_link_libraries     (${TARGET} PRIVATE ${SDL2_LIBRARIES})

    set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(${TARGET} PROPERTIES FOLDER "libs")
endif()

# JSON interface (used by common libraries)
add_library(json_cpp INTERFACE)
target_include_directories(json_cpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Include directory for examples
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# No example programs - only essential libraries for whisper-realtime