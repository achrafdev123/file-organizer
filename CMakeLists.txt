cmake_minimum_required(VERSION 3.10)
project(FileOrganizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(organizer organizer.cpp)

if(MSVC)
    target_compile_options(organizer PRIVATE /W4 /EHsc)
else()
    target_compile_options(organizer PRIVATE -Wall -Wextra -pedantic)
endif()
