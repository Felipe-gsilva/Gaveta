cmake_minimum_required(VERSION 3.10)
set(project_name gaveta)
project(${project_name} VERSION 0.0.1)

include(CTest)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if (WIN32)
    message(STATUS "Target system is Windows")
    set(CMAKE_C_FLAGS_DEBUG "-g3 -O0 -Wall -Wextra -Wpedantic")
elseif (UNIX)
    message(STATUS "Target system is Unix-like (including Linux)")
    set(CMAKE_C_FLAGS_DEBUG "-g3 -O0 -Wall -Wextra -Wpedantic -fsanitize=address")
else ()
    message(STATUS "Target system is neither Windows nor Unix-like")
endif ()

file(GLOB SRC_FILES src/*.c)
file(GLOB TEST_FILES test/*.c)
file(GLOB_RECURSE MODULE_FILES
    "${CMAKE_SOURCE_DIR}/src/modules/*.c"
    "${CMAKE_SOURCE_DIR}/src/modules/*/*.c"
    "${CMAKE_SOURCE_DIR}/src/modules/*/*/*.c"
)


include_directories(src)

# dynamic linking the modules
add_library(${project_name}_lib STATIC ${MODULE_FILES})

add_executable(${project_name} ${SRC_FILES})
target_link_libraries(${project_name} PRIVATE ${project_name}_lib)

# testing
enable_testing()

add_test(NAME ${project_name}_test COMMAND ${project_name}_test)
add_executable(${project_name}_test ${TEST_FILES})
target_link_libraries(${project_name}_test PRIVATE ${project_name}_lib)


message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C flags debug: ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "Test files: ${TEST_FILES}")
