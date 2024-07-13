#include "SetPlaying.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <gst/gst.h>

int main(int argc, char *argv[]) {
  int ret;

  gst_init(&argc, &argv);

  {
    QGuiApplication app(argc, argv);

    GstElement *pipeline = gst_pipeline_new(NULL);
    GstElement *sink     = gst_element_factory_make("qmlglsink", NULL);

    gint port = 5000;
    // setupLocalCapturePipeline(pipeline, sink);
    // setupH264ReceivePipeline(pipeline, sink, port);
    setupH265ReceivePipeline(pipeline, sink, port);
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

void setupLocalCapturePipeline(GstElement *pipeline, GstElement *sink) {
  GstElement *src          = gst_element_factory_make("v4l2src", NULL);
  GstElement *videoconvert = gst_element_factory_make("videoconvert", NULL);
  GstElement *glupload     = gst_element_factory_make("glupload", NULL);
  GstElement *sink         = gst_element_factory_make("qmlglsink", NULL);

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

  g_object_set(src, "port", 5000, NULL);
  GstCaps *caps = gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING, "video", "encoding-name", G_TYPE_STRING, "JPEG", "payload", G_TYPE_INT, 26, NULL);
  g_object_set(src, "caps", caps, NULL);
  gst_caps_unref(caps);

  gst_bin_add_many(GST_BIN(pipeline), src, rtpdepay, jpegparse, decoder, videoconvert, glupload, sink, NULL);
  gst_element_link_many(src, rtpdepay, jpegparse, decoder, videoconvert, glupload, sink, NULL);
}

void setupH264ReceivePipeline(GstElement *pipeline, GstElement *sink, gint port) {
  GstElement *src          = gst_element_factory_make("udpsrc", NULL);
  GstElement *rtpdepay     = gst_element_factory_make("rtph264depay", NULL);
  GstElement *h264parse    = gst_element_factory_make("h264parse", NULL);
  GstElement *decoder      = gst_element_factory_make("avdec_h264", NULL);
  GstElement *videoconvert = gst_element_factory_make("videoconvert", NULL);
  GstElement *glupload     = gst_element_factory_make("glupload", NULL);

  g_object_set(src, "port", 5000, NULL);
  GstCaps *caps = gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING, "video", "encoding-name", G_TYPE_STRING, "H264", "payload", G_TYPE_INT, 96, NULL);
  g_object_set(src, "caps", caps, NULL);
  gst_caps_unref(caps);

  gst_bin_add_many(GST_BIN(pipeline), src, rtpdepay, h264parse, decoder, videoconvert, glupload, sink, NULL);
  gst_element_link_many(src, rtpdepay, h264parse, decoder, videoconvert, glupload, sink, NULL);
}

void setupH265ReceivePipeline(GstElement *pipeline, GstElement *sink, gint port) {
  GstElement *src          = gst_element_factory_make("udpsrc", NULL);
  GstElement *rtpdepay     = gst_element_factory_make("rtph265depay", NULL);
  GstElement *h265parse    = gst_element_factory_make("h265parse", NULL);
  GstElement *decoder      = gst_element_factory_make("avdec_h265", NULL);
  GstElement *videoconvert = gst_element_factory_make("videoconvert", NULL);
  GstElement *glupload     = gst_element_factory_make("glupload", NULL);

  g_object_set(src, "port", 5000, NULL);
  GstCaps *caps = gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING, "video", "encoding-name", G_TYPE_STRING, "H265", "payload", G_TYPE_INT, 96, NULL);
  g_object_set(src, "caps", caps, NULL);
  gst_caps_unref(caps);

  gst_bin_add_many(GST_BIN(pipeline), src, rtpdepay, h265parse, decoder, videoconvert, glupload, sink, NULL);
  gst_element_link_many(src, rtpdepay, h265parse, decoder, videoconvert, glupload, sink, NULL);
}
