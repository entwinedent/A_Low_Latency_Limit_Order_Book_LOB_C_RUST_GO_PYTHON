# C++23 Feature Detection Module
# This module checks for C++23 feature support in the compiler

include(CheckCXXSourceCompiles)

# Check for std::expected
check_cxx_source_compiles("
#include <expected>
int main() {
    std::expected<int, int> e = 42;
    return 0;
}
" HAVE_STD_EXPECTED)

# Check for std::print
check_cxx_source_compiles("
#include <print>
int main() {
    std::print(\"Hello\");
    return 0;
}
" HAVE_STD_PRINT)

# Check for deducing this
check_cxx_source_compiles("
struct Widget {
    auto func(this auto& self) { return self; }
};
int main() {
    Widget w;
    w.func();
    return 0;
}
" HAVE_DEDUCING_THIS)

# Check for std::generator
check_cxx_source_compiles("
#include <generator>
std::generator<int> range(int start, int end) {
    for (int i = start; i < end; ++i) {
        co_yield i;
    }
}
int main() {
    return 0;
}
" HAVE_STD_GENERATOR)

# Check for std::mdspan
check_cxx_source_compiles("
#include <mdspan>
int main() {
    int data[4] = {1, 2, 3, 4};
    std::mdspan<int, std::extents<2>> span(data);
    return 0;
}
" HAVE_STD_MDSPAN)

# Note: Kokkos mdspan availability is checked in main CMakeLists.txt
# based on whether the library is actually present in the environment

# Check for C++23 string view improvements
check_cxx_source_compiles("
#include <string_view>
int main() {
    std::string_view sv = \"hello\";
    auto contains = sv.contains('h');
    return 0;
}
" HAVE_STRING_VIEW_CONTAINS)

# Print feature detection results
message(STATUS "C++23 Feature Detection Results:")
message(STATUS "  std::expected: ${HAVE_STD_EXPECTED}")
message(STATUS "  std::print: ${HAVE_STD_PRINT}")
message(STATUS "  Deducing this: ${HAVE_DEDUCING_THIS}")
message(STATUS "  std::generator: ${HAVE_STD_GENERATOR}")
message(STATUS "  std::mdspan: ${HAVE_STD_MDSPAN}")
message(STATUS "  Kokkos mdspan: ${HAVE_KOKKOS_MDSPAN}")
message(STATUS "  string_view::contains: ${HAVE_STRING_VIEW_CONTAINS}")

# Set compile definitions based on feature availability
if(HAVE_STD_EXPECTED)
    add_compile_definitions(HAVE_STD_EXPECTED=1)
endif()
if(HAVE_STD_PRINT)
    add_compile_definitions(HAVE_STD_PRINT=1)
endif()
if(HAVE_DEDUCING_THIS)
    add_compile_definitions(HAVE_DEDUCING_THIS=1)
endif()
if(HAVE_STD_GENERATOR)
    add_compile_definitions(HAVE_STD_GENERATOR=1)
endif()
if(HAVE_STD_MDSPAN)
    add_compile_definitions(HAVE_STD_MDSPAN=1)
endif()
if(HAVE_KOKKOS_MDSPAN)
    add_compile_definitions(HAVE_KOKKOS_MDSPAN=1)
endif()
if(HAVE_STRING_VIEW_CONTAINS)
    add_compile_definitions(HAVE_STRING_VIEW_CONTAINS=1)
endif()