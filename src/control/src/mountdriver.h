#ifndef MOUNTDRIVER_H
#define MOUNTDRIVER_H

#include <QObject>
#include <QQmlEngine>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTextStream>
#include <QStringList>
#include <QDebug>

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
        Q_INVOKABLE void sendSignal(int dirPin, int stepPin, int direction, int stepCount);

    signals:
        void currentPortChanged();
        void availablePortsChanged();

    private slots:
        void handleBytesWritten(qint64 bytes);
        void handleTimeout();
        void handleError(QSerialPort::SerialPortError error);
        void handleReadyRead();

    private:
        QTimer m_timer;
        QStringList m_availablePortNames;
        QSerialPort *m_serialPort = nullptr;
};

#endif // MOUNTDRIVER_H
