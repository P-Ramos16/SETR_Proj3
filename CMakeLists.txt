# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(basic_01_blinkDemo)

<<<<<<< HEAD
target_sources(app PRIVATE
  src/main.c
  src/buttons.c
  src/rtdb.c)
=======
target_sources(app PRIVATE src/main.c src/UART.c src/buttons.c src/mainBK.c)
>>>>>>> 2080724bbbfa384291901b1f7a70d5f027dc3406
