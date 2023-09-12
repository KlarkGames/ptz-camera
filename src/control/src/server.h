#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtNetwork>
#include <QIODevice>
#include <QtWebSockets>
#include <QtCore>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>

#include "deepsort.h"

class Server : public QObject
{
    Q_OBJECT

    public:
        explicit Server(QObject *parent = nullptr);
        void updClientSettings(QJsonObject params);
        void handleObjectsRequest(std::vector<TrackingObject::ObjectInfo> objects);
        QHostAddress address();

    signals:
        void newConnection();
        void dataReceived(QByteArray);
        void rotateCmdReceived(QJsonObject);

        void setSettingRecieved(QJsonObject params);
        void setTrackingObjecIdRecieved(int id);

        void getSettingsRequest();

    private:
        void initServer();
        void initBroadcast();
        QTimer m_broadcastTimer;
        QUdpSocket m_broadcastSocket;
        static const int PORT = 41419;
        static const int BROADCAST_INTERVAL_MS = 1000;

        bool m_debug = true;
        bool m_isRecording = false, m_isTracking = false;
        QHostAddress m_address;
        qint64 m_recStartTime;
        QWebSocketServer *m_pWebSocketServer;
        QList<QWebSocket*> m_clients;

    private slots:
        void sendBroadcastPacket();
        void handleConnection();
        void processTextMessage(QString message);
        void socketDisconnected();
        void handleCommand(QJsonDocument doc);

};

#endif // SERVER_H
