# Aseprite PSD Library
# Copyright (C) 2019-2021 Igara Studio S.A.

cmake_minimum_required(VERSION 3.1)

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
  encoder.cpp
  image_resources.cpp
  psd.cpp
  stdio.cpp)

if(PSD_TOOLS)
  add_subdirectory(tools)
endif()
