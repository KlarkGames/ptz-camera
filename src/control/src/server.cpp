#include "server.h"
#include "common_defs.h"

Server::Server(QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(new QWebSocketServer(QStringLiteral("Chat Server"),
                                            QWebSocketServer::NonSecureMode,
                                            this))
{
    initServer();
    initBroadcast();
}

void Server::initServer()
{
    const QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (const QHostAddress &entry : ipAddressesList) {
        if (entry != QHostAddress::LocalHost && entry.toIPv4Address()) {
            m_address = entry;
            break;
        }
    }

    if (m_address.isNull()) {
        qCritical() << tr("Server:") << tr("Unable to bind network address.");
        return;
    }
    if (m_pWebSocketServer->listen(m_address, WS_PORT)) {
        if (m_debug)
            qDebug() << tr("The server is running on: %1:%2").arg(m_address.toString()).arg(WS_PORT);
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                this, &Server::handleConnection);
    }
}

void Server::initBroadcast()
{
    QObject::connect(&m_broadcastTimer, &QTimer::timeout, this, &Server::sendBroadcastPacket);
    m_broadcastTimer.start(BROADCAST_INTERVAL_MS);
}

void Server::sendBroadcastPacket()
{
    QByteArray data = m_address.toString().toUtf8();
    auto n = m_broadcastSocket.writeDatagram(data, QHostAddress::Broadcast, BROADCAST_PORT);

    if (m_debug && n < data.size()) {
        qDebug() << tr("Failed to broadcast datagram!");
    }
}

QHostAddress Server::address()
{
    return m_address;
}

void Server::handleConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &Server::processTextMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &Server::socketDisconnected);

    if (m_debug)
        qDebug() << "Connected: " << pSocket->localAddress();

    m_clients << pSocket;

    emit newConnection();
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

void Server::updClientSettings(QJsonObject params)
{
    QJsonObject msg;
    msg["jsonrpc"] = "2.0";
    msg["method"] = "updSettings";
    msg["params"] = params;
    QString msg_s = QJsonDocument(msg).toJson(QJsonDocument::Compact);

    for (auto *client : m_clients)
        client->sendTextMessage(msg_s);
}

void Server::handleCommand(QJsonDocument doc)
{
    QJsonObject msg = doc.object();

    QString command = msg.value("method").toString();
    QJsonObject params = msg.value("params").toObject();

    if (command == "rotate") {
        emit rotateCmdReceived(params);
    } else if (command == "setSettingsAsk") {
        emit setSettingRecieved(params);
    } else if (command == "getSettings") {
        emit getSettingsRequest();
    }
}

void Server::handleObjectsRequest(std::vector<TrackingObject::ObjectInfo> objects)
{
    QJsonObject msg, params;
    QJsonArray objArray;

    for (auto obj : objects) {
        objArray.append(obj.toJson());
    }

    params["objects"] = objArray;
    msg["jsonrpc"] = "2.0";
    msg["method"] = "updTrackingObjects";
    msg["params"] = params;

    QString msg_s = QJsonDocument(msg).toJson(QJsonDocument::Compact);

    for (auto *client : m_clients)
        client->sendTextMessage(msg_s);
}
