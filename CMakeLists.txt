cmake_minimum_required(VERSION 3.20)

# Read version from VERSION file
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PROJECT_VERSION_RAW)
    string(STRIP "${PROJECT_VERSION_RAW}" PROJECT_VERSION)
else()
    set(PROJECT_VERSION "1.0.0")
    message(WARNING "VERSION file not found, using default version ${PROJECT_VERSION}")
endif()

project(lob_engine
    VERSION ${PROJECT_VERSION}
    DESCRIPTION "Low-Latency Limit Order Book Engine"
    LANGUAGES CXX
)

# Set C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_definitions(_SECURE_SCL=0 _SCL_SECURE_NO_WARNINGS)
endif()

# Include CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(StandardProjectSettings)
include(PreventInSourceBuilds)
include(Sanitizers)
include(CheckCXX23)

# Options
option(BUILD_TESTS "Build unit and integration tests" ON)
option(BUILD_BENCHMARKS "Build benchmarks" ON)
option(BUILD_APPS "Build CLI applications" ON)
option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_COVERAGE "Enable compiler instrumentation for coverage collection" OFF)

# Enable sanitizers if requested
if(ENABLE_SANITIZERS)
    set(ENABLE_ASAN ON)
    set(ENABLE_UBSAN ON)
endif()

# Apply sanitizers
if(ENABLE_ASAN)
    enable_sanitizer(ADDRESS)
endif()
if(ENABLE_UBSAN)
    enable_sanitizer(UNDEFINED)
endif()
if(ENABLE_TSAN)
    enable_sanitizer(THREAD)
endif()

if(ENABLE_COVERAGE)
    if(MSVC)
        add_compile_options(/Zi /Ob0 /Od)
        add_link_options(/DEBUG)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            # clang-cl coverage builds may hit third-party Clang warnings
            # emitted as errors when /WX is enabled.
            add_compile_options(-Wno-error=character-conversion)
            add_compile_options(-Wno-error=c2y-extensions)
            add_compile_options(-Wno-error=invalid-offsetof)
        endif()
    else()
        add_compile_options(--coverage)
        add_link_options(--coverage)
    endif()
endif()

# Dependencies: Try Conan first, fall back to FetchContent
if(EXISTS "${CMAKE_BINARY_DIR}/conan_toolchain.cmake")
    message(STATUS "Using Conan for dependency management")
    include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
else()
    message(STATUS "Conan not found, using FetchContent for dependencies")
    include(FetchContent)

    # GoogleTest
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    FetchContent_MakeAvailable(googletest)

    # Google Benchmark
    if(BUILD_BENCHMARKS)
        FetchContent_Declare(
            benchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.8.3
        )
        # Disable building benchmark library tests and examples
        set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(benchmark)
    endif()

    # spdlog (header-only)
    # Use pre-cloned spdlog from Docker environment if available
    if(EXISTS "/tmp/spdlog/include")
        message(STATUS "Using pre-cloned spdlog from Docker environment")
        add_library(spdlog INTERFACE)
        target_include_directories(spdlog INTERFACE /tmp/spdlog/include)
    else()
        FetchContent_Declare(
            spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG v1.12.0
        )
        # Don't make available, just include headers
        FetchContent_GetProperties(spdlog)
        if(NOT spdlog_POPULATED)
            FetchContent_Populate(spdlog)
        endif()
        add_library(spdlog INTERFACE)
        target_include_directories(spdlog INTERFACE ${spdlog_SOURCE_DIR}/include)
    endif()

    # Kokkos mdspan (header-only, for std::mdspan fallback)
    # Use pre-cloned Kokkos mdspan from Docker environment if available
    if(EXISTS "/tmp/mdspan/include")
        message(STATUS "Using pre-cloned Kokkos mdspan from Docker environment")
        add_library(kokkos_mdspan INTERFACE)
        target_include_directories(kokkos_mdspan INTERFACE /tmp/mdspan/include)
        set(HAVE_KOKKOS_MDSPAN 1 CACHE INTERNAL "Kokkos mdspan available")
    else()
        message(STATUS "Skipping Kokkos mdspan (not available on this platform)")
        set(HAVE_KOKKOS_MDSPAN 0 CACHE INTERNAL "Kokkos mdspan not available")
    endif()
endif()

# Add subdirectories
add_subdirectory(core)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
    add_subdirectory(bench)
endif()

if(BUILD_APPS)
    add_subdirectory(apps)
endif()

# Note: Language bindings (Go, Rust, Python) are built separately
# using their respective build systems (go build, cargo, pip/setuptools)

# Print configuration summary
message(STATUS "Configuration Summary:")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "  Build Benchmarks: ${BUILD_BENCHMARKS}")
message(STATUS "  Build Apps: ${BUILD_APPS}")
message(STATUS "  Address Sanitizer: ${ENABLE_ASAN}")
message(STATUS "  Undefined Behavior Sanitizer: ${ENABLE_UBSAN}")
message(STATUS "  Thread Sanitizer: ${ENABLE_TSAN}")
message(STATUS "  Coverage Instrumentation: ${ENABLE_COVERAGE}")
