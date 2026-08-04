fn main() {
    // Use cxx-build for safe C++ interop
    let mut build = cxx_build::bridge("src/ffi.rs");
    
    build.file("../../core/src/OrderBook.cpp")
         .file("../../core/src/CAPI.cpp")
         .file("cxx_wrapper.cpp")
         .include("../../core/include")
         .include("../../core/spdlog/include")
         .include(".");

    // Set C++23 standard
    build.flag_if_supported("-std:c++23");

    // MSVC-specific flags for Windows
    if cfg!(target_env = "msvc") {
        build.flag("/std:c++23");
        build.flag("/EHsc");  // Enable exception handling
        build.flag("/utf-8");  // Enable UTF-8 source encoding
    }

    build.compile("lob_core");
    
    println!("cargo:rerun-if-changed=../../core/src/OrderBook.cpp");
    println!("cargo:rerun-if-changed=../../core/src/CAPI.cpp");
    println!("cargo:rerun-if-changed=cxx_wrapper.h");
    println!("cargo:rerun-if-changed=cxx_wrapper.cpp");
    println!("cargo:rerun-if-changed=../../core/include/lob/OrderBook.h");
    println!("cargo:rerun-if-changed=../../core/include/lob/CAPI.h");
    println!("cargo:rerun-if-changed=../../core/include/lob/IntrusiveList.h");
    println!("cargo:rerun-if-changed=../../core/include/lob/OrderTypes.h");
    println!("cargo:rerun-if-changed=src/ffi.rs");
}
