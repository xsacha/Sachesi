#ifndef BLITZINFO_H
#define BLITZINFO_H

#include <QObject>

enum BarType {
    NotInstallableType = 0,
    AppType,
    RadioType,
    OSType,
};

// Blitz means it has more than one OS or Radio.
// In this situation we need to work out which one is intended instead of asking about every single one.
// If there is only one good OS and Radio, the intention is clear.
class BlitzInfo: public QObject {
    Q_OBJECT
public:
    BlitzInfo(QList<QString> filenames, QString deviceOS, QString deviceRadio);
    void blitzCheck(QString name);
    bool isSafe() { return (osIsSafe && radioIsSafe); }
    bool isBlitz() { return (osCount > 1 || radioCount > 1); }
    bool osIsSafe, radioIsSafe;
    int radioCount, osCount;
private:
    QString _deviceOS, _deviceRadio;
};

#endif // BLITZINFO_H
