#include "camerawrapper.h"

CameraWrapper::CameraWrapper()
{
    connect(&m_timer, &QTimer::timeout, this, &CameraWrapper::handleTimeout);
    m_timer.setInterval(5000);
    this->m_timer.start();
}

void CameraWrapper::setCurrentCamera(QString cameraId)
{
    qDebug("Setting current camera to: %s", qUtf8Printable(cameraId));

    foreach (const QCameraDevice &cameraDevice, QMediaDevices::videoInputs()) {
        QString avaliableId = cameraDevice.id();
        if (avaliableId == cameraId) {

            this->m_cameraDevice = new QCameraDevice(cameraDevice);

            this->m_camera = new QCamera(cameraDevice);
            this->m_camera->start();
            emit this->cameraChanged();
        }
    }
}

QCamera* CameraWrapper::getCamera()
{
    return m_camera;
}


void CameraWrapper::setCamera(QCamera* newCamera)
{
    m_camera = newCamera;
}


QStringList CameraWrapper::availableCameraIds()
{
    QStringList cameraId;
    cameraId.append("");
    cameraId.append(this->m_availableCameraIds);
    return cameraId;
}


QString CameraWrapper::currentCameraId()
{
    if (this->m_camera == nullptr) {
        return "";
    }

    return this->m_camera->cameraDevice().id();
}

void CameraWrapper::handleTimeout() {
    foreach (const QCameraDevice &camera, QMediaDevices::videoInputs()) {
        QString cameraId = camera.id();
        if (!this->m_availableCameraIds.contains(cameraId)) {
            this->m_availableCameraIds.append(cameraId);
            qDebug("Camera available: %s", qUtf8Printable(camera.description()));
        }
    }
    emit availableCamerasChanged();
}
