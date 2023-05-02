# RoadRealm
Group #8: Road Network Planning Game

_Helpful Linking or Setting Up Links_
1. CMAKE. (2023). FindOpenGL. FindOpenGL - CMake 3.26.3 Documentation. Retrieved April 17, 2023, from https://cmake.org/cmake/help/latest/module/FindOpenGL.html 
2. Wilke, A., &amp; rebli. (2018, April 22). CMAKE doesn't link libglu using qtopengl. Stack Overflow. Retrieved April 17, 2023, from https://stackoverflow.com/a/6826716
3. Zeitler, W. (2015, June 15). Mingw64: add a statically linked library adds libstdc++ dependency. CMAKE. Retrieved April 17, 2023, from https://cmake.org/pipermail/cmake/2019-June/069611.html 


### Current Directory Structure For The Project.
---
>  **Disclaimer** 
The project files will be inside the **RoadNet** directory. If you are not intending on using cmake, you can get the files pertain to the project in RoadNet. The rest of the folders and files are the necessary Opengl essentials (GLFW, glad) along with the supplied instructor's libaries.

 - GraphicsLinking
    - include
    - lib
 - RoadNet
    - 4-Demo-MouseTest.cpp (row and column demostration)
    - 6-Demo-Sprite.cpp
    - A-StarIsBorn.cpp
    - main.cpp (assignment 1)
- CMakeLists.txt
- default_main.cpp (same concept as RoadNet/main.cpp doesn't use instructor libraries)
- glad.c

### CMakeLists.txt
---
If you are developing on different operating systems (linux, ios, freebsd, etc..), or using different IDEs, the functionalities offered by the visual studio IDE does not translate in completeness to all these platforms. With cmake the benefit is portability. A few imporant properties will be highlighted, references will be left for more insight.

- Line 1 - 3, _**cmake_minimum_required(<name>) , project(<name>)**_ 
    - The Cmake version, project-name are specified.
- Line 4 - 5, _**set(CMAKE_CXX_STANDARD 20) , set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")**_ :
    - Completely Optional, However if you are interested in running the .exe file (**RoadRealm.exe**), you will be need these flags to be complied and linked.
- Line 15, _**add_executable()**_
    - Execution files (*.cpp, *.h).

Examples of _**add_executable()**_ 
 
&check; Good Example
```sh
add_executable(RoadRealm glad.c
        # Road Network Team 8 Executable File Section
        RoadNet/4-Demo-MouseTest.cpp # is a main file

        # Professor's Executable File Section
        GraphicsLinking/lib/GLXtras.cpp  # is not a main file
        GraphicsLinking/lib/Draw.cpp  # is not a main file)
```
&check; Good Example
```sh
add_executable(RoadRealm glad.c
        # Road Network Team 8 Executable File Section
        RoadNet/4-Demo-MouseTest.cpp # is a main file
        RoadNet/AIRunner/Maze.cpp # is not a main file

        # Professor's Executable File Section
        GraphicsLinking/lib/GLXtras.cpp  # is not a main file
        GraphicsLinking/lib/Draw.cpp  # is not a main file
        GraphicsLinking/lib/Text.cpp  # is not a main file) 
```
&cross; Bad Example, 3 files with a **main()** sub-routine acting as the main execution point for program leading to a conflict. 
```sh
add_executable(RoadRealm glad.c
        # Road Network Team 8 Executable File Section
        RoadNet/4-Demo-MouseTest.cpp # is a main file
        RoadNet/6-Demo-Sprite.cpp # is a main file
        RoadNet/main.cpp # is a main file

        # Professor's Executable File Section
        GraphicsLinking/lib/GLXtras.cpp  # is not a main file
        GraphicsLinking/lib/Draw.cpp  # is not a main file )
```
