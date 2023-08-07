#include "streamer.h"
#include <QDateTime>

Streamer::Streamer(QObject *parent)
    : QObject{parent}
    , frameInterval(5)
{}

bool Streamer::initStreaming(QHostAddress address, QString cameraDevice)
{
    GError *err = NULL;
    QString cmd = QString(
        "v4l2src device=%1 ! tee name=t0 ! queue max-size-buffers=1000 leaky=downstream ! jpegenc ! \
         appsink drop=true async=false max-buffers=30 name=appsink0 t0. ! queue max-size-buffers=1000 leaky=downstream ! \
         videoconvert ! video/x-raw,format=I420 ! \
         x264enc tune=zerolatency speed-preset=superfast key-int-max=12 byte-stream=true ! \
         tee name=t1 ! queue max-size-buffers=1000 leaky=downstream ! tcpserversink host=%2 port=%3 async=false t1. ! \
         queue max-size-buffers=1000 leaky=downstream ! mpegtsmux ! filesink async=false location=/dev/null name=filesink0"
    ).arg(cameraDevice, address.toString(), QString::number(PORT));

    m_pipeline = gst_parse_launch(cmd.toUtf8().constData(), &err);

    if (err) {
        qDebug() << "Failed to create pipeline:" << err->message;
        return false;
    }

    m_appsink = gst_bin_get_by_name(GST_BIN(m_pipeline), "appsink0");
    m_filesink = gst_bin_get_by_name(GST_BIN(m_pipeline), "filesink0");

    g_object_set (m_appsink, "emit-signals", TRUE, NULL);
    g_signal_connect (m_appsink, "new-sample", G_CALLBACK(Streamer::appsinkCallback), this);

    if (gst_element_set_state(m_pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        qDebug() << "Unable to set the pipeline to the playing state";
        GstBus *bus = gst_element_get_bus (m_pipeline);
        GstMessage *msg =
            gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

        if (msg != NULL) {
            GError *err;
            gchar *debug_info;

            switch (GST_MESSAGE_TYPE (msg)) {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error (msg, &err, &debug_info);
                g_printerr ("Error received from element %s: %s\n",
                           GST_OBJECT_NAME (msg->src), err->message);
                g_printerr ("Debugging information: %s\n",
                           debug_info ? debug_info : "none");
                g_clear_error (&err);
                g_free (debug_info);
                break;
            case GST_MESSAGE_EOS:
                g_print ("End-Of-Stream reached.\n");
                break;
            default:
                /* We should not reach here because we only asked for ERRORs and EOS */
                g_printerr ("Unexpected message received.\n");
                break;
            }
            gst_message_unref (msg);
        }

        gst_object_unref (bus);

        return false;
    }

    return true;
}

void Streamer::setRecording(bool value)
{
    if (m_isRecording == value)
        return;

    QString filename = value ? QString("REC_%1.mp4").arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)) : QString("/dev/null");

    gst_element_set_state(m_filesink, GST_STATE_NULL);
    g_object_set(m_filesink, "location", filename.toUtf8().constData(), NULL);
    gst_element_set_state(m_filesink, GST_STATE_PLAYING);

    m_isRecording = value;
    if (value) {
        m_recStartTime = QDateTime::currentMSecsSinceEpoch();
    }
}

bool Streamer::isRecording()
{
    return m_isRecording;
}

qint64 Streamer::getRecStartTime()
{
    return m_recStartTime;
}

GstFlowReturn Streamer::appsinkCallback(GstElement *appsink, Streamer *obj)
{
    if (++obj->m_frameCounter < obj->frameInterval.value())
        return GST_FLOW_OK;

    GstSample *sample;
    GstBuffer *buf;
    GstMemory *mem;
    GstMapInfo info;
    QImage frame;

    obj->m_frameCounter = 0;

    g_signal_emit_by_name (appsink, "pull-sample", &sample);
    if (!sample)
        return GST_FLOW_OK;

    buf = gst_sample_get_buffer(sample);
    mem = gst_buffer_peek_memory(buf, 0);
    gst_memory_ref(mem);
    gst_memory_map(mem, &info, GST_MAP_READ);

    frame = QImage::fromData(QByteArray((const char*)info.data, (qsizetype)info.size));

    gst_memory_unmap(mem, &info);
    gst_memory_unref(mem);
    gst_sample_unref (sample);

    emit obj->frameReady(frame);
    return GST_FLOW_OK;
}
