# Sanitizer support for AddressSanitizer, UndefinedBehaviorSanitizer, and ThreadSanitizer

function(enable_sanitizer SANITIZER)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(WARNING "Sanitizers are only supported with GCC and Clang")
        return()
    endif()
    
    if(SANITIZER STREQUAL "ADDRESS")
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    elseif(SANITIZER STREQUAL "UNDEFINED")
        add_compile_options(-fsanitize=undefined)
        add_link_options(-fsanitize=undefined)
    elseif(SANITIZER STREQUAL "THREAD")
        add_compile_options(-fsanitize=thread)
        add_link_options(-fsanitize=thread)
    else()
        message(WARNING "Unknown sanitizer: ${SANITIZER}")
    endif()
    
    # Common sanitizer flags
    add_compile_options(-fno-omit-frame-pointer)
    add_compile_options(-fno-optimize-sibling-calls)
endfunction()
