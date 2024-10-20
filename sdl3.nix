{
  lib,
  stdenv,
  fetchFromGitHub,
  alsa-lib,
  cmake,
  dbus,
  fcitx5,
  libdecor,
  libdrm,
  libffi,
  libjack2,
  libpulseaudio,
  libxkbcommon,
  mesa,
  nas,
  ninja,
  pipewire,
  sndio,
  systemdLibs,
  validatePkgConfig,
  wayland,
  wayland-scanner,
  libGL,
  xorg,
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "sdl3";
  version = "preview-3.1.3";

  src = fetchFromGitHub {
    owner = "libsdl-org";
    repo = "SDL";
    rev = "e292d1f5ace469f718d7b6b4dec8c28e37dcaa0e";
    hash = "sha256-S7yRcLHMPgq6+gec8l+ESxp2dJ+6Po/UNsBUXptQzMQ=";
  };

  outputs = [
    "lib"
    "dev"
    "out"
  ];

  nativeBuildInputs = [
    cmake
    ninja
    validatePkgConfig
    wayland-scanner
  ];

  buildInputs =
    lib.optionals stdenv.isLinux [
      alsa-lib
      dbus
      fcitx5
      libdecor
      libdrm
      libjack2
      libpulseaudio
      mesa # libgbm
      nas # libaudo
      pipewire
      sndio
      systemdLibs # libudev

      # SDL_VIDEODRIVER=wayland
      wayland
      libGL
      libffi

      # SDL_VIDEODRIVER=x11
      libxkbcommon
      xorg.libX11
      xorg.libXcursor
      xorg.libXext
      xorg.libXfixes
      xorg.libXi
      xorg.libXrandr
    ];

  cmakeFlags = [
    "-DSDL_ALSA_SHARED=OFF"
    "-DSDL_JACK_SHARED=OFF"
    "-DSDL_PIPEWIRE_SHARED=OFF"
    "-DSDL_PULSEAUDIO_SHARED=OFF"
    "-DSDL_SNDIO_SHARED=OFF"
    "-DSDL_X11_SHARED=OFF"
    "-DSDL_WAYLAND_SHARED=OFF"
    "-DSDL_WAYLAND_LIBDECOR_SHARED=OFF"
    "-DSDL_KMSDRM_SHARED=OFF"
    "-DSDL_HIDAPI_LIBUSB_SHARED=OFF"
  ];

  meta = {
    description = "Cross-platform development library (Pre-release version)";
    homepage = "https://libsdl.org";
    license = lib.licenses.zlib;
    maintainers = with lib.maintainers; [ getchoo ];
    pkgConfigModules = [ "sdl3" ];
  };
})
