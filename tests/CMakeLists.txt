cmake_minimum_required(VERSION 3.0.0)
project(IEEE754_binary_encoder VERSION 0.1.0)

include(CTest)
enable_testing()

if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()
if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()

include_directories("../")
add_executable(IEEE754_binary_encoder main.cpp ../IEEE754_binary_encoder.c)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
