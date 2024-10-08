#include "pipeline.h"

#include <cstring>
#include <functional>
#include <memory>
#include <vector>

#define GST_USE_UNSTABLE_API

#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <gst/wayland/wayland.h>

#include <glib-2.0/glib/gstrfuncs.h>
#include <spdlog/spdlog.h>

namespace player {

namespace {

void PadAdded(GstElement* element, GstPad* pad, gpointer user_data) {
  GstElement* pipeline = GST_ELEMENT(user_data);

  auto caps = GstCapsPtr{gst_pad_get_current_caps(pad), &gst_caps_unref};
  GstStructure* caps_struct = gst_caps_get_structure(caps.get(), 0);
  const gchar* media_type = gst_structure_get_name(caps_struct);

  GstElement* queue = nullptr;
  if (g_str_has_prefix(media_type, "video")) {
    queue = gst_bin_get_by_name(GST_BIN(pipeline), "queuevideo");
  } else if (g_str_has_prefix(media_type, "audio")) {
    queue = gst_bin_get_by_name(GST_BIN(pipeline), "queueaudio");
  }

  if (queue) {
    auto sinkpad = GstPadPtr{gst_element_get_static_pad(queue, "sink")};
    if (not LinkPads(pad, sinkpad.get())) {
      exit(1);
    }
  } else {
    spdlog::error("Cannot handle: {}", media_type);
  }
}

GstBusSyncReply BusSyncHandler(GstBus* bus, GstMessage* message,
                               gpointer user_data) {
  VideoPipeline* pipe = static_cast<VideoPipeline*>(user_data);

  // these two messages have to be handled in sync mode,
  // otherwise the sink will create its own context and window

  if (gst_is_wl_display_handle_need_context_message(message)) {
    spdlog::info("Setting GstContext for wayland");
    auto display_handle = (struct wl_display*)pipe->Display();
    auto context = GstContextPtr{
        gst_wl_display_handle_context_new(display_handle), &gst_context_unref};
    gst_element_set_context(GST_ELEMENT(GST_MESSAGE_SRC(message)),
                            context.get());
    gst_message_unref(message);
    return GST_BUS_DROP;
  }

  if (gst_is_video_overlay_prepare_window_handle_message(message)) {
    spdlog::info("Setting window handle for wayland");
    GstVideoOverlay* videoOverlay = GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message));
    pipe->SetOverlay(videoOverlay);
    auto window_handle = (struct wl_surface*)pipe->Surface();
    gst_video_overlay_set_window_handle(videoOverlay, (guintptr)window_handle);
    gst_video_overlay_set_render_rectangle(videoOverlay, 0, 0, 1024, 768);
    gst_message_unref(message);
    return GST_BUS_DROP;
  }

  return GST_BUS_PASS;
}

bool ProcessMessage(GstMessage* msg) {
  bool terminate = false;

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {
      GError* err;
      gchar* debug_info;

      gst_message_parse_error(msg, &err, &debug_info);
      g_printerr("Error received from element %s: %s\n",
                 GST_OBJECT_NAME(msg->src), err->message);
      g_printerr("Debugging information: %s\n",
                 debug_info ? debug_info : "none");
      g_clear_error(&err);
      g_free(debug_info);
      terminate = true;
      break;
    }
    case GST_MESSAGE_EOS: {
      spdlog::info("End-Of-Stream reached");
      terminate = true;
      break;
    }
    default:
      const gchar* message_type =
          gst_message_type_get_name(GST_MESSAGE_TYPE(msg));
      spdlog::info("Received message: {}", message_type);
      break;
  }

  return terminate;
}
}  // namespace

VideoPipeline::VideoPipeline(const char* input, void* display, void* surface)
    : display(display), surface(surface) {
  pipeline = {gst_pipeline_new("VideoPipeline"), {}};

  auto src = Make("filesrc");
  g_object_set(src.get(), "location", input, NULL);

  auto parse = Make("parsebin");
  g_signal_connect(parse.get(), "pad-added", (GCallback)PadAdded,
                   pipeline.get());

  auto queue_video = Make("queue", "queuevideo");
  auto decode_video = Make("v4l2slh264dec");
  auto sink_video = Make("waylandsink");

  auto queue_audio = Make("queue", "queueaudio");
  auto decode_audio = Make("avdec_aac");
  auto convert_audio = Make("audioconvert");
  auto sink_audio = Make("pulsesink");

  std::vector<std::vector<GstElement*>> elements_to_link = {
      // demux
      {src.get(), parse.get()},
      // video pipe
      {queue_video.get(), decode_video.get(), sink_video.get()},
      // audio pipe
      {queue_audio.get(), decode_audio.get(), convert_audio.get(),
       sink_audio.get()}};

  auto elements = std::vector<std::reference_wrapper<GstElementPtr>>{
      src,         parse,        queue_video,   decode_video, sink_video,
      queue_audio, decode_audio, convert_audio, sink_audio};

  // transfer ownership of elements to GstPipeline
  for (auto& elem : elements) {
    gst_bin_add(GST_BIN(pipeline.get()), elem.get().release());
  }

  if (not LinkAll(elements_to_link)) {
    exit(1);
  }

  bus = {gst_pipeline_get_bus(GST_PIPELINE(pipeline.get())), {}};
  gst_bus_set_sync_handler(bus.get(), BusSyncHandler, this, NULL);
}

VideoPipeline::~VideoPipeline() {
  gst_element_set_state(pipeline.get(), GST_STATE_NULL);
}

void VideoPipeline::Play() {
  gst_element_set_state(pipeline.get(), GST_STATE_PLAYING);
}

bool VideoPipeline::ProcessMessages() {
  auto msg = GstMessagePtr{gst_bus_pop(bus.get()), &gst_message_unref};
  bool terminate = false;

  while (msg) {
    terminate |= ProcessMessage(msg.get());
    msg = GstMessagePtr{gst_bus_pop(bus.get()), &gst_message_unref};
  }

  return terminate;
}

void VideoPipeline::Resize(int width, int height) {
  gst_video_overlay_set_render_rectangle(overlay, 0, 0, width, height);
}

void VideoPipeline::Pause() {
  gst_element_set_state(pipeline.get(), GST_STATE_PAUSED);
}

}  // namespace player
