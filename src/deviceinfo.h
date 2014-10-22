#pragma once

#include <QObject>

class DeviceInfo : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString friendlyName MEMBER friendlyName                      NOTIFY friendlyNameChanged)
    Q_PROPERTY(QString os           READ   os           WRITE setOs          NOTIFY osChanged)
    Q_PROPERTY(QString radio        READ   radio        WRITE setRadio       NOTIFY radioChanged)
    Q_PROPERTY(int     battery      MEMBER battery      WRITE setBattery     NOTIFY batteryChanged)
    Q_PROPERTY(QString name         MEMBER name         WRITE setName        NOTIFY nameChanged)
    Q_PROPERTY(QString pin          MEMBER pin          WRITE setPin         NOTIFY pinChanged)
    Q_PROPERTY(QString hw           MEMBER hw           WRITE setHw          NOTIFY hwChanged)
    Q_PROPERTY(int     hwFamily     MEMBER hwFamily                          NOTIFY hwFamilyChanged())
    Q_PROPERTY(QString bbid         MEMBER bbid         WRITE setBbid        NOTIFY bbidChanged)
public:
    DeviceInfo()
        : QObject()
        , battery(-1)
    {
    }
    QString friendlyName;
    QString os;
    QString radio;
    int battery;
    QString name;
    QString pin;
    QString hw;
    QString hwFamily;
    QString bbid;

signals:
    void friendlyNameChanged();
    void osChanged();
    void radioChanged();
    void batteryChanged();
    void nameChanged();
    void pinChanged();
    void hwChanged();
    void hwFamilyChanged();
    void bbidChanged();

};
Q_DECLARE_METATYPE(DeviceInfo* );
