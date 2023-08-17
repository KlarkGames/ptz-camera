#include "mountdriver.h"

MountDriver::MountDriver()
{
    connect(&m_timer, &QTimer::timeout, this, &MountDriver::handleTimeout);
    m_timer.setInterval(5000);
    this->m_timer.start();
}

void MountDriver::setCurrentPort(QString portName)
{
    qDebug("Setting active port to: %s", qUtf8Printable(portName));
    if (this->portIsOpen() && portName != this->m_serialPort->portName()) {
        this->m_serialPort->close();
    }

    if (m_availablePorts.contains(portName)) {
        this->m_serialPort = m_availablePorts[portName];
        this->m_serialPort->setBaudRate(9600);
        this->m_serialPort->open(QIODeviceBase::ReadWrite);

        connect(m_serialPort, &QSerialPort::errorOccurred,
                this, &MountDriver::handleError);
        connect(m_serialPort, &QSerialPort::readyRead,
                this, &MountDriver::handleReadyRead);
    }
}

bool MountDriver::portExists()
{
    return this->m_serialPort != nullptr;
}

bool MountDriver::portIsOpen()
{
    return this->portExists() && this->m_serialPort->isOpen();
}

void MountDriver::setSettings(QJsonObject params)
{
    if (params.contains("currentPort")) {
        setCurrentPort(params["currentPort"].toString());
    }
}

QJsonObject MountDriver::getSettings()
{
    QJsonObject params;

    params["avaliablePorts"] = QJsonArray::fromStringList(availablePortNames());
    params["currentPort"] = currentPortName();
    params["avaliableCameras"] = QJsonArray::fromStringList(availableCameraIds());

    return params;
}

QStringList MountDriver::availablePortNames()
{
    QStringList portNames;
    portNames.append("");
    portNames.append(this->m_availablePorts.keys());
    return portNames;
}

QString MountDriver::currentPortName()
{
    if (this->m_serialPort == nullptr) {
        return "";
    }

    return this->m_serialPort->portName();
}

QStringList MountDriver::availableCameraIds()
{
    return m_availableCameraIds;
}

void MountDriver::rotate(QJsonObject params)
{
    QString direction = params.value("direction").toString();
    QString command = params.value("command").toString();

    Direction *directionPointer;
    Direction value;

    QPointer<ArduinoCommand> arduinoCommand = stringToArduinoCommand(direction);
    if (command == "launch") {
        this->sendSignal(QString::fromStdString(arduinoCommand->start().to_string()));
    } else if (command == "stop") {
        this->sendSignal(QString::fromStdString(arduinoCommand->stop().to_string()));
    } else {
        qDebug() << "Invalid command. Got: "
                 << command.toLatin1()
                 << ", allowed \"launch\", \"stop\"";
    }

    // TODO: Delete after Artuino return state implementation

    if (direction == "left") {
        directionPointer = &arduinoState.first;
        value = Direction::left;
    } else if (direction == "right") {
        directionPointer = &arduinoState.first;
        value = Direction::right;
    } else if (direction == "up") {
        directionPointer = &arduinoState.second;
        value = Direction::top;
    } else if (direction == "down") {
        directionPointer = &arduinoState.second;
        value = Direction::bottom;
    }


    if (command == "launch") {
        *directionPointer = value;
    } else if (command == "stop") {
        *directionPointer = Direction::hold;
    }
}

void MountDriver::handleReadyRead()
{
    QByteArray readData;
    readData.append(m_serialPort->readAll());
    if (!readData.isEmpty()) {
        qDebug("Got message from port: %s", qUtf8Printable(readData));
    }
}

void MountDriver::handleTimeout()
{
    updateAvailablePorts();
    updateAvailableCameras();
}

void MountDriver::updateAvailablePorts()
{
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QString portName = info.portName();
        if (!m_availablePorts.contains(portName)) {
            m_availablePorts[portName] = new QSerialPort(info);
            qDebug("Port available: %s", qUtf8Printable(info.portName()));
        }
    }
    emit availablePortsChanged();
}

void MountDriver::updateAvailableCameras()
{
    foreach (const QCameraDevice &camera, QMediaDevices::videoInputs()) {
        QString cameraId = camera.id();
        if (!this->m_availableCameraIds.contains(cameraId)) {
            this->m_availableCameraIds.append(cameraId);
            qDebug("Camera available: %s", qUtf8Printable(camera.description()));
        }
    }
    emit availableCamerasChanged();
}

void MountDriver::handleError(QSerialPort::SerialPortError error)
{
    qDebug() << QObject::tr("Operation timed out for port %1, error: %2")
                .arg(m_serialPort->portName(), m_serialPort->errorString())
             << Qt::endl;
}

void MountDriver::handleNeuralNetRequest(QPair <Direction, Direction> directions) {
    Direction horizontal_direction = directions.first;
    Direction vertical_direction = directions.second;

    ArduinoCommand *horizontalCommand = directionToArduinoCommand(directions.first);
    ArduinoCommand *verticalCommand = directionToArduinoCommand(directions.second);

    std::bitset<4> preSignal = horizontalCommand->start() | verticalCommand->start();
    QString signal = QString::fromStdString(preSignal.to_string());

    // TODO: Delete after arduino return implementation

    switch (horizontal_direction) {
    case Direction::left:
        arduinoState.first = Direction::left;
        break;
    case Direction::right:
        arduinoState.first = Direction::right;
        break;
    default:
        arduinoState.first = Direction::hold;
        break;
    }

    switch (vertical_direction) {
    case Direction::top:
        arduinoState.second = Direction::top;
        break;
    case Direction::bottom:
        arduinoState.second = Direction::bottom;
        break;
    default:
        arduinoState.second = Direction::hold;
        break;
    }

    sendSignal(signal);
}

void MountDriver::sendSignal(QString signal)
{
    if (this->m_serialPort == nullptr) {
        qDebug("Port is not intiated");
        return;
    }
    if (!this->m_serialPort->isOpen()) {
        qDebug("Port is not opened");
        return;
    }

    qDebug("Sending: %s", qUtf8Printable(signal));

    QByteArray signalBytes = signal.toUtf8();
    const qint64 bytesWritten = m_serialPort->write(signalBytes);

    if (bytesWritten == -1) {
        qDebug() << QObject::tr("Failed to write the data to port %1, error: %2")
                            .arg(m_serialPort->portName(), m_serialPort->errorString())
                 << Qt::endl;
    } else if (bytesWritten != signalBytes.size()) {
        qDebug() << QObject::tr("Failed to write all the data to port %1, error: %2")
                            .arg(m_serialPort->portName(), m_serialPort->errorString())
                 << Qt::endl;
    }
}
