#ifdef HAVE_KOKKOS_MDSPAN
#include <mdspan/mdspan.hpp>
#include <iostream>

int main() {
    std::cout << "Testing Kokkos mdspan integration..." << std::endl;
    
    // Test basic mdspan functionality
    int data[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    
    // Create a 1D mdspan first to test basic functionality
    Kokkos::mdspan<int, Kokkos::extents<std::size_t, 12>> span1d(data);
    
    std::cout << "Kokkos mdspan 1D view created successfully" << std::endl;
    std::cout << "Element at index 0: " << span1d[0] << std::endl;
    std::cout << "Element at index 5: " << span1d[5] << std::endl;
    std::cout << "Element at index 11: " << span1d[11] << std::endl;
    
    std::cout << "Kokkos mdspan integration test PASSED" << std::endl;
    return 0;
}
#else
#include <iostream>
int main() {
    std::cout << "Kokkos mdspan not available - test skipped" << std::endl;
    return 0;
}
#endif
