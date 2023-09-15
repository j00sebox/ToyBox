#include(${CMAKE_CURRENT_LIST_DIR}/scene/CMakeLists.txt)
#include(${CMAKE_CURRENT_LIST_DIR}/renderer/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/rendering/CMakeLists.txt)
#include(${CMAKE_CURRENT_LIST_DIR}/components/CMakeLists.txt)
#include(${CMAKE_CURRENT_LIST_DIR}/events/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/profiler/CMakeLists.txt)
#include(${CMAKE_CURRENT_LIST_DIR}/editor/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/util/CMakeLists.txt)

list(APPEND SRCS
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
#        ${CMAKE_CURRENT_LIST_DIR}/Entity.h
#        ${CMAKE_CURRENT_LIST_DIR}/Application.h
#        ${CMAKE_CURRENT_LIST_DIR}/FileOperations.h
#        ${CMAKE_CURRENT_LIST_DIR}/GLError.h
#        ${CMAKE_CURRENT_LIST_DIR}/GLTFLoader.h
#        ${CMAKE_CURRENT_LIST_DIR}/ImGuiHelper.h
#        ${CMAKE_CURRENT_LIST_DIR}/ImGuiHelper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Input.h
        ${CMAKE_CURRENT_LIST_DIR}/Log.h
        ${CMAKE_CURRENT_LIST_DIR}/Log.cpp
#        ${CMAKE_CURRENT_LIST_DIR}/Window.h
#        ${CMAKE_CURRENT_LIST_DIR}/ViewPort.h
#        ${CMAKE_CURRENT_LIST_DIR}/ViewPort.cpp
#        ${CMAKE_CURRENT_LIST_DIR}/Application.cpp
#        ${CMAKE_CURRENT_LIST_DIR}/FileOperations.cpp
#        ${CMAKE_CURRENT_LIST_DIR}/GLError.cpp
#        ${CMAKE_CURRENT_LIST_DIR}/GLTFLoader.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Input.cpp
#        ${CMAKE_CURRENT_LIST_DIR}/Window.cpp
#        ${CMAKE_CURRENT_LIST_DIR}/ModelLoader.h
#        ${CMAKE_CURRENT_LIST_DIR}/ModelLoader.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Engine.hpp
        ${CMAKE_CURRENT_LIST_DIR}/Engine.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Types.hpp
)
