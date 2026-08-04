# Standard project settings for compiler flags and warnings

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler-specific flags
if(MSVC)
    # MSVC flags
    add_compile_options(/W4 /permissive-)
    # Disable specific warnings for performance-critical code
    add_compile_options(/wd4244)  # conversion from 'type1' to 'type2', possible loss of data
    add_compile_options(/wd4267)  # conversion from 'size_t' to 'type', possible loss of data
    add_compile_options(/wd4996)  # deprecated functions
else()
    # GCC/Clang flags
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_compile_options(-Wno-unused-parameter)
    add_compile_options(-Wno-sign-compare)
    
    # Release optimizations
    add_compile_options($<$<CONFIG:Release>:-O3>)
    if(NOT APPLE)
        add_compile_options($<$<CONFIG:Release>:-march=native>)
    endif()
    add_compile_options($<$<CONFIG:Release>:-ffast-math>)
    
    # Debug flags
    add_compile_options($<$<CONFIG:Debug>:-g>)
    add_compile_options($<$<CONFIG:Debug>:-O0>)
endif()

# Position-independent code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Visibility attributes for shared libraries
if(NOT WIN32)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
endif()
