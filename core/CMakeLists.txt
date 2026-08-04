# Core C++ Order Book Engine Library

# Header-only library for template components
add_library(lob_core INTERFACE)
target_include_directories(lob_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Implementation library
add_library(lob_core_impl STATIC
    src/OrderBook.cpp
)

target_link_libraries(lob_core_impl
    PUBLIC
        lob_core
        spdlog
)

# Create shared library for FFI bindings
add_library(lob_core_shared SHARED
    src/OrderBook.cpp
    src/CAPI.cpp
)

target_include_directories(lob_core_shared
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(lob_core_shared
    PUBLIC
        spdlog
)

# Set library version
set_target_properties(lob_core_shared PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME lob_core
)

# Platform-specific shared library settings
if(WIN32)
    set_target_properties(lob_core_shared PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

# Install rules
install(TARGETS lob_core_impl lob_core_shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/lob
    DESTINATION include
)

# Alias for convenience
add_library(lob::core ALIAS lob_core_impl)
