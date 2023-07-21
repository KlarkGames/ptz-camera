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
    if (this->m_serialPort == nullptr) {
        this->m_serialPort = new QSerialPort(this);
    }
    if (this->m_serialPort->isOpen() && portName != this->m_serialPort->portName()) {
        this->m_serialPort->close();
    }

    foreach(QString availablePortName, this->m_availablePortNames) {
        if (availablePortName == portName) {
            this->m_serialPort->setPortName(portName);
            this->m_serialPort->setBaudRate(9600);
            this->m_serialPort->open(QIODeviceBase::ReadWrite);

            connect(m_serialPort, &QSerialPort::bytesWritten,
                    this, &MountDriver::handleBytesWritten);
            connect(m_serialPort, &QSerialPort::errorOccurred,
                    this, &MountDriver::handleError);
            connect(m_serialPort, &QSerialPort::readyRead,
                    this, &MountDriver::handleReadyRead);
        }
    }
}

QStringList MountDriver::availablePortNames()
{
    QStringList portNames;
    portNames.append("");
    portNames.append(this->m_availablePortNames);
    return portNames;
}

QString MountDriver::currentPortName()
{
    if (this->m_serialPort == nullptr) {
        return "";
    }

    return this->m_serialPort->portName();
}

void MountDriver::rotate(QJsonObject params)
{
    QString direction = params.value("direction").toString();
    QString command = params.value("command").toString();

    QString signal = "0000";

    QChar *powerPointer = nullptr;
    Direction *directionPointer;
    Direction value;

    if (direction == "left") {
        directionPointer = &arduinoState.first;
        value = Direction::left;
        signal[0] = '1';
        powerPointer = &signal[1];
    } else if (direction == "right") {
        directionPointer = &arduinoState.first;
        value = Direction::right;
        signal[0] = '0';
        powerPointer = &signal[1];
    } else if (direction == "up") {
        directionPointer = &arduinoState.second;
        value = Direction::top;
        signal[2] = '0';
        powerPointer = &signal[3];
    } else if (direction == "down") {
        directionPointer = &arduinoState.second;
        value = Direction::bottom;
        signal[2] = '1';
        powerPointer = &signal[3];
    } else {
        qDebug("Invalid direction. Got: " + direction.toLatin1() + ", allowed \"left\", \"right\", \"up\", \"down\"");
    }

    if (powerPointer != nullptr) {
        if (command == "launch") {
            *powerPointer = '1';
            *directionPointer = value;
        } else if (command == "stop") {
            *powerPointer = '0';
            *directionPointer = Direction::hold;
        } else {
            qDebug("Invalid command. Got: " + command.toLatin1() + ", allowed \"launch\", \"stop\"");
        }
    }
    this->sendSignal(signal);
}

void MountDriver::handleReadyRead()
{
    QByteArray readData;
    readData.append(m_serialPort->readAll());
    if (!readData.isEmpty()) {
        qDebug("Got message from port: %s", qUtf8Printable(readData));
    }
}

void MountDriver::handleBytesWritten(qint64 bytes)
{
}

void MountDriver::handleTimeout()
{
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QString portName = info.portName();
        if (!this->m_availablePortNames.contains(portName)) {
            this->m_availablePortNames.append(portName);
            qDebug("Port available: %s", qUtf8Printable(info.portName()));
        }
    }
    emit availablePortsChanged();
}

void MountDriver::handleError(QSerialPort::SerialPortError error)
{
    qDebug() << QObject::tr("Operation timed out for port %1, error: %2")
                .arg(m_serialPort->portName())
                .arg(m_serialPort->errorString())
             << Qt::endl;
}

void MountDriver::handleNeuralNetRequest(QPair <Direction, Direction> directions) {
    Direction horizontal_direction = directions.first;
    Direction vertical_direction = directions.second;

    QVariantMap horizontal_map;
    QVariantMap vertical_map;

    QString signal = "";

    switch (horizontal_direction) {
    case Direction::left:
        signal += "11";
        arduinoState.first = Direction::left;
        break;
    case Direction::right:
        signal += "01";
        arduinoState.first = Direction::right;
        break;
    default:
        signal += "00";
        arduinoState.first = Direction::hold;
        break;
    }

    switch (vertical_direction) {
    case Direction::top:
        signal += "01";
        arduinoState.second = Direction::top;
        break;
    case Direction::bottom:
        signal += "11";
        arduinoState.second = Direction::bottom;
        break;
    default:
        signal += "00";
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
                            .arg(m_serialPort->portName())
                            .arg(m_serialPort->errorString())
                         << Qt::endl;
    } else if (bytesWritten != signalBytes.size()) {
        qDebug() << QObject::tr("Failed to write all the data to port %1, error: %2")
                            .arg(m_serialPort->portName())
                            .arg(m_serialPort->errorString())
                         << Qt::endl;
    }
}
