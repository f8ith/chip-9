CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

project(chip9)

set(CMAKE_CXX_STANDARD 11)

find_package(SDL2 REQUIRED)
add_executable(chip9 main.cpp Chip8.cpp Chip8.h)
include_directories(chip9 ${SDL2_INCLUDE_DIRS})
target_link_libraries(chip9 ${SDL2_LIBRARIES})
link_directories("/opt/homebrew/lib")
include_directories("/opt/homebrew/include")
