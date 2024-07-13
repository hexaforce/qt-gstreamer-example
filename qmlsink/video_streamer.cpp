// g++ -o video_streamer video_streamer.cpp `pkg-config --cflags --libs gstreamer-1.0`

// if (argc != 4) {
//   g_printerr("Usage: %s <device> <host> <port>\n", argv[0]);
//   return -1;
// }

// const char *device = argv[1];
// const char *host   = argv[2];
// int         port   = atoi(argv[3]);

#include <gst/gst.h>

enum class CodecType { H264, H265 };

// gst-launch-1.0 -e v4l2src device=/dev/video0 ! 'image/jpeg,width=1920,height=1080,framerate=60/1' ! jpegparse ! rtpjpegpay ! udpsink host=127.0.0.1 port=5000
GstStateChangeReturn setupJpegTransmitPipeline(GstElement *pipeline, const char *device, const char *host, int port) {
  GstElement *source    = gst_element_factory_make("v4l2src", NULL);
  GstElement *filter    = gst_element_factory_make("capsfilter", NULL);
  GstElement *parser    = gst_element_factory_make("jpegparse", NULL);
  GstElement *payloader = gst_element_factory_make("rtpjpegpay", NULL);
  GstElement *sink      = gst_element_factory_make("udpsink", NULL);

  g_object_set(source, "device", device, NULL);

  GstCaps *caps = gst_caps_new_simple("image/jpeg", "width", G_TYPE_INT, 1920, "height", G_TYPE_INT, 1080, "framerate", GST_TYPE_FRACTION, 60, 1, NULL);
  g_object_set(filter, "caps", caps, NULL);
  gst_caps_unref(caps);

  g_object_set(sink, "host", host, "port", port, NULL);

  gst_bin_add_many(GST_BIN(pipeline), source, filter, parser, payloader, sink, NULL);
  gst_element_link_many(source, filter, parser, payloader, sink, NULL);

  GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
  return ret;
}

// gst-launch-1.0 -e v4l2src device=/dev/video0 ! 'video/x-h264,width=1920,height=1080,framerate=30/1' ! h264parse ! rtph264pay ! udpsink host=127.0.0.1 port=5000
// gst-launch-1.0 -e v4l2src device=/dev/video0 ! 'video/x-h265,width=1920,height=1080,framerate=30/1' ! h265parse ! rtph265pay ! udpsink host=127.0.0.1 port=5000
GstStateChangeReturn setupH26xTransmitPipeline(GstElement *pipeline, const char *device, const char *host, int port, CodecType codecType) {
    GstElement *source    = gst_element_factory_make("v4l2src", NULL);
    GstElement *filter    = gst_element_factory_make("capsfilter", NULL);
    GstElement *parser    = nullptr;
    GstElement *payloader = nullptr;
    GstElement *sink      = gst_element_factory_make("udpsink", NULL);

    g_object_set(source, "device", device, NULL);

    GstCaps *caps = nullptr;
    if (codecType == CodecType::H264) {
        caps = gst_caps_new_simple("video/x-h264", "width", G_TYPE_INT, 1920, "height", G_TYPE_INT, 1080, "framerate", GST_TYPE_FRACTION, 30, 1, NULL);
        parser = gst_element_factory_make("h264parse", NULL);
        payloader = gst_element_factory_make("rtph264pay", NULL);
    } else if (codecType == CodecType::H265) {
        caps = gst_caps_new_simple("video/x-h265", "width", G_TYPE_INT, 1920, "height", G_TYPE_INT, 1080, "framerate", GST_TYPE_FRACTION, 30, 1, NULL);
        parser = gst_element_factory_make("h265parse", NULL);
        payloader = gst_element_factory_make("rtph265pay", NULL);
    }

    if (!caps || !parser || !payloader) {
        g_printerr("Failed to create caps, parser, or payloader element.\n");
        return GST_STATE_CHANGE_FAILURE;
    }

    g_object_set(filter, "caps", caps, NULL);
    gst_caps_unref(caps);

    g_object_set(sink, "host", host, "port", port, NULL);

    gst_bin_add_many(GST_BIN(pipeline), source, filter, parser, payloader, sink, NULL);
    gst_element_link_many(source, filter, parser, payloader, sink, NULL);

    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    return ret;
}

