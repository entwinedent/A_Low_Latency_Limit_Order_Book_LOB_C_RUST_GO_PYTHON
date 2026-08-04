# CLI Applications

add_executable(lob_cli
    main.cpp
)

target_link_libraries(lob_cli
    PRIVATE
        lob_core_impl
        spdlog
)

# Install CLI
install(TARGETS lob_cli
    RUNTIME DESTINATION bin
)
