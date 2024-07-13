#include "SetPlaying.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <gst/gst.h>

enum class CodecType { H264, H265 };

void setupLocalCapturePipeline(GstElement *pipeline, GstElement *sink) {
  GstElement *src          = gst_element_factory_make("v4l2src", NULL);
  GstElement *videoconvert = gst_element_factory_make("videoconvert", NULL);
  GstElement *glupload     = gst_element_factory_make("glupload", NULL);

  gst_bin_add_many(GST_BIN(pipeline), src, videoconvert, glupload, sink, NULL);
  gst_element_link_many(src, videoconvert, glupload, sink, NULL);
}

void setupJpegReceivePipeline(GstElement *pipeline, GstElement *sink, gint port) {
  GstElement *src          = gst_element_factory_make("udpsrc", NULL);
  GstElement *rtpdepay     = gst_element_factory_make("rtpjpegdepay", NULL);
  GstElement *jpegparse    = gst_element_factory_make("jpegparse", NULL);
  GstElement *decoder      = gst_element_factory_make("jpegdec", NULL);
  GstElement *videoconvert = gst_element_factory_make("videoconvert", NULL);
  GstElement *glupload     = gst_element_factory_make("glupload", NULL);

  g_object_set(src, "port", port, NULL);
  GstCaps *caps = gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING, "video", "encoding-name", G_TYPE_STRING, "JPEG", "payload", G_TYPE_INT, 26, NULL);
  g_object_set(src, "caps", caps, NULL);
  gst_caps_unref(caps);

  gst_bin_add_many(GST_BIN(pipeline), src, rtpdepay, jpegparse, decoder, videoconvert, glupload, sink, NULL);
  gst_element_link_many(src, rtpdepay, jpegparse, decoder, videoconvert, glupload, sink, NULL);
}

void setupH26xReceivePipeline(GstElement *pipeline, GstElement *sink, gint port, CodecType codecType) {
  GstElement *src          = gst_element_factory_make("udpsrc", NULL);
  GstElement *rtpdepay     = nullptr;
  GstElement *parse        = nullptr;
  GstElement *decoder      = nullptr;
  GstElement *videoconvert = gst_element_factory_make("videoconvert", NULL);
  GstElement *glupload     = gst_element_factory_make("glupload", NULL);

  g_object_set(src, "port", port, NULL);

  if (codecType == CodecType::H264) {
    rtpdepay = gst_element_factory_make("rtph264depay", NULL);
    parse    = gst_element_factory_make("h264parse", NULL);
    decoder  = gst_element_factory_make("avdec_h264", NULL);
  } else if (codecType == CodecType::H265) {
    rtpdepay = gst_element_factory_make("rtph265depay", NULL);
    parse    = gst_element_factory_make("h265parse", NULL);
    decoder  = gst_element_factory_make("avdec_h265", NULL);
  } else {
    g_printerr("Unsupported codec type.\n");
    return;
  }

  GstCaps *caps = gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING, "video", "encoding-name", G_TYPE_STRING, (codecType == CodecType::H264) ? "H264" : "H265", "payload", G_TYPE_INT, 96, NULL);
  g_object_set(src, "caps", caps, NULL);
  gst_caps_unref(caps);

  gst_bin_add_many(GST_BIN(pipeline), src, rtpdepay, parse, decoder, videoconvert, glupload, sink, NULL);
  gst_element_link_many(src, rtpdepay, parse, decoder, videoconvert, glupload, sink, NULL);
}

int main(int argc, char *argv[]) {
  int ret;

  gst_init(&argc, &argv);

  {
    QGuiApplication app(argc, argv);

    GstElement *pipeline = gst_pipeline_new(NULL);
    GstElement *sink     = gst_element_factory_make("qmlglsink", NULL);

    gint port = 5000;
    // setupLocalCapturePipeline(pipeline, sink);
    setupH26xReceivePipeline(pipeline, sink, port, CodecType::H264);
    // setupH265ReceivePipeline(pipeline, sink, port);
    // setupJpegReceivePipeline(pipeline, sink, port);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    QQuickItem   *videoItem;
    QQuickWindow *rootObject;

    rootObject = static_cast<QQuickWindow *>(engine.rootObjects().first());
    videoItem  = rootObject->findChild<QQuickItem *>("videoItem");
    g_object_set(sink, "widget", videoItem, NULL);

    rootObject->scheduleRenderJob(new SetPlaying(pipeline), QQuickWindow::BeforeSynchronizingStage);

    ret = app.exec();

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
  }

  gst_deinit();

  return ret;
}
