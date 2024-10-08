#pragma once

#include <memory>
#include <vector>

#include <gst/gst.h>

namespace player {
template <typename TGstType>
class GstDeleter {
 public:
  void operator()(TGstType* elem) { gst_object_unref(elem); }
};

template <typename TGlibType>
class GlibDeleter {
 public:
  void operator()(TGlibType* elem) { g_free(elem); }
};

using GstElementPtr = std::unique_ptr<GstElement, GstDeleter<GstElement>>;
using GstBusPtr = std::unique_ptr<GstBus, GstDeleter<GstBus>>;
using GstPadPtr = std::unique_ptr<GstPad, GstDeleter<GstPad>>;
using GstObjectPtr = std::unique_ptr<GstObject, GstDeleter<GstObject>>;

using GlibString = std::unique_ptr<gchar, GlibDeleter<gchar>>;

using GstCapsPtr = std::unique_ptr<GstCaps, decltype(&gst_caps_unref)>;
using GstMessagePtr = std::unique_ptr<GstMessage, decltype(&gst_message_unref)>;
using GstContextPtr = std::unique_ptr<GstContext, decltype(&gst_context_unref)>;

[[nodiscard]] bool LinkPads(GstPad* src, GstPad* sink);

[[nodiscard]] bool LinkElements(GstElement* src, GstElement* sink);

[[nodiscard]] bool LinkAll(
    const std::vector<std::vector<GstElement*>>& elements_to_link);

inline GstElementPtr Make(const char* element, const char* name = nullptr) {
  return {gst_element_factory_make(element, name), {}};
}

template <typename TGstType>
std::string GetObjectName(TGstType* elem) {
  if (auto name = GlibString{gst_object_get_name(GST_OBJECT_CAST(elem))}) {
    return std::string{name.get()};
  }
  return "[unknown]";
}

template <typename TGstType>
std::string GetParentName(TGstType* elem) {
  if (auto parent =
          GstObjectPtr{gst_object_get_parent(GST_OBJECT_CAST(elem))}) {
    return GetObjectName(parent.get());
  }
  return "[unknown]";
}

}  // namespace player
