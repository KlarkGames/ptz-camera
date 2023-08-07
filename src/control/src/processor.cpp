#include "processor.h"


Processor::Processor(QObject *parent)
    : QObject{parent}
{
    this->m_server = new Server();
    this->m_mountDriver = new MountDriver();
    this->m_streamer = new Streamer();

    connect(m_server, &Server::rotateCmdReceived, m_mountDriver, &MountDriver::rotate);
    connect(m_server, &Server::setSettingRecieved, this, &Processor::setSettings);
    connect(m_server, &Server::getSettingsRequest, this, [=](){m_server->updClientSettings(getSettings());});

    m_streamer->initStreaming(m_server->address(), "/dev/video0");

    m_verticalBorder = 0.7;
    m_horizontalBorder = 0.7;

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

Processor::~Processor()
{
    QJsonObject params = getSettings();
    params["tracking"] = false;
    params["recording"] = false;
    m_server->updClientSettings(params);
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

void Processor::setSettings(QJsonObject params)
{
    if (params.keys().contains("tracking")) {
        setTracking(params["tracking"].toBool());
    }
    if (params.keys().contains("recording")) {
        m_streamer->setRecording(params["recording"].toBool());
    }
    if (params.keys().contains("horizontalBorder")) {
        m_horizontalBorder = params["horizontalBorder"].toDouble();
    }
    if (params.keys().contains("verticalBorder")) {
        m_verticalBorder = params["verticalBorder"].toDouble();
    }

    if (params.keys().contains("targetId")) {
        m_targetingId = params["targetId"].toInt();
    }
    m_server->updClientSettings(getSettings());
}

QJsonObject Processor::getSettings()
{
    QJsonObject params;

    params["tracking"] = m_isTracking;
    bool recording = m_streamer->isRecording();
    params["recording"] = recording;
    if (recording) {
        params["recStartTime"] = QString::number(m_streamer->getRecStartTime());
    }
    params["targetId"] = m_targetingId;
    params["horizontalBorder"] = m_horizontalBorder;
    params["verticalBorder"] = m_verticalBorder;

    return params;
}

void Processor::setTargetingId(int id)
{
    m_targetingId = id;
}

void Processor::handleFrameWithNN(QImage frame)
{
    m_videoSize = frame.size();
    cv::Mat input(m_videoSize.height(), m_videoSize.width(), CV_8UC4, frame.bits());
    cv::cvtColor(input, input, cv::COLOR_BGRA2RGB);

    std::vector<DeepSORT::ObjectInfo> objects = m_deepSort->forward(input);

    for (DeepSORT::ObjectInfo object : objects) {
        if (object.id == m_targetingId) {
            m_cameraDirections = getDirections(QRect(
                    object.bbox.x,
                    object.bbox.y,
                    object.bbox.width,
                    object.bbox.height
                ));
        }
    }

    m_server->handleObjectsRequest(objects);
}

void Processor::moveCamera() {
    if (m_isTracking && (m_cameraDirections.first != Direction::hold ||
                         m_cameraDirections.second != Direction::hold))
        emit this->moveCameraRequest(m_cameraDirections);
}

QPair<Direction, Direction> Processor::getDirections(QRect bbox) {
    //QSize frame_size = m_videoSink->videoSize();
    QRect border(
        int(m_videoSize.width() / 2 - m_videoSize.width() * m_horizontalBorder / 2),
        int(m_videoSize.height() / 2 - m_videoSize.height() * m_verticalBorder / 2),
        int(m_videoSize.width() * m_horizontalBorder),
        int(m_videoSize.height() * m_verticalBorder)
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
