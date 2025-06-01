file(GLOB SOURCES ./*.c)

add_library(cmdproc STATIC ${SOURCES})

target_include_directories(cmdproc PUBLIC .)