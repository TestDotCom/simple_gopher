project(windows)

add_executable(server_win 
    ${gophersrv_SOURCE_DIR}/server.c
    ${PROJECT_SOURCE_DIR}/src/wingetopt.c
)

target_include_directories(server_win
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${libs_SOURCE_DIR}/include
)

target_link_libraries(server_win
    PRIVATE
        common::core
)

add_executable(subserver src/subserver.c)

target_include_directories(subserver
    PRIVATE
        ${libs_SOURCE_DIR}/include
)

target_link_libraries(subserver
    PRIVATE
        common::core
)

add_executable(client_logger src/client_logger.c)

target_include_directories(client_logger
    PRIVATE
        ${libs_SOURCE_DIR}/include
)

target_link_libraries(client_logger
    PRIVATE
        common::core
)
