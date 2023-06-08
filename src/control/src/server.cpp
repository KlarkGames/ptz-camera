#include "server.h"

Server::Server(QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(new QWebSocketServer(QStringLiteral("Chat Server"),
                                            QWebSocketServer::NonSecureMode,
                                            this))
{
    initServer();
}

void Server::initServer()
{
    int port = 41419;
    QHostAddress ipAddress;
    const QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (const QHostAddress &entry : ipAddressesList) {
        if (entry != QHostAddress::LocalHost && entry.toIPv4Address()) {
            ipAddress = entry;
            break;
        }
    }

    if (ipAddress.isNull()) {
        qCritical() << tr("Server:") << tr("Unable to bind network address.");
        return;
    }
    if (m_pWebSocketServer->listen(ipAddress, port)) {
        if (m_debug)
            qDebug() << tr("The server is running on: %1:%2").arg(ipAddress.toString()).arg(port);
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                this, &Server::handleConnection);
    }
}

void Server::handleConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &Server::processTextMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &Server::socketDisconnected);

    m_clients << pSocket;
}

void Server::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "Message received:" << message;
    if (pClient) {
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &jsonError);
        if (jsonError.error != QJsonParseError::NoError){
            qDebug() << jsonError.errorString();
            pClient->sendTextMessage(jsonError.errorString());
        }
         else {
            this->handleCommand(doc);
        }
    }
}

void Server::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "socketDisconnected:" << pClient;
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void Server::handleCommand(QJsonDocument doc)
{
    QJsonObject jObject = doc.object();
    QVariantMap mainMap = jObject.toVariantMap();
    QString protocol = mainMap.value("jsonrpc").toString();
    QString command = mainMap.value("method").toString();
    if (command == "rotate") {
        QVariantMap paramsMap = mainMap.value("params").toMap();
        emit rotateCmdRecieved(paramsMap);
    }
}
