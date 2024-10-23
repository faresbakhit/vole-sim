cmake_minimum_required(VERSION 3.5)

project(vole-sim
        VERSION 0.1.0
        DESCRIPTION "Vole Machine Simulator & GUI"
        LANGUAGES CXX)

file(GLOB_RECURSE sources src/*.cpp src/*.h)
add_executable(vole-sim ${sources}
        vole.h
        vole.cpp)
set_property(TARGET vole-sim PROPERTY CXX_STANDARD 17)
set_property(TARGET vole-sim PROPERTY CXX_STANDARD_REQUIRED On)
set_property(TARGET vole-sim PROPERTY CXX_EXTENSIONS Off)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(vole-sim PRIVATE -Wall -Wextra -fno-omit-frame-pointer -fsanitize=address)
    target_link_options(vole-sim PRIVATE -fno-omit-frame-pointer -fsanitize=address)
  endif()
endif()
