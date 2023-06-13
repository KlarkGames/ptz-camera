#include "processor.h"

Processor::Processor(QObject *parent)
    : QObject{parent}
{
    this->m_server = new Server();
    this->m_mountDriver = new MountDriver();
    connect(m_server, &Server::rotateCmdRecieved, m_mountDriver, &MountDriver::rotate);
}

QVideoSink *Processor::videoSink() const
{
    return m_videoSink.get();
}

MountDriver *Processor::mountDriver() const
{
    return m_mountDriver.get();
}

void Processor::setVideoSink(QVideoSink *newVideoSink)
{
    if (m_videoSink == newVideoSink)
        return;
    m_videoSink = newVideoSink;
    connect(newVideoSink, &QVideoSink::videoFrameChanged, this, &Processor::hvideoFrameChanged);
    emit videoSinkChanged();
}

void Processor::rotateMount(QVariantMap paramsMap)
{
    this->m_mountDriver->rotate(paramsMap);
}

void Processor::hvideoFrameChanged(const QVideoFrame &frame) {
    // qDebug("Frame size: %i", frame.startTime());
}
