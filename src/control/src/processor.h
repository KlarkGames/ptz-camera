#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QObject>
#include <QPointer>
#include <QVideoSink>
#include <QQmlEngine>
#include <QTimer>
#include <QImage>
#include <QPainter>
#include <QSize>
#include <QVideoFrame>
#include <QRandomGenerator>
#include <QDateTime>

#include "server.h"
#include "mountdriver.h"

class Processor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVideoSink* processorVideoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
    Q_PROPERTY(MountDriver* mountDriver READ mountDriver NOTIFY mountDriverChanged)
    QML_ELEMENT

    public:
        explicit Processor(QObject *parent = nullptr);
        QVideoSink *videoSink() const;
        MountDriver *mountDriver() const;
        void setVideoSink(QVideoSink *newVideoSink);
        Q_INVOKABLE void rotateMount(QVariantMap);

    signals:
        void videoSinkChanged();
        void mountDriverChanged();

    public slots:
        void hvideoFrameChanged(const QVideoFrame &frame);

    private:
        QPointer<QVideoSink> m_videoSink;
        QPointer<MountDriver> m_mountDriver;
        QPointer<Server> m_server;
};

#endif // PROCESSOR_H
