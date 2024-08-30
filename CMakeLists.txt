cmake_minimum_required(VERSION 3.13)

if(DEFINED ENV{DEVKITPRO})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{DEVKITPRO}/cmake/Switch.cmake" CACHE STRING "DevkitPro Switch toolchain")
else()
    message(FATAL_ERROR "Please set DEVKITPRO env before calling cmake. https://devkitpro.org/wiki/Getting_Started")
endif()

include(cmake/prelude.cmake)

project(
        libnikola
        LANGUAGES CXX
        VERSION 1.4.0
        DESCRIPTION "The support library for Tesla overlays"
        HOMEPAGE_URL "https://github.com/Pugemon/libnikola"
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
        libnikola_libnikola
        source/nikola.cpp
        source/utils/get_funcs.cpp
        source/utils/ini_funcs.cpp
        source/utils/string_funcs.cpp
        source/tesla/hlp.cpp
        source/tesla/elm.cpp
        source/tesla/gfx.cpp
        source/tesla/impl.cpp
        source/tesla.cpp
)
add_library(libnikola::libnikola ALIAS libnikola_libnikola)


set_target_properties(
        libnikola_libnikola PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
        VERSION "${PROJECT_VERSION}"
        SOVERSION "${PROJECT_VERSION_MAJOR}"
        EXPORT_NAME libnikola
        OUTPUT_NAME libnikola
)

target_include_directories(
        libnikola_libnikola ${warning_guard}
        PUBLIC
        "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)


target_compile_features(libnikola_libnikola PUBLIC cxx_std_20)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT libnikola_DEVELOPER_MODE)
    return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
    message(
            AUTHOR_WARNING
            "Developer mode is intended for developers of libnikola"
    )
endif()

include(cmake/dev-mode.cmake)