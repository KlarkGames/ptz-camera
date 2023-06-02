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

class Processor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVideoSink* processorVideoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
    QML_ELEMENT

    public:
        explicit Processor(QObject *parent = nullptr);
        QVideoSink *videoSink() const;
        void setVideoSink(QVideoSink *newVideoSink);

    signals:
        void videoSinkChanged();

    public slots:
        void hvideoFrameChanged(const QVideoFrame &frame);

    private:
        QPointer<QVideoSink> m_videoSink;
};

#endif // PROCESSOR_H
