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

# Define the executable
add_executable(redis_image_viewer main.cpp app.cpp app_cfg.cpp json11.cpp)

# Include directories
target_include_directories(redis_image_viewer PRIVATE
    ${HIREDIS_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
)

# Link all libraries
target_link_libraries(redis_image_viewer
    ${HIREDIS_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
)

# Install the binary and default config into the target rootfs
install(TARGETS redis_image_viewer RUNTIME DESTINATION bin)
install(FILES app.cfg.json DESTINATION etc/redis-image-viewer RENAME config.json)
install(DIRECTORY images/ DESTINATION var/lib/redis-image-viewer/images FILES_MATCHING PATTERN "*.png")