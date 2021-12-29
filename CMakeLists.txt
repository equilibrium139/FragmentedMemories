cmake_minimum_required(VERSION 3.22)

project(FragmentedMemories LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(FragmentedMemories src/Main.cpp
                                  src/AnimatedModel.h
                                  src/AnimatedModel.cpp
                                  src/Camera.h
                                  src/Game.h
                                  src/Game.cpp
                                  src/Light.h
                                  src/Material.h
                                  src/Mesh.h
                                  src/Mesh.cpp
                                  src/Model.h
                                  src/Model.cpp
                                  src/Renderer.h
                                  src/Renderer.cpp
                                  src/Scene.h
                                  src/Scene.cpp
                                  src/Shader.h
                                  src/Shader.cpp
                                  src/Texture.h
                                  src/Texture.cpp
                                  src/stb_image.cpp
)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

find_path(STB_INCLUDE_DIRS "stb.h")

target_include_directories(FragmentedMemories PRIVATE ${STB_INCLUDE_DIRS})
target_link_libraries(FragmentedMemories glfw
                                         glad::glad
                                         glm::glm
)
# target_include_directories(FragmentedMemories PUBLIC ${glfw_SOURCE_DIR}/glfw)

if(MSVC)
  target_compile_options(FragmentedMemories PRIVATE /W4)
else()
  target_compile_options(FragmentedMemories PRIVATE -Wall -Wextra -pedantic)
endif()

if (WIN32)
    # add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    #                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #                    "$<TARGET_FILE_DIR:assimp>/$<TARGET_FILE_NAME:assimp>"
    #                    $<TARGET_FILE_DIR:FragmentedMemories>
    # )

    add_custom_command(TARGET FragmentedMemories POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_SOURCE_DIR}/data"
                       $<TARGET_FILE_DIR:FragmentedMemories>
    )               
endif()