cmake_minimum_required(VERSION 3.10)
project(cvm CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cvm
    lexer.cpp
    parser.cpp
    ast.cpp
    opcodes.cpp
    compiler.cpp
    vm.cpp
    main.cpp
)

if (MSVC)
    target_compile_options(cvm PRIVATE /W4)
else()
    target_compile_options(cvm PRIVATE -Wall -Wextra)
endif()
