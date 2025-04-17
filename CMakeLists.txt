# Aseprite PSD Library
# Copyright (C) 2019-2025 Igara Studio S.A.

cmake_minimum_required(VERSION 3.15)

project(psd CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(PSD_TOOLS "Compile psd tools" on)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

include_directories(.)

add_library(psd
  decoder.cpp
  image_resources.cpp
  psd.cpp
  stdio.cpp)

if(PSD_TOOLS)
  add_subdirectory(tools)
endif()
