#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtNetwork>
#include <QIODevice>
#include <QtWebSockets>
#include <QtCore>
#include <QJsonObject>
#include <QJsonArray>

#include "deepsort.h"

class Server : public QObject
{
    Q_OBJECT

    public:
        explicit Server(QObject *parent = nullptr);
        void setRecordingStatus(bool value, qint64 time);
        void setTrackingStatus(bool value);
        void handleObjectsRequest(std::vector<DeepSORT::ObjectInfo> objects);
        QHostAddress address();

    signals:
        void dataReceived(QByteArray);
        void rotateCmdReceived(QJsonObject);
        void setRecordingCmdReceived(bool value);
        void setTrackingCmdReceived(bool value);

    private:
        void initServer();

        bool m_debug = true;
        bool m_isRecording = false, m_isTracking = false;
        QHostAddress m_address;
        qint64 m_recStartTime;
        QWebSocketServer *m_pWebSocketServer;
        QList<QWebSocket*> m_clients;

    private slots:
        void handleConnection();
        void processTextMessage(QString message);
        void socketDisconnected();
        void handleCommand(QJsonDocument doc);

};

#endif // SERVER_H
