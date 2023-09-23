#ifndef AVAILABLESERVERMODEL_H
#define AVAILABLESERVERMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <QTimer>
#include <QUdpSocket>
#include <QList>
#include <QHash>
#include <QDateTime>
#include <QQmlEngine>

class AvailableServerModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    enum RoleNames {
        AddressRole = Qt::UserRole,
        HostnameRole
    };
    Q_ENUM(RoleNames)

    explicit AvailableServerModel(QObject *parent = nullptr);
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual QHash<int, QByteArray> roleNames() const;

private slots:
    void filterOld();
    void handlePacket();

private:
    const int TIMEOUT_MS = 1500;
    const int UPDATE_INTERVAL_MS = 300;

    struct Data {
        qint64 timestamp;
        QString address;
        QString hostname;
    };

    QTimer m_timer;
    QUdpSocket m_socket;
    QList<Data> m_list;
    QHash<QString, QPersistentModelIndex> m_hash;
};

#endif // AVAILABLESERVERMODEL_H
