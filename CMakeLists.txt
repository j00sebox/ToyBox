cmake_minimum_required(VERSION 3.23.1)

project(toy_box CXX)
set(CMAKE_CXX_STANDARD 20)

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

add_subdirectory(src)
add_subdirectory(external/glfw)
add_subdirectory(external/glad)
add_subdirectory(external/mathz)
add_executable(${PROJECT_NAME} src/main.cpp)

target_include_directories(${PROJECT_NAME} 
    PUBLIC external/glfw/include
    PUBLIC external/glad/include
    PUBLIC external/mathz/include
)

target_link_directories(${PROJECT_NAME}
    PRIVATE external/glfw/src
    PRIVATE exeternal/glad/src
)

target_link_libraries(${PROJECT_NAME} glfw glad)