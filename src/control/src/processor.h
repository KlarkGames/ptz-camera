#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QObject>
#include <QPointer>
#include <QVideoSink>
#include <QQmlEngine>
#include <QTimer>
#include <QImage>
#include <QPainter>
#include <QSize>
#include <QVideoFrame>
#include <QRandomGenerator>
#include <QDateTime>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <QImageWriter>
#include <QCamera>
#include "camerawrapper.h"

#include "server.h"
#include "mountdriver.h"
#include "streamer.h"
#include "deepsort.h"

class Processor : public QObject
{
    Q_OBJECT
    //Q_PROPERTY(QVideoSink* processorVideoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
    Q_PROPERTY(MountDriver* mountDriver READ mountDriver NOTIFY mountDriverChanged)
    Q_PROPERTY(CameraWrapper* cameraWrapper READ getCameraWrapper WRITE setCameraWrapper NOTIFY cameraWrapperChanged)
    QML_ELEMENT

    public:
        explicit Processor(QObject *parent = nullptr);
        //QVideoSink *videoSink() const;
        MountDriver *mountDriver() const;
        CameraWrapper *getCameraWrapper() const;
        //void setVideoSink(QVideoSink *newVideoSink);
        void setCameraWrapper(CameraWrapper *newCameraWrapper);
        void setCamera(QString cameraName);
        QPair<Direction, Direction> getDirections(QRect bbox);
        Q_INVOKABLE void rotateMount(QVariantMap);
        void setTracking(bool value);


    signals:
        void trackingStatusChanged(bool value);
        void videoSinkChanged();
        void cameraWrapperChanged();
        void mountDriverChanged();
        void moveCameraRequest(QPair<Direction, Direction> cameraDirections);
        //void handleFrameRequest(const QVideoFrame &frame);
        void handleObjectsRequest(std::vector<ObjectInfo> objects);

    public slots:
        //void hvideoFrameChanged(const QVideoFrame &frame);
        void moveCamera();
        void handleFrameWithNN(QImage frame);



    private:
        //QPointer<QVideoSink> m_videoSink;
        QPointer<MountDriver> m_mountDriver;
        QPointer<Streamer> m_streamer;
        QPointer<Server> m_server;
        QSize m_videoSize;
        bool m_isTracking = false;

        QPointer<CameraWrapper> m_cameraWrapper;
        QPointer<QCamera> camera;
        QTimer m_timer;

        DeepSORT m_deepSort;

        float vertical_border;
        float horizontal_border;
        int m_counter;
        int m_max_count;
        QPair<Direction, Direction> cameraDirections;
};

#endif // PROCESSOR_H
