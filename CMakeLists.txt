# This build was tested on Windows 10 using the Visual Studio 2019 64-bit toolchain
cmake_minimum_required(VERSION 3.21)
project(GantryExt
        VERSION 1.0
        DESCRIPTION "Extension Module for pixel-gantry-controlx"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(Database SHARED src/database.cpp src/database.h)
# The .lib files should be found automatically if they are installed in the standard place by the
# MySQL Connector C++ 64-bit msi installer, typically this is C:\Program Files\MySQL\Connector C++ 8.0\lib64\vs14
# In addition, to run during development, the dlls should be copied to somewhere on your PATH or the build directory
target_include_directories(Database PUBLIC "C:/Program Files/MySQL/Connector C++ 8.0/include")
target_link_directories(Database PUBLIC "C:/Program Files/MySQL/Connector C++ 8.0/lib64/vs14/")
target_link_libraries(Database PUBLIC mysqlcppconn8 libssl libcrypto)


add_executable(test_Database src/test_Database.cpp)
target_link_libraries(test_Database PUBLIC Database)


add_library(Vision SHARED src/vision.cpp src/vision.h src/utils.cpp src/utils.h)
# To get these libs, download the OpenCV 4.5.5 installer for windows and unpack it to C:
target_include_directories(Vision PUBLIC "C:/opencv/build/include")
target_link_directories(Vision PUBLIC "C:/opencv/build/x64/vc14/lib")
target_link_libraries(Vision PUBLIC opencv_world455)

add_executable(test_Vision src/test_Vision.cpp)
target_link_libraries(test_Vision PUBLIC Vision)
