#ifndef MOUNTDRIVER_H
#define MOUNTDRIVER_H

#include <QObject>
#include <QQmlEngine>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTextStream>
#include <QStringList>
#include <QJsonObject>
#include <QCamera>
#include <QMediaDevices>
#include <QDebug>
#include "utils.h"

class MountDriver : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentPortName READ currentPortName WRITE setCurrentPort NOTIFY currentPortChanged)
    Q_PROPERTY(QStringList availablePortNames READ availablePortNames NOTIFY availablePortsChanged)

    QML_ELEMENT

    public:
        MountDriver();
        void setCurrentPort(QString portName);
        QStringList availablePortNames();
        QString currentPortName();
        QStringList availableCameraIds();
        void rotate(QJsonObject params);
        QPair<Direction, Direction> arduinoState;

    signals:
        void currentPortChanged();
        void availablePortsChanged();
        void availableCamerasChanged();

    public slots:
        void handleNeuralNetRequest(QPair<Direction, Direction> directions);

    private slots:
        void handleBytesWritten(qint64 bytes);
        void handleTimeout();
        void handleError(QSerialPort::SerialPortError error);
        void handleReadyRead();

    private:
        QTimer m_timer;
        QStringList m_availablePortNames;
        QStringList m_availableCameraIds;
        QSerialPort *m_serialPort = nullptr;
        void sendSignal(QString signal);
};

#endif // MOUNTDRIVER_H
