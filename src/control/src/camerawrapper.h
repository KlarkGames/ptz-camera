#ifndef CAMERAWRAPPER_H
#define CAMERAWRAPPER_H

#include <QObject>
#include <QQmlEngine>
#include <QMediaDevices>
#include <QCamera>
#include <QStringList>
#include <QMediaCaptureSession>
#include <QVideoFrame>
#include <QTimer>

class CameraWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QCamera* camera READ getCamera WRITE setCamera NOTIFY cameraChanged)
    Q_PROPERTY(QString currentCameraId READ currentCameraId WRITE setCurrentCamera NOTIFY cameraChanged)
    Q_PROPERTY(QStringList availableCameraIds READ availableCameraIds NOTIFY availableCamerasChanged)

    QML_ELEMENT

    public:
        CameraWrapper();

        void setCurrentCamera(QString cameraId);
        void setCamera(QCamera* newCamera);

        QStringList availableCameraIds();
        QString currentCameraId();

        QCamera* getCamera();

    signals:
        void cameraChanged();
        void availableCamerasChanged();


    private slots:
        void handleTimeout();


    private:
        QTimer m_timer;
        QStringList m_availableCameraIds;
        QCamera* m_camera = nullptr;
        QCameraDevice* m_cameraDevice = nullptr;
};

#endif // CAMERAWRAPPER_H
