#pragma once

#include <memory>
#include <vector>

#include <gst/gst.h>

namespace player {
template <typename TGstType>
class GstDeleter {
 public:
  void operator()(TGstType *elem) { gst_object_unref(elem); }
};

template <typename TGlibType>
class GlibDeleter {
 public:
  void operator()(TGlibType *elem) { g_free(elem); }
};

using GstElementPtr = std::unique_ptr<GstElement, GstDeleter<GstElement>>;
using GstBusPtr = std::unique_ptr<GstBus, GstDeleter<GstBus>>;
using GstPadPtr = std::unique_ptr<GstPad, GstDeleter<GstPad>>;
using GstObjectPtr = std::unique_ptr<GstObject, GstDeleter<GstObject>>;

using GlibCharPtr = std::unique_ptr<gchar, GlibDeleter<gchar>>;
using GlibErrorPtr = std::unique_ptr<GError, decltype(&g_error_free)>;

using GstCapsPtr = std::unique_ptr<GstCaps, decltype(&gst_caps_unref)>;
using GstMessagePtr = std::unique_ptr<GstMessage, decltype(&gst_message_unref)>;
using GstContextPtr = std::unique_ptr<GstContext, decltype(&gst_context_unref)>;
using GstTagListPtr =
    std::unique_ptr<GstTagList, decltype(&gst_tag_list_unref)>;

enum class LinkResult { SUCCESS, ERROR };

[[nodiscard]] LinkResult LinkPads(GstPad *src, GstPad *sink);

[[nodiscard]] LinkResult LinkElements(GstElement *src, GstElement *sink);

[[nodiscard]] LinkResult LinkAll(
    const std::vector<std::vector<GstElement *>> &elements_to_link);

GstElementPtr Make(const char *element, const char *name = nullptr);

template <typename TGstType>
std::string GetObjectName(TGstType *elem) {
  if (auto name = GlibCharPtr{gst_object_get_name(GST_OBJECT_CAST(elem))}) {
    return std::string{name.get()};
  }
  return "[unknown]";
}

template <typename TGstType>
std::string GetParentName(TGstType *elem) {
  if (auto parent =
          GstObjectPtr{gst_object_get_parent(GST_OBJECT_CAST(elem))}) {
    return GetObjectName(parent.get());
  }
  return "[unknown]";
}

void PrintErrorMessage(GstMessage *msg);

void PrintStateChangedMessage(GstMessage *msg);

void PrintStreamStatusMessage(GstMessage *msg);

void PrintTagMessage(GstMessage *msg);

}  // namespace player
