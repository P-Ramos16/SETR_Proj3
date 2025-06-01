# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Proj_3)

#  Add source files from main directory
target_sources(app PRIVATE
    src/main.c
)

#  Add include directories
target_include_directories(app PRIVATE
    src
    src/modules
)

#  Add subdirectory for modules
add_subdirectory(src/modules)
