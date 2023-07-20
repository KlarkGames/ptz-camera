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

    vertical_border = 0.7;
    horizontal_border = 0.7;

    m_cameraWrapper = new CameraWrapper();

    m_timer.setInterval(1000);

    m_counter = 0;
    m_max_count = 5;

    connect(
        this,
        &Processor::moveCameraRequest,
        m_mountDriver,
        &MountDriver::handleNeuralNetRequest
    );

    connect(
        &m_timer,
        &QTimer::timeout,
        this,
        &Processor::moveCamera
    );

    m_timer.start();
}

MountDriver *Processor::mountDriver() const
{
    return m_mountDriver.get();
}

CameraWrapper *Processor::getCameraWrapper() const
{
    return m_cameraWrapper.get();
}

void Processor::rotateMount(QJsonObject params)
{
    this->m_mountDriver->rotate(params);
}

void Processor::setCameraWrapper(CameraWrapper *newCameraWrapper)
{
    if (m_cameraWrapper == newCameraWrapper)
        return;
    m_cameraWrapper = newCameraWrapper;
    emit cameraWrapperChanged();
}

void Processor::handleFrameWithNN(QImage frame)
{
    m_videoSize = frame.size();
    cv::Mat input(m_videoSize.height(), m_videoSize.width(), CV_8UC4, frame.bits());
    cv::cvtColor(input, input, cv::COLOR_BGRA2RGB);

    std::vector<ObjectInfo> objects = m_deepSort->forward(input);
    emit handleObjectsRequest(objects);
}

void Processor::moveCamera() {
    if (cameraDirections.first != Direction::hold || cameraDirections.second != Direction::hold)
        emit this->moveCameraRequest(cameraDirections);
}

QPair<Direction, Direction> Processor::getDirections(QRect bbox) {
    //QSize frame_size = m_videoSink->videoSize();
    QRect border(
        int(m_videoSize.width() / 2 - m_videoSize.width() * horizontal_border / 2),
        int(m_videoSize.height() / 2 - m_videoSize.height() * vertical_border / 2),
        int(m_videoSize.width() * horizontal_border),
        int(m_videoSize.height() * vertical_border)
    );

    QPoint center = bbox.center();

    Direction horizontal_direction;
    Direction vertical_direction;

    if (center.x() <= border.right() && center.x() >= border.left())
        horizontal_direction = Direction::hold;
    else if (center.x() < border.left())
        horizontal_direction = Direction::left;
    else if (center.x() > border.right())
        horizontal_direction = Direction::right;

    if (center.y() >= border.top() && center.y() <= border.bottom())
        vertical_direction = Direction::hold;
    else if (center.y() > border.bottom())
        vertical_direction = Direction::bottom;
    else if (center.y() < border.top())
        vertical_direction = Direction::top;

    return QPair<Direction, Direction>(horizontal_direction, vertical_direction);
}

void Processor::setTracking(bool value)
{
    if (m_isTracking == value)
        return;

    if (value) {
        if (m_deepSort.isNull()) {
            try {
                m_deepSort = QSharedPointer<DeepSORT>::create();
            } catch (...) {
                qDebug() << "Failed to initialize DeepSORT";
                m_deepSort.clear();
                return;
            }
        }

        QObject::connect(m_streamer, &Streamer::frameReady, this, &Processor::handleFrameWithNN);
    } else {
        QObject::disconnect(m_streamer, &Streamer::frameReady, this, &Processor::handleFrameWithNN);
    }

    m_isTracking = value;
    emit trackingStatusChanged(value);
}
