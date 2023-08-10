#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QObject>
#include <QPointer>
#include <QSharedPointer>
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

#include "server.h"
#include "mountdriver.h"
#include "streamer.h"
#include "deepsort.h"

class Processor : public QObject
{
    Q_OBJECT

    public:
        explicit Processor(QObject *parent = nullptr);
        ~Processor();
        void setSettings(QJsonObject params);
        QJsonObject getSettings();

        MountDriver *mountDriver() const;
        void rotateMount(QJsonObject);
        void setCamera(QString cameraName);

        QPair<Direction, Direction> getDirections(QRect bbox);
        void setTracking(bool value);
        void setTargetingId(int id);


    signals:
        void trackingStatusChanged(bool value);
        void videoSinkChanged();
        void cameraWrapperChanged();
        void mountDriverChanged();
        void moveCameraRequest(QPair<Direction, Direction> cameraDirections);
        void settingsChanged(QJsonObject params);

    public slots:
        void moveCamera();
        void handleFrameWithNN(QImage frame);



    private:
        QPointer<MountDriver> m_mountDriver;
        QPointer<Streamer> m_streamer;
        QPointer<Server> m_server;
        QSize m_videoSize;
        bool m_isTracking = false;

        QSharedPointer<DeepSORT> m_deepSort;
        QTimer m_timer;

        double m_verticalBorder;
        double m_horizontalBorder;
        int m_targetingId = -1;
        int m_counter;
        int m_max_count;
        QPair<Direction, Direction> m_cameraDirections;
};

#endif // PROCESSOR_H
