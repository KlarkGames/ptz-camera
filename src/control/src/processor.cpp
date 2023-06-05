#include "processor.h"

Processor::Processor(QObject *parent)
    : QObject{parent}
{
}

QVideoSink *Processor::videoSink() const
{
    return m_videoSink.get();
}

void Processor::setVideoSink(QVideoSink *newVideoSink)
{
    if (m_videoSink == newVideoSink)
        return;
    m_videoSink = newVideoSink;
    connect(newVideoSink, &QVideoSink::videoFrameChanged, this, &Processor::hvideoFrameChanged);
    emit videoSinkChanged();
}

void Processor::hvideoFrameChanged(const QVideoFrame &frame) {
    // qDebug("Frame size: %i", frame.startTime());
}
