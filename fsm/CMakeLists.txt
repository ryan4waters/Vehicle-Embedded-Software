cmake_minimum_required(VERSION 3.16)
project(FSM_Base_Package_V2 C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(fsm_core src/FSM_Core.c)
target_include_directories(fsm_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(fsm_example
    example/main.c
    example/PDU_FSM_Example.c)
target_include_directories(fsm_example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/example)
target_link_libraries(fsm_example PRIVATE fsm_core)
