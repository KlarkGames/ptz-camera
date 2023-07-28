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

    QUrl streamSource = url;
    streamSource.setScheme("tcp");
    streamSource.setPort(5000);

    QObject::connect(
        m_player, &QMediaPlayer::hasVideoChanged,
        m_player, &QMediaPlayer::play,
        Qt::SingleShotConnection
    );

    QThreadPool::globalInstance()->start(QRunnable::create([this, streamSource]() {
        m_player->setSource(streamSource);
    }));

    closeConnection();
    m_socket.open(url);
}

void Client::closeConnection()
{
    return m_socket.close();
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

void Client::handleReceivedMessage(QStringView msg_s)
{
    qDebug() << "Received:" << msg_s;

    QJsonDocument doc = QJsonDocument::fromJson(msg_s.toUtf8());
    QJsonObject msg = doc.object();

    QString method = msg.value("method").toString();
    QJsonObject params = msg.value("params").toObject();

    if (method == "init") {
        setIsRecording(params.value("isRecording").toBool());
        if (params.contains("recStartTime"))
            m_recStartTime = params.value("recStartTime").toString().toLongLong();
        setIsTracking(params.value("isTracking").toBool());
    }
    else if (method == "updRecording") {
        setIsRecording(params.value("value").toBool());
        if (m_isRecording)
            m_recStartTime = params.value("time").toString().toLongLong();
    }
    else if (method == "updTracking") {
        setIsTracking(params.value("value").toBool());
    }
    else if (method == "updTrackingObjects") {
        QHash<int, TrackingObjectModel::Data> objects;
        QJsonArray objArray = params["objects"].toArray();

        for (QJsonValueConstRef val : objArray) {
            QJsonObject obj = val.toObject();
            TrackingObjectModel::Data data;

            data.objectId = obj.value("objectId").toInt();
            data.className = obj.value("className").toString();

            QJsonObject rect = obj.value("rect").toObject();
            data.rect = QRect(
                rect.value("x").toInt(),
                rect.value("y").toInt(),
                rect.value("width").toInt(),
                rect.value("height").toInt()
            );

            objects.insert(data.objectId, data);
        }

        m_trackingObjectModel.updateData(objects);
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

bool Client::sendRotateCmd(RotateDirection dir, bool launch)
{
    static const QHash<RotateDirection, QString> DIRS{
        {DIR_LEFT, "left"},
        {DIR_RIGHT, "right"},
        {DIR_UP, "up"},
        {DIR_DOWN, "down"}
    };

    QJsonObject msg, params;

    params["direction"] = DIRS.value(dir);
    params["command"] = launch ? "launch" : "stop";

    msg["jsonrpc"] = "2.0";
    msg["method"] = "rotate";
    msg["params"] = params;

    QString msg_s = QJsonDocument(msg).toJson(QJsonDocument::Compact);
    return m_socket.sendTextMessage(msg_s) >= msg_s.size();
}

bool Client::sendSetRecordingCmd(bool value)
{
    QJsonObject msg, params;

    params["value"] = value;

    msg["jsonrpc"] = "2.0";
    msg["method"] = "setRecording";
    msg["params"] = params;

    QString msg_s = QJsonDocument(msg).toJson(QJsonDocument::Compact);
    return m_socket.sendTextMessage(msg_s) >= msg_s.size();
}

bool Client::sendSetTrackingCmd(bool value)
{
    QJsonObject msg, params;

    params["value"] = value;

    msg["jsonrpc"] = "2.0";
    msg["method"] = "setTracking";
    msg["params"] = params;

    QString msg_s = QJsonDocument(msg).toJson(QJsonDocument::Compact);
    return m_socket.sendTextMessage(msg_s) >= msg_s.size();
}

QMediaPlayer* Client::mediaPlayer()
{
    return m_player;
}

void Client::setMediaPlayer(QMediaPlayer *mediaPlayer)
{
    if (m_player == mediaPlayer)
        return;

    m_player = mediaPlayer;
    emit mediaPlayerChanged();
}

Client::~Client()
{
    closeConnection();
}
