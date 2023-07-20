#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QDateTime>
#include <QtQml/qqmlregistration.h>
#include <QtWebSockets>

#include "trackingobjectmodel.h"

class Client : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QUrl streamSource READ streamSource NOTIFY streamSourceChanged)
    Q_PROPERTY(TrackingObjectModel* trackingObjectModel READ trackingObjectModel CONSTANT)
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY isRecordingChanged)
    Q_PROPERTY(bool isTracking READ isTracking NOTIFY isTrackingChanged)
public:
    enum RotateDirection{
        DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN
    };
    Q_ENUM(RotateDirection)

    explicit Client(QObject *parent = nullptr);
    QUrl streamSource();

    TrackingObjectModel* trackingObjectModel();
    bool isRecording();
    bool isTracking();
    Q_INVOKABLE QTime getRecElapsedTimeMSecs();

    Q_INVOKABLE void connectToHost(QString addr);
    Q_INVOKABLE void closeConnection();
    Q_INVOKABLE bool sendRotateCmd(RotateDirection dir, bool launch);
    Q_INVOKABLE bool sendSetRecordingCmd(bool value);
    Q_INVOKABLE bool sendSetTrackingCmd(bool value);
    virtual ~Client();

private slots:
    void afterConnected();
    void handleReceivedMessage(QStringView msg_s);

signals:
    void connected();
    void disconnected();
    void streamSourceChanged();
    void isRecordingChanged();
    void isTrackingChanged();

private:
    void setIsRecording(bool value);
    void setIsTracking(bool value);
    bool m_isRecording = false, m_isTracking = false;
    qint64 m_recStartTime;
    QWebSocket m_socket;
    QUrl m_streamSource;
    TrackingObjectModel m_trackingObjectModel;
};

#endif // CLIENT_H
