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

void MountDriver::rotate(QVariantMap paramsMap)
{
    QString axis = paramsMap.value("axis").toString();
    int direction = paramsMap.value("direction").toInt();
    int steps = paramsMap.value("steps").toInt();
    int dirPin, stepPin = 0;
    if (axis == "x") {
        dirPin = 5;
        stepPin = 2;
    } else if (axis == "y") {
        dirPin = 6;
        stepPin = 3;
    } else {
        qDebug() << "Wrong axis";
        return;
    }
    this->sendSignal(dirPin, stepPin, direction, steps);
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

void MountDriver::sendSignal(int dirPin, int stepPin, int direction, int stepCount)
{
    if (this->m_serialPort == nullptr) {
        qDebug("Port is not intiated");
        return;
    }
    if (!this->m_serialPort->isOpen()) {
        qDebug("Port is not opened");
        return;
    }
    QString signal = QString("%1 %2 %3 %4")
            .arg(dirPin)
            .arg(stepPin)
            .arg(direction)
            .arg(stepCount);
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
