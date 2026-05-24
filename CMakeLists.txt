cmake_minimum_required(VERSION 3.20)
project(AirDefenseSim)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(AirDefenseSim ${SOURCES}
        src/stb_image.cpp)
target_link_libraries(AirDefenseSim PRIVATE OpenGL::GL glfw)
