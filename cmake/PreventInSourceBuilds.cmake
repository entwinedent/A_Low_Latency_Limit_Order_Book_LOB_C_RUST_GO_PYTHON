# Prevent in-source builds
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR 
        "In-source builds are not allowed. "
        "Please create a separate build directory and run CMake from there. "
        "Example:\n"
        "  mkdir build\n"
        "  cd build\n"
        "  cmake ..\n"
        "  make"
    )
endif()
