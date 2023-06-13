#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtNetwork>
#include <QIODevice>
#include <QtWebSockets>
#include <QtCore>

class Server : public QObject
{
    Q_OBJECT

    public:
        explicit Server(QObject *parent = nullptr);

    signals:
        void dataReceived(QByteArray);
        void rotateCmdRecieved(QVariantMap);

    private:
        void initServer();

        bool m_debug = true;
        QWebSocketServer *m_pWebSocketServer;
        QList<QWebSocket *> m_clients;

        QMetaObject::Connection m_newClientHandler;

    private slots:
        void handleConnection();
        void processTextMessage(QString message);
        void socketDisconnected();
        void handleCommand(QJsonDocument doc);

};

#endif // SERVER_H
