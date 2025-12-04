cmake_minimum_required(VERSION 3.10)
set(project_name gaveta)
project(${project_name} VERSION 0.0.1)
include(CTest)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_C_FLAGS_DEBUG "-g3 -O0 -Wall -Wextra -Wpedantic -fsanitize=address")

file(GLOB SRC_FILES src/*.c)
file(GLOB MODULE_FILES src/modules/**/*.c)
file(GLOB TEST_FILES test/*.c)

include_directories(src)

# dynamic linking the modules
add_library(gaveta_lib STATIC ${MODULE_FILES})

add_executable(${project_name} ${SRC_FILES})
target_link_libraries(${project_name} PRIVATE gaveta_lib)

# testing
enable_testing()

add_test(NAME ${project_name}_test COMMAND ${project_name}_test)
add_executable(${project_name}_test ${TEST_FILES})
target_link_libraries(${project_name}_test PRIVATE gaveta_lib)


message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C flags debug: ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "Test files: ${TEST_FILES}")
