#include "client.h"
#include <QJsonObject>

Client::Client(QObject *parent)
    : QObject{parent}
    , m_socket{}
{
    connect(&m_socket, &QWebSocket::connected, this, &Client::afterConnected);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &Client::handleReceivedMessage);
    connect(&m_socket, &QWebSocket::disconnected, this, [this](){ emit disconnected(); });
}

void Client::connectToHost(QString addr)
{
    QUrl url = QUrl::fromUserInput(addr);
    url.setScheme("ws");
    if (url.port() < 0)
        url.setPort(41419);

    m_streamSource = url;
    m_streamSource.setScheme("tcp");
    m_streamSource.setPort(5000);
    emit streamSourceChanged();

    closeConnection();
    m_socket.open(url);
}

void Client::closeConnection()
{
    return m_socket.close();
}

QUrl Client::streamSource()
{
    return m_streamSource;
}

void Client::afterConnected()
{
    emit connected();
}

void Client::handleReceivedMessage(QStringView msg)
{
    qDebug() << "Received:" << msg;
}

bool Client::sendRotateCmd(RotateDirection dir, int steps)
{
    QJsonObject cmd, params;
    switch (dir) {
    case DIR_LEFT:
    case DIR_RIGHT:
        params["axis"] = "x";
        break;
    case DIR_UP:
    case DIR_DOWN:
        params["axis"] = "y";
        break;
    default:
        return false;
    }

    params["direction"] = (int)dir;
    params["steps"] = steps;

    cmd["method"] = "rotate";
    cmd["params"] = params;

    QString cmdMsg = QJsonDocument(cmd).toJson(QJsonDocument::Compact);
    return m_socket.sendTextMessage(cmdMsg) >= cmdMsg.size();
}

Client::~Client()
{
    closeConnection();
}
