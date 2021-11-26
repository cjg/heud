cmake_minimum_required(VERSION 3.18)

project(heud)

execute_process(COMMAND uname -r
        OUTPUT_VARIABLE KERNEL_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)

set(KERNEL_HEADERS "/lib/modules/${KERNEL_VERSION}/source/include")

include_directories(${KERNEL_HEADERS})

add_executable(heud
        heud.c
        key_codes.c)