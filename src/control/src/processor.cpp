#include "processor.h"

Processor::Processor(QObject *parent)
    : QObject{parent}
{
    this->m_server = new Server();
    this->m_mountDriver = new MountDriver();
    connect(m_server, &Server::rotateCmdRecieved, m_mountDriver, &MountDriver::rotate);
    

    vertical_border = 0.7;
    horizontal_border = 0.7;

    m_mountDriver = new MountDriver();
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

    connect(
        this,
        &Processor::handleFrameRequest,
        this,
        &Processor::handleFrameWithNN,
        Qt::QueuedConnection
    );

    m_timer.start();
}

QVideoSink *Processor::videoSink() const
{
    return m_videoSink.get();
}

MountDriver *Processor::mountDriver() const
{
    return m_mountDriver.get();
}

CameraWrapper *Processor::getCameraWrapper() const
{
    return m_cameraWrapper.get();
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

void Processor::setCameraWrapper(CameraWrapper *newCameraWrapper)
{
    if (m_cameraWrapper == newCameraWrapper)
        return;
    m_cameraWrapper = newCameraWrapper;
    emit cameraWrapperChanged();
}

void Processor::hvideoFrameChanged(const QVideoFrame &frame) {

    if (m_counter++ == m_max_count) {
        m_counter = 0;
        emit handleFrameRequest(frame);
    }
}

void Processor::handleFrameWithNN(const QVideoFrame &frame)
{
    int plane = 0;
    QImage::Format image_format = QVideoFrameFormat::imageFormatFromPixelFormat(frame.pixelFormat());

    auto m_tmpVideoFrame = m_videoSink->videoFrame();

    auto handleType = m_tmpVideoFrame.handleType();

    if (frame.isValid()) {
        if (handleType == QVideoFrame::NoHandle) {

            QImage img = m_tmpVideoFrame.toImage();

            cv::Mat input(img.height(), img.width(), CV_8UC4, img.bits());
            cv::cvtColor(input, input, cv::COLOR_BGRA2RGB);

            m_deepSort.forward(input);

            QPainter painter(&img);

//            foreach (Detection detection, detections) {
//                cv::Rect2i bbox = detection.box;
//                painter.drawRect(bbox.x, bbox.y, bbox.width, bbox.height);
//                cv::rectangle(input, cv::Rect_<int>(bbox.x, bbox.y, bbox.width, bbox.height), cv::Scalar(225, 225, 0), 10);
//            }

            cv::imshow("Test window", input);

            painter.end();
        }
    }
}

void Processor::moveCamera() {
    this->cameraDirections = this->getDirections();
    if (cameraDirections.first != Direction::hold || cameraDirections.second != Direction::hold)
        emit this->moveCameraRequest(cameraDirections);
}

QPair<Direction, Direction> Processor::getDirections() {
    cv::Rect2i bbox = m_bbox;
    QSize frame_size = m_videoSink->videoSize();
    QRect border(
        int(frame_size.width() / 2 - frame_size.width() * horizontal_border / 2),
        int(frame_size.height() / 2 - frame_size.height() * vertical_border / 2),
        int(frame_size.width() * horizontal_border),
        int(frame_size.height() * vertical_border)
        );

    QPoint center(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2);

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
