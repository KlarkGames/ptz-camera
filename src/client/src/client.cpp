#include "client.h"
#include <QJsonObject>
#include <QHash>

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

void Client::setIsRecording(bool value)
{
    if (m_isRecording == value)
        return;
    m_isRecording = value;
    emit isRecordingChanged();
}

void Client::setIsTracking(bool value)
{
    if (m_isTracking == value)
        return;
    m_isTracking = value;
    emit isTrackingChanged();
}

QTime Client::getRecElapsedTimeMSecs()
{
    if (!m_isRecording)
        return QTime(0, 0);

    return QTime::fromMSecsSinceStartOfDay(QDateTime::currentMSecsSinceEpoch() - m_recStartTime);
}

void Client::handleReceivedMessage(QStringView msg)
{
    qDebug() << "Received:" << msg;

    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
    QJsonObject obj = doc.object();

    if (obj["event"].toString() == "init") {
        setIsRecording(obj["isRecording"].toBool());
        if (obj.contains("recStartTime"))
            m_recStartTime = obj["recStartTime"].toString().toLongLong();
        setIsTracking(obj["isTracking"].toBool());
    }
    else if (obj["event"].toString() == "recordingStatusChanged") {
        setIsRecording(obj["value"].toBool());
        if (m_isRecording)
            m_recStartTime = obj["time"].toString().toLongLong();
    }
    else if (obj["event"].toString() == "trackingStatusChanged") {
        setIsTracking(obj["value"].toBool());
    }
}

bool Client::isRecording()
{
    return m_isRecording;
}

bool Client::isTracking()
{
    return m_isTracking;
}

TrackingObjectModel* Client::trackingObjectModel()
{
    return &m_trackingObjectModel;
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

bool Client::sendSetRecordingCmd(bool value)
{
    QJsonObject cmd, params;

    params["value"] = value;

    cmd["method"] = "setRecording";
    cmd["params"] = params;

    QString cmdMsg = QJsonDocument(cmd).toJson(QJsonDocument::Compact);
    return m_socket.sendTextMessage(cmdMsg) >= cmdMsg.size();
}

bool Client::sendSetTrackingCmd(bool value)
{
    QJsonObject cmd, params;

    params["value"] = value;

    cmd["method"] = "setTracking";
    cmd["params"] = params;

    QString cmdMsg = QJsonDocument(cmd).toJson(QJsonDocument::Compact);
    return m_socket.sendTextMessage(cmdMsg) >= cmdMsg.size();
}

Client::~Client()
{
    closeConnection();
}
