#pragma once

#include <QObject>
#include <QDateTime>

class DeviceInfo : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString friendlyName   MEMBER friendlyName                         NOTIFY friendlyNameChanged)
    Q_PROPERTY(QString os             MEMBER os            WRITE setOs            NOTIFY osChanged)
    Q_PROPERTY(QString radio          MEMBER radio         WRITE setRadio         NOTIFY radioChanged)
    Q_PROPERTY(int     battery        MEMBER battery       WRITE setBattery       NOTIFY batteryChanged)
    Q_PROPERTY(QString name           MEMBER name          WRITE setName          NOTIFY nameChanged)
    Q_PROPERTY(QString pin            MEMBER pin           WRITE setPin           NOTIFY pinChanged)
    Q_PROPERTY(QString hw             MEMBER hw            WRITE setHw            NOTIFY hwChanged)
    Q_PROPERTY(int     hwFamily       MEMBER hwFamily      WRITE setHwFamily      NOTIFY hwFamilyChanged())
    Q_PROPERTY(QString bbid           MEMBER bbid          WRITE setBbid          NOTIFY bbidChanged)
    Q_PROPERTY(int     protocol       MEMBER protocol      WRITE setProtocol      NOTIFY protocolChanged())
    Q_PROPERTY(bool    devMode        MEMBER devMode       WRITE setDevMode       NOTIFY devModeChanged())
    Q_PROPERTY(bool    setupComplete  MEMBER setupComplete WRITE setSetupComplete NOTIFY setupCompleteChanged())
    Q_PROPERTY(QString restrictions   MEMBER restrictions  WRITE setRestrictions  NOTIFY restrictionsChanged)
    Q_PROPERTY(QString refurbDate     MEMBER refurbDate    WRITE setRefurbDate    NOTIFY refurbDateChanged)
public:
    DeviceInfo()
        : QObject()
        , radio("N/A")
        , battery(-1)
        , hwFamily(0)
        , protocol(1)
        , devMode(false)
    {
    }
    QString friendlyName;
    QString os;
    QString radio;
    int battery;
    QString name;
    QString pin;
    QString hw;
    int hwFamily;
    QString bbid;
    int protocol;
    bool devMode;
    bool setupComplete;
    QString restrictions;
    QString refurbDate;
    void setFriendlyName(const QString &input) { friendlyName = input; emit friendlyNameChanged(); }
    void setOs(const QString &input) { os = input; emit osChanged(); }
    void setRadio(const QString &input) { radio = input; emit radioChanged(); }
    void setBattery(const int &input) { battery = input; emit batteryChanged(); }
    void setName(const QString &input) { name = input; emit nameChanged(); }
    void setPin(const QString &input) { pin = input; emit pinChanged(); }
    void setHw(const QString &input) { hw = input; emit hwChanged(); }
    void setHwFamily(const int &input) { hwFamily = input; emit hwFamilyChanged(); }
    void setBbid(const QString &input) { bbid = input; emit bbidChanged(); }
    void setProtocol(const int &input) { protocol = input; emit protocolChanged(); }
    void setDevMode(const bool &input) { devMode = input; emit devModeChanged(); }
    void setSetupComplete(const bool &input) { setupComplete = input; emit setupCompleteChanged(); }
    void setRestrictions(const QString &input) { restrictions = input; emit restrictionsChanged(); }
    void setRefurbDate(const QString &input) {
        if (input.toInt() == 0)
            refurbDate = "";
        else
            refurbDate = QDateTime::fromTime_t(input.toInt()).toString();
        emit refurbDateChanged();
    }

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
    void protocolChanged();
    void devModeChanged();
    void setupCompleteChanged();
    void restrictionsChanged();
    void refurbDateChanged();

};
Q_DECLARE_METATYPE(DeviceInfo* );
