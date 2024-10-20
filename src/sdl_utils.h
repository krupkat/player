#pragma once

#include <memory>
#include <optional>
#include <string>

#include <SDL3/SDL.h>

#include "utils.h"

namespace player {

using SDLWindowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
using SDLRendererPtr =
    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;

struct SDLContext {
  utils::DestructorCallback sdl_quit;
  std::string wm;
};

struct SDLWindowContext {
  SDLWindowPtr window;
  SDLRendererPtr renderer;
};

std::optional<SDLContext> InitSDL();

std::optional<SDLWindowContext> InitWindow(const char* title, int width,
                                           int height, SDL_WindowFlags flags);

std::optional<SDLWindowContext> InitPopupWindow(SDL_Window* parent, int width,
                                                int height,
                                                SDL_WindowFlags flags);

}  // namespace player
