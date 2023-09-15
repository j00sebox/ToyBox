cmake_minimum_required(VERSION 3.2)

project(ToyBox CXX)
set(CMAKE_CXX_STANDARD 20)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/engine)
set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw/include)

add_compile_definitions(DEBUG)

if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
else()
    add_compile_definitions(PLATFORM_LINUX)
endif()

# Download submodules
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    option(GIT_SUBMODULE "Grab submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT
                    )
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}")
        endif()
    endif()
endif()

add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/glad)
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/imgui)
add_subdirectory(thirdparty/json)
add_subdirectory(thirdparty/stb)
add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/assimp)
add_subdirectory(thirdparty/enkiTS)

include(engine/CMakeLists.txt)
add_executable(${PROJECT_NAME} ${SRCS})

target_precompile_headers(${PROJECT_NAME} PUBLIC engine/pch.h)

target_include_directories(${PROJECT_NAME}
    PUBLIC engine
    PUBLIC engine/renderer
    PUBLIC engine/scene
    PUBLIC engine/components
    PUBLIC engine/entities
    PUBLIC engine/events
    PUBLIC engine/profiler
    PUBLIC thirdparty/glfw/include
    PUBLIC thirdparty/glad/include
    PUBLIC thirdparty/glm
    PUBLIC thirdparty/imgui
    PUBLIC thirdparty/json/include
    PUBLIC thirdparty/stb
    PUBLIC thirdparty/spdlog/include
    PUBLIC thirdparty/assimp/include
    PUBLIC thirdparty/vma
    PUBLIC thirdparty/enkiTS/src
)

target_link_libraries(${PROJECT_NAME} glfw glad imgui stb spdlog assimp vulkan pthread enkiTS)