cmake_minimum_required(VERSION 3.23.1)

project(toy_box)

add_subdirectory(src)
add_executable(${PROJECT_NAME} **.cpp)