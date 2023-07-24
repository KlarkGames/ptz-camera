#include "trackingobjectmodel.h"

TrackingObjectModel::TrackingObjectModel(QObject *parent)
    : QAbstractListModel{parent}
{

}

int TrackingObjectModel::rowCount(const QModelIndex& parent) const
{
    return m_ids.count();
}

QVariant TrackingObjectModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int objectId = m_ids.value(index.row());

    switch (role) {
    case IdRole:
        return objectId;
        break;
    case ClassRole:
        return m_data[objectId].className;
        break;
    case RectRole:
        return m_data[objectId].rect;
        break;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> TrackingObjectModel::roleNames() const
{
    static const QHash<int, QByteArray> roleNames{
        {IdRole, "objectId"},
        {ClassRole, "className"},
        {RectRole, "rect"}
    };
    return roleNames;
}

void TrackingObjectModel::updateData(QHash<int, Data> objects)
{
    for (int objectId : m_ids)
        if (!objects.contains(objectId))
            remove(objectId);

    for (const Data& data : objects)
        insert(data);
}

void TrackingObjectModel::insert(const Data& data)
{
    if (m_data.contains(data.objectId)) {
        Data &obj = m_data[data.objectId];
        int index = obj.index;
        obj = data;
        obj.index = index;
        emit dataChanged(this->index(index), this->index(index));
    } else {
        int index = m_ids.size();
        beginInsertRows(QModelIndex(), index, index);
        m_ids.push_back(data.objectId);
        m_data.insert(data.objectId, data);
        m_data[data.objectId].index = index;
        endInsertRows();
    }
}

void TrackingObjectModel::remove(int objectId)
{
    if (!m_data.contains(objectId))
        return;

    int index = m_data[objectId].index;
    beginRemoveRows(QModelIndex(), index, index);
    m_ids.remove(index);
    m_data.remove(objectId);
    endRemoveRows();
}
