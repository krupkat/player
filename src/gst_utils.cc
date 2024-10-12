#include "gst_utils.h"

#include <gst/gst.h>
#include <spdlog/spdlog.h>

namespace player {
namespace {
std::pair<GlibErrorPtr, GlibCharPtr> ExtractErrorMessage(GstMessage *msg) {
  GError *err;
  gchar *debug_info;
  gst_message_parse_error(msg, &err, &debug_info);
  return {GlibErrorPtr{err, &g_error_free}, GlibCharPtr{debug_info, {}}};
}

GstTagListPtr ExtractTagList(GstMessage *msg) {
  GstTagList *tags = nullptr;
  gst_message_parse_tag(msg, &tags);
  return {tags, &gst_tag_list_unref};
}

const char *ToString(const GstStreamStatusType &stream_status_type) {
  switch (stream_status_type) {
    case GST_STREAM_STATUS_TYPE_CREATE:
      return "CREATE";
    case GST_STREAM_STATUS_TYPE_ENTER:
      return "ENTER";
    case GST_STREAM_STATUS_TYPE_LEAVE:
      return "LEAVE";
    case GST_STREAM_STATUS_TYPE_DESTROY:
      return "DESTROY";
    case GST_STREAM_STATUS_TYPE_START:
      return "START";
    case GST_STREAM_STATUS_TYPE_PAUSE:
      return "PAUSE";
    case GST_STREAM_STATUS_TYPE_STOP:
      return "STOP";
    default:
      return "UNKNOWN";
  }
}
}  // namespace

GstElementPtr Make(const char *element, const char *name) {
  if (auto *elem = gst_element_factory_make(element, name)) {
    return {elem, {}};
  }
  spdlog::error("Couldn't create element: {}", element);
  return {};
}

LinkResult LinkPads(GstPad *src, GstPad *sink) {
  auto ret = gst_pad_link(src, sink);
  if (GST_PAD_LINK_FAILED(ret)) {
    spdlog::error("Link failed {} -> {}", GetParentName(src),
                  GetParentName(sink));
    return LinkResult::ERROR;
  }
  return LinkResult::SUCCESS;
}

LinkResult LinkElements(GstElement *src, GstElement *sink) {
  auto ret = gst_element_link(src, sink);
  if (ret == FALSE) {
    spdlog::error("Link failed {} -> {}", GetObjectName(src),
                  GetObjectName(sink));
    return LinkResult::ERROR;
  }
  return LinkResult::SUCCESS;
}

LinkResult LinkAll(
    const std::vector<std::vector<GstElement *>> &elements_to_link) {
  bool success = true;

  for (const auto &elements : elements_to_link) {
    for (int i = 1; i < elements.size(); i++) {
      success |=
          LinkElements(elements[i - 1], elements[i]) == LinkResult::SUCCESS;
    }
  }

  return success ? LinkResult::SUCCESS : LinkResult::ERROR;
}

void PrintErrorMessage(GstMessage *msg) {
  auto [err, debug_info] = ExtractErrorMessage(msg);

  spdlog::error("[error] {}: {}", GetObjectName(msg->src), err->message);
  if (debug_info) {
    spdlog::error("[debug info] {}", debug_info.get());
  }
}

void PrintStateChangedMessage(GstMessage *msg) {
  GstState old_state, new_state;
  gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);

  spdlog::info("[state changed] {}: {} -> {}", GetObjectName(msg->src),
               gst_element_state_get_name(old_state),
               gst_element_state_get_name(new_state));
}

void PrintStreamStatusMessage(GstMessage *msg) {
  GstStreamStatusType streamStatus;
  gst_message_parse_stream_status(msg, &streamStatus, NULL);

  spdlog::info("[stream status] {}({}): {}", GetParentName(msg->src),
               GetObjectName(msg->src), ToString(streamStatus));
}

void PrintTagMessage(GstMessage *msg) {
  auto tag_list = ExtractTagList(msg);
  auto tag_list_str = GlibCharPtr{gst_tag_list_to_string(tag_list.get())};
  spdlog::info("[tag] {}: {}", GetObjectName(msg->src), tag_list_str.get());
}

}  // namespace player
