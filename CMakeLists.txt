cmake_minimum_required(VERSION 3.16)
#project(my_game C CXX)

set(CMAKE_C_STANDARD 99)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_FOR_WEB "Build for the web with Emscripten" ON)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)  # don't build raylib's examples

if (BUILD_FOR_WEB)
    set(PLATFORM Web CACHE STRING "" FORCE)
endif()


FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(raylib)
configure_file(
        ${CMAKE_SOURCE_DIR}/patches/config.h
        ${raylib_SOURCE_DIR}/src/config.h
        COPYONLY
)

add_executable(${PROJECT_NAME}

        src/main.c
        include/game.h
        src/game.c
        include/game_error.h
        src/game_error.c
        include/get_mouse_pos_rel.h
        src/get_mouse_pos_rel.c
        include/hexagons.h
        include/move_hexagons.h
        src/move_hexagons.c
        src/draw_hex_tiles.c
        src/hexagons.c

)

target_link_libraries(${PROJECT_NAME} raylib)

if (BUILD_FOR_WEB)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ASSET_PATH=\"/assets\") # defines provided to the cpp source
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

    target_link_options(${PROJECT_NAME} PRIVATE
        "-sASYNCIFY"
        "-sTOTAL_MEMORY=134217728"     # 128 MB heap
        "-sSTACK_SIZE=1048576"         # 1 MB stack, same as in windows
        "-sFORCE_FILESYSTEM=1"
        "-sINVOKE_RUN=0"
        "-sEXPORTED_RUNTIME_METHODS=ccall,HEAPF32,callMain"
        "--shell-file=${CMAKE_SOURCE_DIR}/shell.html"
        "--preload-file=${CMAKE_SOURCE_DIR}/assets@/assets"
    )
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE ASSET_PATH=\"assets\")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    )
endif()