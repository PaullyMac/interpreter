cmake_minimum_required(VERSION 3.30)
project(interpreter C)

set(CMAKE_C_STANDARD 17)

add_executable(interpreter scanner.c
        scanner.c
        token.h
        scanner_ci.c
)
