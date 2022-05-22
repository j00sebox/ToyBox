cmake_minimum_required(VERSION 3.23.1)

project(toy_box CXX)
set(CMAKE_CXX_STANDARD 20)

set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include)

# Download submodules
# find_package(Git QUIET)
# if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
#     option(GIT_SUBMODULE "Grab submodules during build" ON)
#     if(GIT_SUBMODULE)
#         message(STATUS "Submodule update")
#         execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
#                         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#                         RESULT_VARIABLE GIT_SUBMOD_RESULT
#                     )
#         if(NOT GIT_SUBMOD_RESULT EQUAL "0")
#             message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}")
#         endif()
#     endif()
# endif()

add_subdirectory(external/glfw)
add_subdirectory(external/glad)
add_subdirectory(external/mathz)
add_subdirectory(external/imgui)
add_subdirectory(external/json)

include(src/CMakeLists.txt)
add_executable(${PROJECT_NAME} ${SRCS})

target_precompile_headers(${PROJECT_NAME} PUBLIC src/pch.h)

target_include_directories(${PROJECT_NAME}
    PUBLIC src
    PUBLIC src/renderer
    PUBLIC src/scene
    PUBLIC src/components
    PUBLIC src/entities
    PUBLIC src/events
    PUBLIC src/profiler
    PUBLIC external/glfw/include
    PUBLIC external/glad/include
    PUBLIC external/mathz/include
    PUBLIC external/imgui
    PUBLIC external/json/include
)

# target_link_directories(${PROJECT_NAME}
#     PRIVATE external/glfw/src
#     PRIVATE exeternal/glad/src
# )

target_link_libraries(${PROJECT_NAME} glfw glad imgui)