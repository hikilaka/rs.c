cmake_minimum_required(VERSION 3.1...3.11)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_VERSION})
endif()

project(rs.c VERSION 0.1
             DESCRIPTION ""
             LANGUAGES C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR/cmake/Modules/}"


find_package(SDL2 REQUIRED)

find_package(BZip2 REQUIRED)


option(USE_SANIZITERS "Use Address, undefined behavior, and memory sanitizers" OFF)


add_compile_options(-Wall -Wextra -Wpedantic -g)

if (USE_SANITIZERS)
    set(SANITIZERS -fno-omit-frame-pointer -fsanitize=address,undefined)
    add_compile_options(-fno-omit-frame-pointer -fsanitize=address,undefined)
endif(USE_SANITIZERS)


add_library(log.c
    STATIC "${CMAKE_CURRENT_SOURCE_DIR}/log.c/src/log.c")

target_include_directories(log.c
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/log.c/src")

target_compile_features(log.c
    PUBLIC c_std_99)

target_compile_definitions(log.c
    PRIVATE LOG_USE_COLOR)


add_executable(client
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/surface.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jag_arc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/buffer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entity.c")

target_include_directories(client
    PUBLIC
        "${SDL2_INCLUDE_DIRS}"
        "${BZIP2_INCLUDE_DIRS}"
        "${CMAKE_CURRENT_SOURCE_DIR}/log.c/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(client
    PUBLIC
        m
        log.c
        "${SDL2_LIBRARIES}"
        "${BZIP2_LIBRARIES}"
        "${SANITIZERS}")

target_compile_features(client
    PUBLIC c_std_99)

