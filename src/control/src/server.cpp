#include "server.h"
#include <QJsonObject>

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
    //QHostAddress ipAddress;
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
    if (m_pWebSocketServer->listen(m_address, port)) {
        if (m_debug)
            qDebug() << tr("The server is running on: %1:%2").arg(m_address.toString()).arg(port);
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                this, &Server::handleConnection);
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

    QJsonObject msg;
    msg["event"] = "init";
    msg["isRecording"] = m_isRecording;
    if (m_isRecording)
        msg["recStartTime"] = QString::number(m_recStartTime);
    msg["isTracking"] = m_isTracking;
    QString msg_s = QJsonDocument(msg).toJson(QJsonDocument::Compact);

    pSocket->sendTextMessage(msg_s);

    if (m_debug)
        qDebug() << "Connected: " << pSocket->localAddress();

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

void Server::setRecordingStatus(bool value, qint64 time)
{
    if (m_isRecording == value)
        return;

    m_isRecording = value;
    if (value)
        m_recStartTime = time;

    QJsonObject msg;
    msg["event"] = "recordingStatusChanged";
    msg["value"] = value;
    msg["time"] = QString::number(time);
    QString msg_s = QJsonDocument(msg).toJson(QJsonDocument::Compact);

    for (auto *client : m_clients)
        client->sendTextMessage(msg_s);
}

void Server::setTrackingStatus(bool value)
{
    if (m_isTracking == value)
        return;

    m_isTracking = value;

    QJsonObject msg;
    msg["event"] = "trackingStatusChanged";
    msg["value"] = value;
    QString msg_s = QJsonDocument(msg).toJson(QJsonDocument::Compact);

    for (auto *client : m_clients)
        client->sendTextMessage(msg_s);
}

void Server::handleCommand(QJsonDocument doc)
{
    QJsonObject jObject = doc.object();
    QVariantMap mainMap = jObject.toVariantMap();
    QString protocol = mainMap.value("jsonrpc").toString();
    QString command = mainMap.value("method").toString();
    QVariantMap paramsMap = mainMap.value("params").toMap();
    if (command == "rotate") {
        emit rotateCmdReceived(paramsMap);
    } else if (command == "setRecording") {
        emit setRecordingCmdReceived(paramsMap.value("value").toBool());
    } else if (command == "setTracking") {
        emit setTrackingCmdReceived(paramsMap.value("value").toBool());
        //setTrackingStatus(paramsMap.value("value").toBool());
    }
}
