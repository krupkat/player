#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include <SDL.h>
#include <SDL_syswm.h>

#include "utils.h"

namespace player {

using SDLWindowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
using SDLRendererPtr =
    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;

struct SDLContext {
  utils::DestructorCallback sdl_quit;
  SDLWindowPtr window;
  SDLRendererPtr renderer;
  SDL_SysWMinfo wm_info;
};

std::optional<SDLContext> InitSDL(int width, int height);

}  // namespace player
