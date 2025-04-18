cmake_minimum_required(VERSION 3.10)
project(hello_window VERSION 1.0.0)
cmake_policy(SET CMP0072 NEW)

find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(hello_window 
    src/config.h 
    src/main.cpp 
    src/glad.c
    src/mesh/cube_mesh.h
    src/mesh/cube_mesh.cpp
    src/mesh/box_mesh.h
    src/mesh/box_mesh.cpp
)

target_include_directories(hello_window 
    PRIVATE 
    dependencies
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(hello_window 
    glfw 
    OpenGL::GL
    ${OpenCV_LIBS}
)