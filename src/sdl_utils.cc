
#include "sdl_utils.h"

#include <cstring>
#include <optional>

#include <SDL.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <spdlog/spdlog.h>

namespace player {
namespace {

void PrintRenderInfo(SDL_Renderer *renderer) {
  SDL_RendererInfo info;

  if (SDL_GetRendererInfo(renderer, &info) == 0) {
    spdlog::info("Current SDL_Renderer: {}", info.name);
  } else {
    spdlog::error("Failed to get SDL_RendererInfo: {}", SDL_GetError());
  }
}

void PrintWindowManagerInfo(const SDL_SysWMinfo &info) {
  switch (info.subsystem) {
    case SDL_SYSWM_X11:
      spdlog::info("Window manager: X11");
      break;
    case SDL_SYSWM_WAYLAND:
      spdlog::info("Window manager: Wayland");
      break;
    default:
      spdlog::info("Window manager: Unknown");
      break;
  }
}
}  // namespace

std::optional<SDLContext> InitSDL(int width, int height) {
  const bool has_wayland_support = (SDL_VideoInit("wayland") == 0);

  if (has_wayland_support) {
    SDL_SetHint(SDL_HINT_VIDEODRIVER, "wayland,x11");
  }

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    spdlog::error("Error: {}", SDL_GetError());
    return {};
  }
  auto sdl_quit = utils::DestructorCallback([] { SDL_Quit(); });

  SDLWindowPtr window = {
      SDL_CreateWindow("Player", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       width, height, SDL_WINDOW_RESIZABLE),
      &SDL_DestroyWindow};

  if (window == nullptr) {
    spdlog::error("Error creating SDL_Window! {}", SDL_GetError());
    return {};
  }

  SDLRendererPtr renderer = {
      SDL_CreateRenderer(window.get(), -1,
                         SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED),
      &SDL_DestroyRenderer};

  if (renderer == nullptr) {
    spdlog::error("Error creating SDL_Renderer! {}", SDL_GetError());
    return {};
  }

  PrintRenderInfo(renderer.get());

  SDL_SysWMinfo wm_info;
  SDL_VERSION(&wm_info.version);

  if (SDL_GetWindowWMInfo(window.get(), &wm_info) == SDL_FALSE) {
    spdlog::error("Error querying window manager info! {}", SDL_GetError());
    return {};
  }

  PrintWindowManagerInfo(wm_info);

  return SDLContext{std::move(sdl_quit), std::move(window), std::move(renderer),
                    wm_info};
}
}  // namespace player
