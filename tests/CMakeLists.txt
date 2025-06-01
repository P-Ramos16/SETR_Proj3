cmake_minimum_required(VERSION 3.20)
project(CmdProc)

#  Set C standard
set(CMAKE_C_STANDARD 99)

#  Include modules directory
include_directories(${CMAKE_SOURCE_DIR}/modules)


add_subdirectory(modules)
add_subdirectory(Unity)

#  Test executables
add_executable(cmdproc_tests cmdproc_tests.c)
target_link_libraries(cmdproc_tests cmdproc unity)
add_test(cmdproc_tests cmdproc)

add_executable(PID_tests PID_tests.c)
target_link_libraries(PID_tests cmdproc unity)
add_test(PID_tests cmdproc)