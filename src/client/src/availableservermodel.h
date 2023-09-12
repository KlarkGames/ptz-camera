#ifndef AVAILABLESERVERMODEL_H
#define AVAILABLESERVERMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <QQmlEngine>
//#include <QtQml/qqmlregistration.h>

class AvailableServerModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    enum RoleNames {
        AddressRole = Qt::UserRole
    };
    Q_ENUM(RoleNames)

    explicit AvailableServerModel(QObject *parent = nullptr);
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual QHash<int, QByteArray> roleNames() const;

    //virtual ~AvailableServerModel();
};

#endif // AVAILABLESERVERMODEL_H
