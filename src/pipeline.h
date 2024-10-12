#pragma once

#include <cstring>

#include <gst/gst.h>
#include <gst/video/videooverlay.h>

#include "gst_utils.h"

namespace player {

class VideoPipeline {
 public:
  explicit VideoPipeline(const char *input, void *display, void *surface);
  ~VideoPipeline();

  void Play();
  bool ProcessMessages();

  void *Display() { return display; }
  void *Surface() { return surface; }

  void SetOverlay(GstVideoOverlay *overlay) { this->overlay = overlay; }
  void Resize(int width, int height);

  void Pause();

 private:
  GstElementPtr pipeline;
  GstBusPtr bus;

  void *display;
  void *surface;

  GstVideoOverlay *overlay;
};
}  // namespace player
