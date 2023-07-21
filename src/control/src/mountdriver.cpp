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

    QString signal = "";

    if (direction == "left") {
        signal += 'L';
    } else if (direction == "right") {
        signal += 'R';
    } else if (direction == "up") {
        signal += 'U';
    } else if (direction == "down") {
        signal += 'D';
    } else {
        qDebug("Invalid direction. Got: " + direction.toLatin1() + ", allowed \"left\", \"right\", \"up\", \"down\"");
    }

    if (command == "launch") {
        signal += 'L';
    } else if (command == "stop") {
        signal += 'S';
    } else {
        qDebug("Invalid command. Got: " + command.toLatin1() + ", allowed \"launch\", \"stop\"");
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

//    if (horizontal_direction == Direction::left)
//        this->sendSignal(5, 2, 1, 50);
//    else if (horizontal_direction == Direction::right)
//        this->sendSignal(5, 2, 0, 50);

//    if (vertical_direction == Direction::top)
//        this->sendSignal(6, 3, 0, 50);
//    else if (vertical_direction == Direction::bottom)
//        this->sendSignal(6, 3, 1, 50);
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
