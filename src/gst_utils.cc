#include "gst_utils.h"

#include <gst/gst.h>

#include <spdlog/spdlog.h>

namespace player {
bool LinkPads(GstPad* src, GstPad* sink) {
  auto ret = gst_pad_link(src, sink);
  if (GST_PAD_LINK_FAILED(ret)) {
    spdlog::error("Link failed {} -> {}", GetParentName(src),
                  GetParentName(sink));
    return false;
  }
  return true;
}

bool LinkElements(GstElement* src, GstElement* sink) {
  auto ret = gst_element_link(src, sink);
  if (ret == FALSE) {
    spdlog::error("Link failed {} -> {}", GetObjectName(src),
                  GetObjectName(sink));
    return false;
  }
  return true;
}

bool LinkAll(const std::vector<std::vector<GstElement*>>& elements_to_link) {
  bool success = true;

  for (const auto& elements : elements_to_link) {
    for (int i = 1; i < elements.size(); i++) {
      success |= LinkElements(elements[i - 1], elements[i]);
    }
  }

  return success;
}
}  // namespace player
