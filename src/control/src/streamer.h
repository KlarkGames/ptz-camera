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

#include "settingsconductor.h"

class Streamer : public QObject, public SettingsConductor
{
    Q_OBJECT
public:
    explicit Streamer(QObject *parent = nullptr);

    void setSettings(QJsonObject params) override;
    void setCameraDevice(QString cameraDevice);
    void setRecording(bool value);

    QJsonObject getSettings() override;
    qint64 getRecStartTime();
    QString getCameraDevice();

    bool initStreaming(QHostAddress address, QString cameraDevice);
    bool isRecording();

    QProperty<int> frameInterval;
    static const int PORT = 5000;

signals:
    void frameReady(QImage frame);
    void cameraDeviceChanged(QString cameraDevice);

private:
    int m_frameCounter = 0;
    bool m_isRecording = false;
    qint64 m_recStartTime;
    QString m_currentCamera;

    GstElement *m_pipeline = nullptr;
    GstElement *m_appsink = nullptr;
    GstElement *m_filesink = nullptr;
    GstElement *m_src = nullptr;

    static GstFlowReturn appsinkCallback(GstElement *appsink, Streamer *obj);
};

#endif // STREAMER_H