// gst-launch-1.0 -e v4l2src device=/dev/video0 ! 'video/x-raw,format=YUY2,width=1920,height=1080,framerate=30/1' ! videoconvert ! x264enc ! h264parse ! rtph264pay ! udpsink host=127.0.0.1 port=5000
// gst-launch-1.0 -e v4l2src device=/dev/video0 ! 'video/x-raw,format=YUY2,width=1920,height=1080,framerate=30/1' ! videoconvert ! x265enc ! h265parse ! rtph265pay ! udpsink host=127.0.0.1 port=5000
GstStateChangeReturn setupH26xTransmitPipeline_RawToH26x_SoftwareEncoding(GstElement *pipeline, const char *device, const char *host, int port, CodecType codecType) {
  GstElement *source       = gst_element_factory_make("v4l2src", NULL);
  GstElement *filter       = gst_element_factory_make("capsfilter", NULL);
  GstElement *videoconvert = gst_element_factory_make("videoconvert", NULL);
  GstElement *encoder      = nullptr;
  GstElement *parser       = nullptr;
  GstElement *payloader    = nullptr;
  GstElement *sink         = gst_element_factory_make("udpsink", NULL);

  g_object_set(source, "device", device, NULL);

  GstCaps *caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "YUY2", "width", G_TYPE_INT, 1920, "height", G_TYPE_INT, 1080, "framerate", GST_TYPE_FRACTION, 30, 1, NULL);
  g_object_set(filter, "caps", caps, NULL);
  gst_caps_unref(caps);

  g_object_set(sink, "host", host, "port", port, NULL);

  if (codecType == CodecType::H264) {
    encoder   = gst_element_factory_make("x264enc", NULL);
    parser    = gst_element_factory_make("h264parse", NULL);
    payloader = gst_element_factory_make("rtph264pay", NULL);
  } else if (codecType == CodecType::H265) {
    encoder   = gst_element_factory_make("x265enc", NULL);
    parser    = gst_element_factory_make("h265parse", NULL);
    payloader = gst_element_factory_make("rtph265pay", NULL);
  }

  if (!encoder || !parser || !payloader) {
    g_printerr("Failed to create encoder, parser, or payloader element.\n");
    return GST_STATE_CHANGE_FAILURE;
  }

  gst_bin_add_many(GST_BIN(pipeline), source, filter, videoconvert, encoder, parser, payloader, sink, NULL);
  gst_element_link_many(source, filter, videoconvert, encoder, parser, payloader, sink, NULL);

  GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
  return ret;
}

// gst-launch-1.0 -e v4l2src device=/dev/video0 ! 'image/jpeg,width=1920,height=1080,framerate=60/1' ! jpegparse ! jpegdec ! videoconvert ! x264enc ! h264parse ! rtph264pay ! udpsink host=127.0.0.1 port=5000
// gst-launch-1.0 -e v4l2src device=/dev/video0 ! 'image/jpeg,width=1920,height=1080,framerate=60/1' ! jpegparse ! jpegdec ! videoconvert ! x265enc ! h265parse ! rtph265pay ! udpsink host=127.0.0.1 port=5000
GstStateChangeReturn setupH26xTransmitPipeline_JpegToH26x_SoftwareEncoding(GstElement *pipeline, const char *device, const char *host, int port, CodecType codecType) {
  GstElement *source       = gst_element_factory_make("v4l2src", NULL);
  GstElement *filter       = gst_element_factory_make("capsfilter", NULL);
  GstElement *jpegparse    = gst_element_factory_make("jpegparse", NULL);
  GstElement *jpegdec      = gst_element_factory_make("jpegdec", NULL);
  GstElement *videoconvert = gst_element_factory_make("videoconvert", NULL);
  GstElement *encoder      = nullptr;
  GstElement *parser       = nullptr;
  GstElement *payloader    = nullptr;
  GstElement *sink         = gst_element_factory_make("udpsink", NULL);

  g_object_set(source, "device", device, NULL);

  GstCaps *caps = gst_caps_new_simple("image/jpeg", "width", G_TYPE_INT, 1920, "height", G_TYPE_INT, 1080, "framerate", GST_TYPE_FRACTION, 60, 1, NULL);
  g_object_set(filter, "caps", caps, NULL);
  gst_caps_unref(caps);

  g_object_set(sink, "host", host, "port", port, NULL);

  if (codecType == CodecType::H264) {
    encoder   = gst_element_factory_make("x264enc", NULL);
    parser    = gst_element_factory_make("h264parse", NULL);
    payloader = gst_element_factory_make("rtph264pay", NULL);
  } else if (codecType == CodecType::H265) {
    encoder   = gst_element_factory_make("x265enc", NULL);
    parser    = gst_element_factory_make("h265parse", NULL);
    payloader = gst_element_factory_make("rtph265pay", NULL);
  }

  if (!encoder || !parser || !payloader) {
    g_printerr("Failed to create encoder, parser, or payloader element.\n");
    return GST_STATE_CHANGE_FAILURE;
  }

  gst_bin_add_many(GST_BIN(pipeline), source, filter, jpegparse, jpegdec, videoconvert, encoder, parser, payloader, sink, NULL);
  gst_element_link_many(source, filter, jpegparse, jpegdec, videoconvert, encoder, parser, payloader, sink, NULL);

  GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
  return ret;
}


