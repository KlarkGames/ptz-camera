#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QtQml/qqmlregistration.h>
#include <QtWebSockets>

class Client : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QUrl streamSource READ streamSource NOTIFY streamSourceChanged)
public:
    enum RotateDirection{
        DIR_LEFT = 1, DIR_RIGHT = 2, DIR_UP = 3, DIR_DOWN = 4
    };
    Q_ENUM(RotateDirection)

    explicit Client(QObject *parent = nullptr);
    QUrl streamSource();
    Q_INVOKABLE void connectToHost(QString addr);
    Q_INVOKABLE void closeConnection();
    Q_INVOKABLE bool sendRotateCmd(RotateDirection dir, int steps = 1);
    virtual ~Client();

private slots:
    void afterConnected();
    void handleReceivedMessage(QStringView msg);

signals:
    void connected();
    void disconnected();
    void streamSourceChanged();

private:
    QWebSocket m_socket;
    QUrl m_streamSource;
};

#endif // CLIENT_H
