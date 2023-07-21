#ifndef TRACKINGOBJECTMODEL_H
#define TRACKINGOBJECTMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <QQmlEngine>
#include <QRect>

class TrackingObjectModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    struct Data {
        union { int index, objectId; };
        QString className;
        QRect rect;
    };

    enum RoleNames {
        IdRole = Qt::UserRole,
        ClassRole,
        RectRole
    };
    Q_ENUM(RoleNames)

    explicit TrackingObjectModel(QObject *parent = nullptr);
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual QHash<int, QByteArray> roleNames() const;
    void updateData(QHash<int, Data> objects);

private:
    QList<int> m_ids;
    QHash<int, Data> m_data;
    void insert(const Data& data);
    void remove(int objectId);
};

#endif // TRACKINGOBJECTMODEL_H
