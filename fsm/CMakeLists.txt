cmake_minimum_required(VERSION 3.15)
project(FSM_Base_Package C)

set(CMAKE_C_STANDARD 99)

include_directories(include)

add_executable(fsm_example
    src/FSM_Core.c
    example/PDU_FSM_Example.c
    example/main.c
)
