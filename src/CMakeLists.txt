cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(gstsycl LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GST REQUIRED IMPORTED_TARGET gstreamer-1.0)
pkg_check_modules(GST_VIDEO REQUIRED IMPORTED_TARGET gstreamer-video-1.0)
pkg_check_modules(GST_WAYLAND REQUIRED IMPORTED_TARGET gstreamer-wayland-1.0)

find_package(SDL2 REQUIRED)
find_package(spdlog REQUIRED)

# player
add_executable(player player.cc sdl_utils.cc pipeline.cc gst_utils.cc)

target_link_libraries(player PRIVATE
    PkgConfig::GST
    PkgConfig::GST_VIDEO
    PkgConfig::GST_WAYLAND
    SDL2::SDL2
    spdlog::spdlog
)
