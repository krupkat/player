
#include "sdl_utils.h"

#include <cstring>
#include <optional>

#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <spdlog/spdlog.h>

namespace player {
namespace {

void PrintRenderInfo(SDL_Renderer* renderer) {
  if (const auto* name = SDL_GetRendererName(renderer)) {
    spdlog::info("Current SDL_Renderer: {}", name);
  } else {
    spdlog::error("Failed to get SDL_RendererInfo: {}", SDL_GetError());
  }
}

}  // namespace

std::optional<SDLContext> InitSDL() {
  SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "wayland,x11");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    spdlog::error("Error: {}", SDL_GetError());
    return {};
  }
  auto sdl_quit = utils::DestructorCallback([] { SDL_Quit(); });

  std::string wm = SDL_GetCurrentVideoDriver();
  spdlog::info("Current window manager: {}", wm);

  return SDLContext{std::move(sdl_quit), std::move(wm)};
}

std::optional<SDLWindowContext> InitWindow(const char* title, int width,
                                           int height, SDL_WindowFlags flags) {
  SDLWindowPtr window = {nullptr, {&SDL_DestroyWindow}};
  SDLRendererPtr renderer = {nullptr, {&SDL_DestroyRenderer}};

  {
    SDL_Window* window_raw;
    SDL_Renderer* renderer_raw;

    if (!SDL_CreateWindowAndRenderer(title, width, height, flags, &window_raw,
                                     &renderer_raw)) {
      spdlog::error("Error creating SDL_Window! {}", SDL_GetError());
    }

    window = {window_raw, &SDL_DestroyWindow};
    renderer = {renderer_raw, &SDL_DestroyRenderer};
  }

  if (!SDL_SetRenderVSync(renderer.get(), 1)) {
    spdlog::error("Error setting VSync! {}", SDL_GetError());
    return {};
  }

  PrintRenderInfo(renderer.get());

  return SDLWindowContext{std::move(window), std::move(renderer)};
}

// SDL_CreatePopupWindow

std::optional<SDLWindowContext> InitPopupWindow(SDL_Window* parent, int width,
                                                int height,
                                                SDL_WindowFlags flags) {
  auto window =
      SDLWindowPtr{SDL_CreatePopupWindow(parent, 0, 0, width, height, flags),
                   &SDL_DestroyWindow};

  if (window.get() == nullptr) {
    spdlog::error("Error creating SDL_Window! {}", SDL_GetError());
    return {};
  }

  auto renderer = SDLRendererPtr{SDL_CreateRenderer(window.get(), NULL),
                                 &SDL_DestroyRenderer};

  if (renderer.get() == nullptr) {
    spdlog::error("Error creating SDL_Renderer! {}", SDL_GetError());
    return {};
  }

  if (!SDL_SetRenderVSync(renderer.get(), 1)) {
    spdlog::error("Error setting VSync! {}", SDL_GetError());
    return {};
  }

  // if (!SDL_SetWindowAlwaysOnTop(window.get(), false)) {
  //   spdlog::error("Error setting window flag! {}", SDL_GetError());
  //   return {};
  // }

  PrintRenderInfo(renderer.get());

  return SDLWindowContext{std::move(window), std::move(renderer)};
}

}  // namespace player
