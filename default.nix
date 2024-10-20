{ pkgs ? import <nixpkgs> { } }:

let
  sdl3 = pkgs.callPackage ./sdl3.nix {};
in

pkgs.mkShell {

  nativeBuildInputs = with pkgs; [
    cmake
    ninja
    pkg-config
  ];

  buildInputs = with pkgs; [
    clang-tools
    sdl3.dev
    spdlog
    wayland-scanner
    wayland-protocols
    wayland.dev
    # GStreamer
    gst_all_1.gstreamer
    gst_all_1.gst-plugins-base
    gst_all_1.gst-plugins-good
    gst_all_1.gst-plugins-bad
    gst_all_1.gst-plugins-ugly
    gst_all_1.gst-libav
    gst_all_1.gst-vaapi
    # Additional gst dependencies
    elfutils
    libunwind
    pcre2
    zstd
    # Additional gst-video dependencies
    orc
    libGL
    wayland
    xorg.libX11
    xorg.libXau
    xorg.libXdmcp
  ];
}
