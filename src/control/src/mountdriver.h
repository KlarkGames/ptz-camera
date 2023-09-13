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
#include <QJsonArray>
#include <QCamera>
#include <QMediaDevices>
#include <QDebug>

#include "arduinocommand.h"
#include "settingsconductor.h"

class MountDriver : public QObject, public SettingsConductor
{
    Q_OBJECT
    public:
        MountDriver();
        void setSettings(QJsonObject params) override;
        void setCurrentPort(QString portName);

        QJsonObject getSettings() override;
        QStringList availablePortNames();
        QString currentPortName();
        QStringList availableCameraIds();

        void rotate(QJsonObject params);
        bool portExists();
        bool portIsOpen();


        QPair<Direction, Direction> arduinoState;

    signals:
        void currentPortChanged();
        void availablePortsChanged();
        void availableCamerasChanged();

    public slots:
        void handleNeuralNetRequest(QPair<Direction, Direction> directions);

    private slots:
        void handleTimeout();
        void handleError(QSerialPort::SerialPortError error);
        void handleReadyRead();

    private:
        QTimer m_timer;
        QStringList m_availableCameraIds;
        QMap<QString, QSerialPort*> m_availablePorts;
        QSerialPort *m_serialPort = nullptr;

        void sendSignal(QString signal);
        void updateAvailablePorts();
        void updateAvailableCameras();
};

#endif // MOUNTDRIVER_H
