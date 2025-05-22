 
# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(Ecosystem_Simulator)

# Options
option(BUILD_TESTS "Build tests" OFF)

# Trouver les packages requis
find_package(raylib 3.0 REQUIRED)

# Sources principales
file(GLOB_RECURSE SOURCES src/*.cpp)

# Executable principal
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src include)
target_link_libraries(${PROJECT_NAME} raylib)

# Tests
if(BUILD_TESTS)
    file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
    add_executable(run_tests ${TEST_SOURCES})
    target_include_directories(run_tests PRIVATE src include)
    target_link_libraries(run_tests raylib)
endif()