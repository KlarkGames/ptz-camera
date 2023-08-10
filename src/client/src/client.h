#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QDateTime>
#include <QtQml/qqmlregistration.h>
#include <QtWebSockets>
#include <QMediaPlayer>
#include <QThreadPool>
#include <QRunnable>

#include "trackingobjectmodel.h"

class Client : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(TrackingObjectModel* trackingObjectModel READ trackingObjectModel CONSTANT)

    Q_PROPERTY(bool isRecording READ isRecording NOTIFY settingsChanged)
    Q_PROPERTY(bool isTracking READ isTracking NOTIFY settingsChanged)

    Q_PROPERTY(double horizontalBorder MEMBER m_horizontalBorder NOTIFY settingsChanged)
    Q_PROPERTY(double verticalBorder MEMBER m_verticalBorder NOTIFY settingsChanged)

    Q_PROPERTY(QString currentCamera MEMBER m_currentCamera NOTIFY settingsChanged)
    Q_PROPERTY(QString currentPort MEMBER m_currentPort NOTIFY settingsChanged)

    Q_PROPERTY(QStringList getCameras READ getCameras NOTIFY settingsChanged);
    Q_PROPERTY(QStringList getPorts READ getPorts NOTIFY settingsChanged);

    Q_PROPERTY(QMediaPlayer* mediaPlayer READ mediaPlayer WRITE setMediaPlayer NOTIFY mediaPlayerChanged)
public:
    enum RotateDirection{
        DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN
    };
    Q_ENUM(RotateDirection)

    explicit Client(QObject *parent = nullptr);
    QMediaPlayer* mediaPlayer();
    void setMediaPlayer(QMediaPlayer *mediaPlayer);

    TrackingObjectModel* trackingObjectModel();
    bool isRecording();
    bool isTracking();

    Q_INVOKABLE QTime getRecElapsedTimeMSecs();
    Q_INVOKABLE QStringList getCameras();
    Q_INVOKABLE QStringList getPorts();

    Q_INVOKABLE void connectToHost(QString addr);
    Q_INVOKABLE void closeConnection();
    Q_INVOKABLE bool sendRotateCmd(RotateDirection dir, bool launch);
    Q_INVOKABLE QJsonObject getSettings();
    Q_INVOKABLE bool sendSetSettingsAsk(QJsonObject params);
    Q_INVOKABLE bool sendGetSettings();

    virtual ~Client();

private slots:
    void afterConnected();
    void handleReceivedMessage(QStringView msg_s);

signals:
    void connected();
    void disconnected();
    void mediaPlayerChanged();
    void settingsChanged();

private:
    void setIsRecording(bool value);
    void setIsTracking(bool value);
    bool m_isRecording = false, m_isTracking = false;
    qint64 m_recStartTime;
    QWebSocket m_socket;
    TrackingObjectModel m_trackingObjectModel;
    QMediaPlayer *m_player;

    QStringList m_availableCameras;
    QStringList m_avaliablePorts;

    QString m_currentCamera;
    QString m_currentPort;

    double m_horizontalBorder = 0.7;
    double m_verticalBorder = 0.7;
};

#endif // CLIENT_H