// gst-launch-1.0 -e v4l2src device=/dev/video0 ! 'video/x-raw,format=YUY2,width=1920,height=1080,framerate=30/1' ! videoconvert ! 'video/x-raw,format=NV12' ! nvvideoconvert ! 'video/x-raw(memory:NVMM),format=NV12' ! nvv4l2h264enc ! h264parse ! rtph264pay ! udpsink host=127.0.0.1 port=5000
// gst-launch-1.0 -e v4l2src device=/dev/video0 ! 'video/x-raw,format=YUY2,width=1920,height=1080,framerate=30/1' ! videoconvert ! 'video/x-raw,format=NV12' ! nvvideoconvert ! 'video/x-raw(memory:NVMM),format=NV12' ! nvv4l2h265enc ! h265parse ! rtph265pay ! udpsink host=127.0.0.1 port=5000
GstStateChangeReturn setupH26xTransmitPipeline_RawToH26x_HardwareEncoding(GstElement *pipeline, const char *device, const char *host, int port, CodecType codecType) {
    GstElement *source         = gst_element_factory_make("v4l2src", NULL);
    GstElement *filter         = gst_element_factory_make("capsfilter", NULL);
    GstElement *videoconvert   = gst_element_factory_make("videoconvert", NULL);
    GstElement *nvvideoconvert = gst_element_factory_make("nvvideoconvert", NULL);
    GstElement *encoder        = nullptr;
    GstElement *parser         = nullptr;
    GstElement *payloader      = nullptr;
    GstElement *sink           = gst_element_factory_make("udpsink", NULL);

    g_object_set(source, "device", device, NULL);

    GstCaps *caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "YUY2", "width", G_TYPE_INT, 1920, "height", G_TYPE_INT, 1080, "framerate", GST_TYPE_FRACTION, 30, 1, NULL);
    g_object_set(filter, "caps", caps, NULL);
    gst_caps_unref(caps);

    g_object_set(sink, "host", host, "port", port, NULL);

    if (codecType == CodecType::H264) {
        encoder = gst_element_factory_make("nvv4l2h264enc", NULL);
        parser = gst_element_factory_make("h264parse", NULL);
        payloader = gst_element_factory_make("rtph264pay", NULL);
    } else if (codecType == CodecType::H265) {
        encoder = gst_element_factory_make("nvv4l2h265enc", NULL);
        parser = gst_element_factory_make("h265parse", NULL);
        payloader = gst_element_factory_make("rtph265pay", NULL);
    }

    if (!encoder || !parser || !payloader) {
        g_printerr("Failed to create encoder, parser, or payloader element.\n");
        return GST_STATE_CHANGE_FAILURE;
    }

    gst_bin_add_many(GST_BIN(pipeline), source, filter, videoconvert, nvvideoconvert, encoder, parser, payloader, sink, NULL);
    gst_element_link_many(source, filter, videoconvert, nvvideoconvert, encoder, parser, payloader, sink, NULL);

    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    return ret;
}

