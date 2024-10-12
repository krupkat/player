#include <cstring>
#include <optional>

#include <gst/gst.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <spdlog/spdlog.h>

#include "pipeline.h"
#include "sdl_utils.h"

int main(int argc, char **argv) {
  auto sdl = player::InitSDL(1024, 768);
  if (not sdl) {
    return -1;
  }

  void *display = nullptr;
  void *surface = nullptr;

  if (sdl->wm_info.subsystem == SDL_SYSWM_WAYLAND) {
    spdlog::info("got wl info");
    display = sdl->wm_info.info.wl.display;
    surface = sdl->wm_info.info.wl.surface;
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
      if (event.type == SDL_QUIT) {
        done = true;
      }
      if (event.type == SDL_WINDOWEVENT &&
          event.window.event == SDL_WINDOWEVENT_CLOSE &&
          event.window.windowID == SDL_GetWindowID(sdl->window.get())) {
        done = true;
      }
      if (event.type == SDL_WINDOWEVENT &&
          event.window.event == SDL_WINDOWEVENT_RESIZED &&
          event.window.windowID == SDL_GetWindowID(sdl->window.get())) {
        pipe.Resize(event.window.data1, event.window.data2);
      }
      if (event.type == SDL_MOUSEBUTTONUP) {
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

    SDL_RenderPresent(sdl->renderer.get());
  }

  return 0;
}
