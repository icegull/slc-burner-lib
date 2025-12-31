cmake_minimum_required(VERSION 3.15)

project(slc-burner-lib VERSION 1.0 LANGUAGES CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX d)

# Common Compiler Warnings
if(MSVC)
    set(COMPILER_WARNINGS /W4 /permissive-)
else()
    set(COMPILER_WARNINGS -Wall -Wextra -Wpedantic)
endif()

# Library
add_library(${PROJECT_NAME} src/Burner.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_WARNINGS})

# Test Executable
set(TEST_TARGET burner_test)
add_executable(${TEST_TARGET} src/main.cpp)
target_link_libraries(${TEST_TARGET} PRIVATE ${PROJECT_NAME})
target_compile_options(${TEST_TARGET} PRIVATE ${COMPILER_WARNINGS})

# Install
include(GNUInstallDirs)

# Determine Platform and Arch for Install Layout
if(WIN32)
    set(PLATFORM_NAME "Windows")
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_NAME "Linux")
elseif(APPLE)
    set(PLATFORM_NAME "Darwin")
else()
    set(PLATFORM_NAME ${CMAKE_SYSTEM_NAME})
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|x86_64|AMD64")
    set(ARCH_NAME "x64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|ARM")
    set(ARCH_NAME "arm")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
    set(ARCH_NAME "arm64")
else()
    set(ARCH_NAME ${CMAKE_SYSTEM_PROCESSOR})
endif()

set(INSTALL_LIB_DIR "lib/${PLATFORM_NAME}/${ARCH_NAME}/$<CONFIG>")
set(INSTALL_BIN_DIR "bin/${PLATFORM_NAME}/${ARCH_NAME}/$<CONFIG>")

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export Targets
install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Config and Version Files
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# CPack
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "SLC")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SLC Burner Library")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "ZIP")

include(CPack)
