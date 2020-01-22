project(linux)

add_executable(server_nix 
    ${gophersrv_SOURCE_DIR}/server.c
    ${PROJECT_SOURCE_DIR}/src/daemonize.c
)

target_include_directories(server_nix
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${libs_SOURCE_DIR}/include
)

target_link_libraries(server_nix
    PRIVATE
        common::core
)
