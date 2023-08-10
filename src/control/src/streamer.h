#ifndef STREAMER_H
#define STREAMER_H

#include <QObject>
#include <QHostAddress>
#include <QProperty>
#include <QImage>
#include <QDateTime>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/gstbuffer.h>

class Streamer : public QObject
{
    Q_OBJECT
public:
    static const int PORT = 5000;
    explicit Streamer(QObject *parent = nullptr);

    bool initStreaming(QHostAddress address, QString cameraDevice);
    void setCameraDevice(QString cameraDevice);
    void setRecording(bool value);
    bool isRecording();
    qint64 getRecStartTime();
    QProperty<int> frameInterval;

signals:
    void frameReady(QImage frame);
    void cameraDeviceChanged(QString cameraDevice);

private:
    int m_frameCounter = 0;
    bool m_isRecording = false;
    qint64 m_recStartTime;
    GstElement *m_pipeline = nullptr, *m_appsink = nullptr, *m_filesink = nullptr, *m_src = nullptr;
    static GstFlowReturn appsinkCallback(GstElement *appsink, Streamer *obj);
};

#endif // STREAMER_H
