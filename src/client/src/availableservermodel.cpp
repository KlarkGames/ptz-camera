#include "availableservermodel.h"
#include "common_defs.h"

AvailableServerModel::AvailableServerModel(QObject *parent)
    : QAbstractListModel{parent}
{
    QObject::connect(&m_socket, &QIODevice::readyRead, this, &AvailableServerModel::handlePacket);
    if (!m_socket.bind(QHostAddress::Broadcast, BROADCAST_PORT, QAbstractSocket::ShareAddress)) {
        qDebug() << QString("Failed to bind to port %1").arg(BROADCAST_PORT);
    }

    QObject::connect(&m_timer, &QTimer::timeout, this, &AvailableServerModel::filterOld);
    m_timer.start(UPDATE_INTERVAL_MS);
}

int AvailableServerModel::rowCount(const QModelIndex& parent) const
{
    return m_list.size();
}

QVariant AvailableServerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case AddressRole:
        return m_list.at(index.row()).address;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> AvailableServerModel::roleNames() const
{
    static const QHash<int, QByteArray> ROLE_NAMES {
        {AddressRole, "address"}
    };

    return ROLE_NAMES;
}

void AvailableServerModel::handlePacket()
{
    while (m_socket.hasPendingDatagrams())
    {
        QByteArray datagram;

        datagram.resize(int(m_socket.pendingDatagramSize()));
        m_socket.readDatagram(datagram.data(), datagram.size());

        QString address = QString::fromUtf8(datagram);
        auto now = QDateTime::currentMSecsSinceEpoch();
        QPersistentModelIndex index;
        int row;

        if (m_hash.contains(address) && (index = m_hash.value(address)).isValid()) {
            row = index.row();
            m_list[row].timestamp = now;
            emit dataChanged(index, index);

            continue;
        }

        Data data;
        data.address = address;
        data.timestamp = now;

        row = m_list.size();
        beginInsertRows(QModelIndex(), row, row);
        m_list.push_back(data);
        m_hash.insert(address, this->index(row));
        endInsertRows();
    }
}

void AvailableServerModel::filterOld()
{
    auto now = QDateTime::currentMSecsSinceEpoch();

    for (int i = 0; i < m_list.size(); ) {
        const Data &data = m_list.at(i);

        if (now - data.timestamp >= TIMEOUT_MS) {
            beginRemoveRows(QModelIndex(), i, i);
            m_hash.remove(data.address);
            m_list.remove(i);
            endRemoveRows();
            continue;
        }

        ++i;
    }
}
