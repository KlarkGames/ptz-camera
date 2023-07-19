#include "processor.h"

Processor::Processor(QObject *parent)
    : QObject{parent}
{
    this->m_server = new Server();
    this->m_mountDriver = new MountDriver();
    this->m_streamer = new Streamer();

    connect(m_server, &Server::rotateCmdReceived, m_mountDriver, &MountDriver::rotate);
    connect(m_server, &Server::setRecordingCmdReceived, m_streamer, &Streamer::setRecording);
    connect(m_streamer, &Streamer::recordingStatusChanged, m_server, &Server::setRecordingStatus);
    connect(m_server, &Server::setTrackingCmdReceived, this, &Processor::setTracking);
    connect(this, &Processor::trackingStatusChanged, m_server, &Server::setTrackingStatus);

    m_streamer->initStreaming(m_server->address(), "/dev/video0");
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

void Processor::handleFrameWithNN(QImage frame)
{

}

void Processor::setTracking(bool value)
{
    if (m_isTracking == value)
        return;

    if (value)
        QObject::connect(m_streamer, &Streamer::frameReady, this, &Processor::handleFrameWithNN);
    else
        QObject::disconnect(m_streamer, &Streamer::frameReady, this, &Processor::handleFrameWithNN);

    m_isTracking = value;
    emit trackingStatusChanged(value);
}
