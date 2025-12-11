cmake_minimum_required(VERSION 3.0)
project(RedisImageApp CXX)

# Set C++17 standard (more compatible with Buildroot)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)

# Find Hiredis library
pkg_check_modules(HIREDIS REQUIRED hiredis)

# Find SDL2 and SDL2_image
find_package(SDL2 REQUIRED)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

# Find nlohmann/json
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    find_path(NLOHMANN_JSON_INCLUDE_DIR
        NAMES nlohmann/json.hpp
        PATHS /usr/include /usr/local/include
    )
endif()

# Define the executable
add_executable(redis_image_viewer main.cpp app.cpp app_cfg.cpp)

# Include directories
target_include_directories(redis_image_viewer PRIVATE
    ${HIREDIS_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
)

# Add nlohmann/json include directory if found manually
if(NOT nlohmann_json_FOUND AND NLOHMANN_JSON_INCLUDE_DIR)
    target_include_directories(redis_image_viewer PRIVATE ${NLOHMANN_JSON_INCLUDE_DIR})
endif()

# Link all libraries
target_link_libraries(redis_image_viewer
    ${HIREDIS_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
)

# Link nlohmann/json if found as a package
if(nlohmann_json_FOUND)
    target_link_libraries(redis_image_viewer nlohmann_json::nlohmann_json)
endif()