// gst-launch-1.0 -e v4l2src device=/dev/video0 ! 'image/jpeg,width=1920,height=1080,framerate=60/1' ! jpegparse ! jpegdec ! videoconvert ! 'video/x-raw,format=NV12' ! nvvideoconvert ! 'video/x-raw(memory:NVMM),format=NV12' ! nvv4l2h264enc ! h264parse ! rtph264pay ! udpsink host=127.0.0.1 port=5000
// gst-launch-1.0 -e v4l2src device=/dev/video0 ! 'image/jpeg,width=1920,height=1080,framerate=60/1' ! jpegparse ! jpegdec ! videoconvert ! 'video/x-raw,format=NV12' ! nvvideoconvert ! 'video/x-raw(memory:NVMM),format=NV12' ! nvv4l2h265enc ! h265parse ! rtph265pay ! udpsink host=127.0.0.1 port=5000
GstStateChangeReturn setupH26xTransmitPipeline_JpegToH26x_HardwareEncoding(GstElement *pipeline, const char *device, const char *host, int port, CodecType codecType) {
    GstElement *source          = gst_element_factory_make("v4l2src", NULL);
    GstElement *filter          = gst_element_factory_make("capsfilter", NULL);
    GstElement *jpegparser      = gst_element_factory_make("jpegparse", NULL);
    GstElement *jpegdec         = gst_element_factory_make("jpegdec", NULL);
    GstElement *videoconvert1   = gst_element_factory_make("videoconvert", NULL);
    GstElement *nvvideoconvert1 = gst_element_factory_make("nvvideoconvert", NULL);
    GstElement *nvvideoconvert2 = gst_element_factory_make("nvvideoconvert", NULL);
    GstElement *encoder         = NULL;
    GstElement *parser          = NULL;
    GstElement *payloader       = NULL;
    GstElement *sink           = gst_element_factory_make("udpsink", NULL);

    g_object_set(source, "device", device, NULL);

    GstCaps *caps = gst_caps_new_simple("image/jpeg", "width", G_TYPE_INT, 1920, "height", G_TYPE_INT, 1080, "framerate", GST_TYPE_FRACTION, 60, 1, NULL);
    g_object_set(filter, "caps", caps, NULL);
    gst_caps_unref(caps);

    g_object_set(sink, "host", host, "port", port, NULL);

    if (codecType == CodecType::H264) {
        encoder = gst_element_factory_make("nvv4l2h264enc", NULL);
        parser  = gst_element_factory_make("h264parse", NULL);
        payloader = gst_element_factory_make("rtph264pay", NULL);
    } else if (codecType == CodecType::H265) {
        encoder = gst_element_factory_make("nvv4l2h265enc", NULL);
        parser  = gst_element_factory_make("h265parse", NULL);
        payloader = gst_element_factory_make("rtph265pay", NULL);
    } else {
        g_printerr("Unsupported codec type.\n");
        return GST_STATE_CHANGE_FAILURE;
    }

    gst_bin_add_many(GST_BIN(pipeline), source, filter, jpegparser, jpegdec, videoconvert1, nvvideoconvert1, nvvideoconvert2, encoder, parser, payloader, sink, NULL);
    gst_element_link_many(source, filter, jpegparser, jpegdec, videoconvert1, nvvideoconvert1, nvvideoconvert2, encoder, parser, payloader, sink, NULL);

    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    return ret;
}

int main(int argc, char *argv[]) {

  gst_init(&argc, &argv);

  {
    const char *device = "/dev/video0";
    const char *host   = "127.0.0.1"; // destinationAddress
    int         port   = 5000;        // videoPort

    // Create elements
    GstElement *pipeline = gst_pipeline_new(NULL);

    // GstStateChangeReturn ret = setupJPEGTransmitPipeline(pipeline, device, host, port);
    GstStateChangeReturn ret = setupH26xTransmitPipeline(pipeline, device, host, port, CodecType::H264);
    // GstStateChangeReturn ret_h264 = setupH26xTransmitPipeline_JpegToH26x_SoftwareEncoding(pipeline, device, host, port, CodecType::H264);
    // GstStateChangeReturn ret_h264 = setupH26xTransmitPipeline_RawToH26x_SoftwareEncoding(pipeline, device, host, port, CodecType::H264);

    // GstStateChangeReturn ret = setupH265TransmitPipeline(pipeline, device, host, port);
    if (ret == GST_STATE_CHANGE_FAILURE) {
      g_printerr("Failed to start pipeline.\n");
      return -1;
    }

    // Wait until error or EOS
    GstBus     *bus = gst_element_get_bus(pipeline);
    GstMessage *msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_EOS);

    // Parse message
    if (msg != NULL) {
      GError *err;
      gchar  *debug_info;
      switch (GST_MESSAGE_TYPE(msg)) {
      case GST_MESSAGE_ERROR:
        gst_message_parse_error(msg, &err, &debug_info);
        g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
        g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
        g_clear_error(&err);
        g_free(debug_info);
        break;
      case GST_MESSAGE_EOS:
        g_print("End-Of-Stream reached.\n");
        break;
      default:
        // Should not be reached
        g_printerr("Unexpected message received.\n");
        break;
      }
      gst_message_unref(msg);
    }

    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
  }

  return 0;
}
