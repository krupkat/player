#include <cstring>
#include <optional>

#include <gst/gst.h>
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include "pipeline.h"
#include "sdl_utils.h"

int main(int argc, char **argv) {
  auto sdl = player::InitSDL();
  if (not sdl) {
    return -1;
  }

  auto w1 = player::InitWindow("Player", 1024, 768, SDL_WINDOW_RESIZABLE);
  if (not w1) {
    return -1;
  }

  auto w2 =
      player::InitPopupWindow(w1->window.get(), 100, 100,
                              SDL_WINDOW_POPUP_MENU | SDL_WINDOW_TRANSPARENT |
                                  SDL_WINDOW_NOT_FOCUSABLE | SDL_WINDOW_HIDDEN);
  if (not w2) {
    return -1;
  }

  // if (!SDL_SetWindowParent(w2->window.get(), w1->window.get())) {
  //   spdlog::error("Error setting parent window! {}", SDL_GetError());
  // }

  void *display = nullptr;
  void *surface = nullptr;

  if (sdl->wm == "wayland") {
    display =
        SDL_GetPointerProperty(SDL_GetWindowProperties(w1->window.get()),
                               SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
    surface =
        SDL_GetPointerProperty(SDL_GetWindowProperties(w1->window.get()),
                               SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
  }

  gst_init(&argc, &argv);

  player::VideoPipeline pipe(
      "/home/tom/Downloads/bourne_ultimatum_trailer/video.mp4", display,
      surface);

  pipe.Play();

  bool done = false;
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event) > 0) {
      if (event.type == SDL_EVENT_QUIT) {
        done = true;
      }
      if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
          event.window.windowID == SDL_GetWindowID(w1->window.get())) {
        done = true;
      }
      if (event.type == SDL_EVENT_WINDOW_RESIZED &&
          event.window.windowID == SDL_GetWindowID(w1->window.get())) {
        pipe.Resize(event.window.data1, event.window.data2);

        // SDL_SetWindow
      }
      if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (event.button.button == SDL_BUTTON_RIGHT) {
          pipe.Pause();
        }
        if (event.button.button == SDL_BUTTON_LEFT) {
          pipe.Play();
        }
      }
    }

    if (pipe.ProcessMessages()) {
      done = true;
    }

    SDL_RenderPresent(w1->renderer.get());

    SDL_SetRenderDrawColor(w2->renderer.get(), 128, 172, 62, 128);
    auto rect = SDL_FRect{0.f, 0.f, 100.f, 100.f};
    SDL_RenderFillRect(w2->renderer.get(), &rect);
    SDL_RenderPresent(w2->renderer.get());

    SDL_ShowWindow(w2->window.get());
  }

  return 0;
}
