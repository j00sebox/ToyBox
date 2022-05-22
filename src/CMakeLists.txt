# target_sources(src_lib
#     PRIVATE
#         ${CMAKE_CURRENT_LIST_DIR}/Application.cpp
#         ${CMAKE_CURRENT_LIST_DIR}/Camera.cpp
#         ${CMAKE_CURRENT_LIST_DIR}/FileOperations.cpp
#         ${CMAKE_CURRENT_LIST_DIR}/GLError.cpp
#         ${CMAKE_CURRENT_LIST_DIR}/GLTFLoader.cpp
#         ${CMAKE_CURRENT_LIST_DIR}/Input.cpp
#         ${CMAKE_CURRENT_LIST_DIR}/Window.cpp
#     PUBLIC
#         ${CMAKE_CURRENT_LIST_DIR}/Application.h
#         ${CMAKE_CURRENT_LIST_DIR}/Camera.h
#         ${CMAKE_CURRENT_LIST_DIR}/FileOperations.h
#         ${CMAKE_CURRENT_LIST_DIR}/GLError.h
#         ${CMAKE_CURRENT_LIST_DIR}/GLTFLoader.h
#         ${CMAKE_CURRENT_LIST_DIR}/ImGuiHelper.h
#         ${CMAKE_CURRENT_LIST_DIR}/Input.h
#         ${CMAKE_CURRENT_LIST_DIR}/Log.h
#         ${CMAKE_CURRENT_LIST_DIR}/Window.h
# )

include(${CMAKE_CURRENT_LIST_DIR}/scene/CMakeLists.txt)

list(APPEND SRCS
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Application.h
        ${CMAKE_CURRENT_LIST_DIR}/Camera.h
        ${CMAKE_CURRENT_LIST_DIR}/FileOperations.h
        ${CMAKE_CURRENT_LIST_DIR}/GLError.h
        ${CMAKE_CURRENT_LIST_DIR}/GLTFLoader.h
        ${CMAKE_CURRENT_LIST_DIR}/ImGuiHelper.h
        ${CMAKE_CURRENT_LIST_DIR}/Input.h
        ${CMAKE_CURRENT_LIST_DIR}/Log.h
        ${CMAKE_CURRENT_LIST_DIR}/Window.h
        ${CMAKE_CURRENT_LIST_DIR}/Application.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Camera.cpp
        ${CMAKE_CURRENT_LIST_DIR}/FileOperations.cpp
        ${CMAKE_CURRENT_LIST_DIR}/GLError.cpp
        ${CMAKE_CURRENT_LIST_DIR}/GLTFLoader.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Input.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Window.cpp
)



# add_subdirectory(components)
# add_subdirectory(entities)
# add_subdirectory(events)
# add_subdirectory(profiler)
# add_subdirectory(renderer)
# add_subdirectory(scene)

# find_library(GLFW glad)
# target_link_libraries(src_lib PUBLIC ${GLFW})

# target_compile_definitions(src_lib PUBLIC USE_GLAD)