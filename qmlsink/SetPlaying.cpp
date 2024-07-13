#include "SetPlaying.h"

SetPlaying::SetPlaying(GstElement *pipeline) { this->m_pipeline = pipeline ? static_cast<GstElement *>(gst_object_ref(pipeline)) : NULL; }

SetPlaying::~SetPlaying() {
  if (this->m_pipeline)
    gst_object_unref(this->m_pipeline);
}

void SetPlaying::run() {
  if (this->m_pipeline)
    gst_element_set_state(this->m_pipeline, GST_STATE_PLAYING);
}
