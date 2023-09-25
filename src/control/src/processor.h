#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QObject>
#include <QSharedPointer>
#include <QVideoSink>
#include <QTimer>
#include <QImage>
#include <QDateTime>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <QCamera>

#include "server.h"
#include "mountdriver.h"
#include "streamer.h"
#include "deepsort.h"
#include "settingsconductor.h"

class Processor : public QObject, public SettingsConductor
{
    Q_OBJECT

    public:
        explicit Processor(QObject *parent = nullptr);
        ~Processor();
        void setSettings(QJsonObject params) override;
        void setCamera(QString cameraName);
        void setTracking(bool value);
        void setTargetingId(int id);

        void rotateMount(QJsonObject);

        QJsonObject getSettings() override;
        QPair<Direction, Direction> getDirections(QRect bbox);

    signals:
        void trackingStatusChanged(bool value);
        void videoSinkChanged();
        void moveCameraRequest(QPair<Direction, Direction> cameraDirections);
        void settingsChanged(QJsonObject params);

    public slots:
        void moveCamera();
        void handleFrameWithNN(QImage frame);

    private:
        QSharedPointer<MountDriver> m_mountDriver;
        QSharedPointer<Streamer> m_streamer;
        QSharedPointer<Server> m_server;
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
