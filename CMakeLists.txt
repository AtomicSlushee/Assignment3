# Authors: Ken Steele, Andrew Byron and Eric Lichtenstein
# Assignment 3

# prevent in-source builds
cmake_minimum_required( VERSION 3.1 )
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source builds are not allowed. Create and use a 'build' folder.")
endif()

# Use C++11
set (CMAKE_CXX_STANDARD 11)

# start of project
project( hlsyn )
#uncomment next line to place executable in root, not src
#SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
add_subdirectory( src )
