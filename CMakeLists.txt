cmake_minimum_required(VERSION 3.20)
project(MyCppProject LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB CORE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp")
file(GLOB IMG_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/img/*.cpp")
file(GLOB WAV_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/wav/*.cpp")

set(ALL_SOURCES ${SRC_FILES} ${CORE_FILES} ${IMG_FILES} ${WAV_FILES})
add_executable(main ${ALL_SOURCES})

target_include_directories(main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/stb"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/CLI11"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/libsndfile/include"
)

target_link_libraries(main PRIVATE stdc++exp)

target_link_libraries(main PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/libsndfile/lib/libsndfile.a")

target_link_options(main PRIVATE "-static")

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(main PRIVATE "-fdiagnostics-color=always")
endif()