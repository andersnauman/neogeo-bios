cmake_minimum_required(VERSION 3.10)
project(checksum C)

add_executable(${PROJECT_NAME} checksum.c)

install(TARGETS ${PROJECT_NAME} DESTINATION tools)