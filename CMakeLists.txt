cmake_minimum_required(VERSION 3.15)

project(slc-burner-lib VERSION 1.0 LANGUAGES CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library
add_library(${PROJECT_NAME} src/Burner.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Test Executable
set(TEST_TARGET burner_test)
add_executable(${TEST_TARGET} src/main.cpp)
target_link_libraries(${TEST_TARGET} PRIVATE ${PROJECT_NAME})

if(MSVC)
    target_compile_options(${TEST_TARGET} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${TEST_TARGET} PRIVATE -Wall -Wextra -Wpedantic)
endif()