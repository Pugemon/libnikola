cmake_minimum_required(VERSION 3.13)

if(DEFINED ENV{DEVKITPRO})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{DEVKITPRO}/cmake/Switch.cmake" CACHE STRING "DevkitPro Switch toolchain")
else()
    message(FATAL_ERROR "Please set DEVKITPRO env before calling cmake. https://devkitpro.org/wiki/Getting_Started")
endif()

project(
        LibNikola-Example
        LANGUAGES CXX
        VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME}
        source/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        source/
        include/
)

target_link_libraries(${PROJECT_NAME} PRIVATE libnikola::libnikola)

nx_generate_nacp(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.nacp"
        AUTHOR "Pugemon"
        VERSION "${PROJECT_VERSION}"
        NAME "${PROJECT_NAME}"
)

nx_create_nro(${PROJECT_NAME}
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.ovl"
        NACP "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.nacp"
        NOICON
)