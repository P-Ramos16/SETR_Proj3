# SPDX-License-Identifier: Apache-2.0
#  Add module source files
target_sources(app PRIVATE
    cmdproc.c
    rtdb.c
    PID.c
    buttons.c
)

#  Add module-specific include directories if needed
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
