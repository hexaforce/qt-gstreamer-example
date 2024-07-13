#ifndef SETPLAYING_H
#define SETPLAYING_H

#include <QRunnable>
#include <gst/gst.h>

class SetPlaying : public QRunnable {
public:
  SetPlaying(GstElement *pipeline);
  ~SetPlaying();

  void run() override;

private:
  GstElement *m_pipeline;
};

#endif // SETPLAYING_H
