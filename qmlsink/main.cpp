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

    GstElement *pipeline     = gst_pipeline_new(NULL);
    GstElement *src          = gst_element_factory_make("v4l2src", NULL);
    GstElement *videoconvert = gst_element_factory_make("videoconvert", NULL);
    GstElement *glupload     = gst_element_factory_make("glupload", NULL);
    GstElement *sink         = gst_element_factory_make("qmlglsink", NULL);

    gst_bin_add_many(GST_BIN(pipeline), src, videoconvert, glupload, sink, NULL);
    gst_element_link_many(src, videoconvert, glupload, sink, NULL);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    QQuickItem   *videoItem;
    QQuickWindow *rootObject;

    /* find and set the videoItem on the sink */
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
