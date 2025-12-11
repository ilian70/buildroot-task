cmake_minimum_required(VERSION 3.0)
project(RedisImageApp CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Hiredis library
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

# Find SDL2 and SDL2_image
find_package(SDL2 REQUIRED)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

# Define the executable
add_executable(redis_image_viewer main.cpp)

# Link all libraries
target_link_libraries(redis_image_viewer
    ${HIREDIS_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
)

# Include directories
target_include_directories(redis_image_viewer PRIVATE
    ${HIREDIS_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
)