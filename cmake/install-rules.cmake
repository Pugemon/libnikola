if(PROJECT_IS_TOP_LEVEL)
    set(
            CMAKE_INSTALL_INCLUDEDIR "include/libnikola-${PROJECT_VERSION}"
            CACHE STRING ""
    )
    set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package libnikola)

install(
        DIRECTORY
        include/
        "${PROJECT_BINARY_DIR}/export/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        COMPONENT libnikola_Development
)

install(
        TARGETS libnikola_libnikola
        EXPORT libnikolaTargets
        RUNTIME #
        COMPONENT libnikola_Runtime
        LIBRARY #
        COMPONENT libnikola_Runtime
        NAMELINK_COMPONENT libnikola_Development
        ARCHIVE #
        COMPONENT libnikola_Development
        INCLUDES #
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
        "${package}ConfigVersion.cmake"
        COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
        libnikola_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
        CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE libnikola_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(libnikola_INSTALL_CMAKEDIR)

install(
        FILES cmake/install-config.cmake
        DESTINATION "${libnikola_INSTALL_CMAKEDIR}"
        RENAME "${package}Config.cmake"
        COMPONENT libnikola_Development
)

install(
        FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
        DESTINATION "${libnikola_INSTALL_CMAKEDIR}"
        COMPONENT libnikola_Development
)

install(
        EXPORT libnikolaTargets
        NAMESPACE libnikola::
        DESTINATION "${libnikola_INSTALL_CMAKEDIR}"
        COMPONENT libnikola_Development
)

if(PROJECT_IS_TOP_LEVEL)
    include(CPack)
endif()
