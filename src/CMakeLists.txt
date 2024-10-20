cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(gstsycl LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GST REQUIRED IMPORTED_TARGET gstreamer-1.0)
pkg_check_modules(GST_VIDEO REQUIRED IMPORTED_TARGET gstreamer-video-1.0)
pkg_check_modules(GST_WAYLAND REQUIRED IMPORTED_TARGET gstreamer-wayland-1.0)

find_package(SDL3 REQUIRED)
find_package(spdlog REQUIRED)

# player
add_executable(player player.cc sdl_utils.cc pipeline.cc gst_utils.cc)

target_link_libraries(player PRIVATE
    PkgConfig::GST
    PkgConfig::GST_VIDEO
    PkgConfig::GST_WAYLAND
    SDL3::SDL3
    spdlog::spdlog
)

#player2

pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client>=1.18)
pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)

find_program(WAYLAND_SCANNER NAMES wayland-scanner)

add_executable(player2 player2.cc)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/wayland-generated-protocols")
target_include_directories(player2 PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/wayland-generated-protocols")

macro(WaylandProtocolGen _XML _PROTL)
    set(_WAYLAND_PROT_C_CODE "${CMAKE_CURRENT_BINARY_DIR}/wayland-generated-protocols/${_PROTL}-protocol.c")
    set(_WAYLAND_PROT_H_CODE "${CMAKE_CURRENT_BINARY_DIR}/wayland-generated-protocols/${_PROTL}-client-protocol.h")

    add_custom_command(
        OUTPUT "${_WAYLAND_PROT_H_CODE}"
        DEPENDS "${_XML}"
        COMMAND "${WAYLAND_SCANNER}"
        ARGS client-header "${_XML}" "${_WAYLAND_PROT_H_CODE}"
    )

    add_custom_command(
        OUTPUT "${_WAYLAND_PROT_C_CODE}"
        DEPENDS "${_WAYLAND_PROT_H_CODE}"
        COMMAND "${WAYLAND_SCANNER}"
        ARGS private-code "${_XML}" "${_WAYLAND_PROT_C_CODE}"
    )

    target_sources(player2 PRIVATE "${_WAYLAND_PROT_C_CODE}")
endmacro()

WaylandProtocolGen("${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml" "xdg-shell")

target_link_libraries(player2 PRIVATE
    SDL3::SDL3
    PkgConfig::WAYLAND_CLIENT
)

