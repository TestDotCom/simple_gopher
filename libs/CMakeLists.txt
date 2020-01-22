project(core)

if(WIN32)
    set(SRC src_win)
else()
    set(SRC src_nix)
endif()

add_library(${PROJECT_NAME}
    STATIC
        src/stdconf.c
        src/stdsocket.c
        src/stdgopher.c
        src/stdlog.c
        src/socket_wrap.c
        ${SRC}/io_wrap.c
        ${SRC}/stdsignal.c
        ${SRC}/log_client.c
        ${SRC}/stdthread.c
        ${SRC}/multiplx.c
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            m
            shlwapi
            ws2_32
    )
else()
    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            m
    )
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/include
)

add_library(common::core ALIAS ${PROJECT_NAME})
