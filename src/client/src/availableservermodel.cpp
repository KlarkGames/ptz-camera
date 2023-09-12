#include "availableservermodel.h"

AvailableServerModel::AvailableServerModel(QObject *parent)
    : QAbstractListModel{parent}
{

}

int AvailableServerModel::rowCount(const QModelIndex& parent) const
{
    return 0;
}

QVariant AvailableServerModel::data(const QModelIndex& index, int role) const
{
    switch (role) {
    case AddressRole:
        // TODO
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
