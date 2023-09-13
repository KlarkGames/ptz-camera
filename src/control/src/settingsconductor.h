#ifndef SETTINGSCONDUCTOR_H
#define SETTINGSCONDUCTOR_H

#include <QJsonObject>

class SettingsConductor
{
    virtual void setSettings(QJsonObject params) = 0;
    virtual QJsonObject getSettings() = 0;
};

#endif // SETTINGSCONDUCTOR_H
