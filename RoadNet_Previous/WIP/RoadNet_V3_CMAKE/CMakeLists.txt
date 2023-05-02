cmake_minimum_required(VERSION 3.24)
project(RoadRealm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")

include_directories(GraphicsLinking/include)
include_directories(GraphicsLinking/include/GL)
include_directories(GraphicsLinking/include/glad)
include_directories(GraphicsLinking/include/GLFW)
include_directories(GraphicsLinking/include/OpenVR)

link_directories(GraphicsLinking/lib)

add_executable(RoadRealm glad.c
        # Road Network Team 8 Executable File Section
        RoadNet/RoadNetMain.cpp

        # Professor's Executable File Section
        GraphicsLinking/lib/GLXtras.cpp
        GraphicsLinking/lib/Draw.cpp
        GraphicsLinking/lib/IO.cpp
        GraphicsLinking/lib/Letters.cpp
        GraphicsLinking/lib/Text.cpp)

find_package(OpenGL REQUIRED)

target_link_libraries(RoadRealm OpenGL::GL)
target_link_libraries(RoadRealm OpenGL::GLU)
target_link_libraries(RoadRealm glfw3)