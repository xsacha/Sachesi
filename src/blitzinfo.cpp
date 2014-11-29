#include "blitzinfo.h"
#include <QFileInfo>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

BlitzInfo::BlitzInfo(QList<QString> filenames, QString deviceOS, QString deviceRadio)
    : QObject()
    , osIsSafe(false)
    , radioIsSafe(false)
    , radioCount(0)
    , osCount(0)
    , _deviceOS(deviceOS)
    , _deviceRadio(deviceRadio)
{
    foreach(QString barFile, filenames)
    {
        BarInfo info = blitzCheck(barFile);
        if (info.type == OSType) {
            osCount++;
            if (info.name == "GOOD")
                osIsSafe = true;
        } else if (info.type == RadioType) {
            radioCount++;
            if (info.name == "GOOD")
                radioIsSafe = true;
        }
    }
}

BarInfo BlitzInfo::blitzCheck(QString name)
{
    BarInfo barInfo = {name, "", "", NotInstallableType};
    // Check if it's a 'hidden' file as we use these for temporary file downloads.
    if (QFileInfo(name).fileName().startsWith('.'))
        return barInfo;

    QuaZipFile manifest(name, "META-INF/MANIFEST.MF", QuaZip::csSensitive);
    if (!manifest.open(QIODevice::ReadOnly))
        return barInfo;
    QString appName, type;
    while (!manifest.atEnd()) {
        QByteArray newLine = manifest.readLine();
        if (newLine.startsWith("Package-Name:")  || newLine.startsWith("Patch-Package-Name:")) {
            appName = newLine.split(':').last().simplified();
            if (newLine.startsWith("Patch") && type == "system") {
                if (appName.contains("radio"))
                    barInfo.type = RadioType;
                else
                    barInfo.type = OSType;
            }
        }
        else if (newLine.startsWith("Package-Type:")  || newLine.startsWith("Patch-Package-Type:")) {
            type = newLine.split(':').last().simplified();
            if (type == "system" && barInfo.type == NotInstallableType)
                barInfo.type = OSType;
            else if (type != "patch")
                break;
        }
        else if (newLine.startsWith("System-Type:")) {
            if (newLine.split(':').last().simplified() == "radio")
                barInfo.type = RadioType;
            break;
        }
    }
    if (barInfo.type != OSType && barInfo.type != RadioType)
        return barInfo;

    barInfo.name = "GOOD";
    if (barInfo.type == OSType) {
        QString installableOS = appName.split("os.").last().remove(".desktop").replace("verizon", "factory");
        if (_deviceOS != "" && installableOS != _deviceOS && !(installableOS.contains("8974") && _deviceOS.contains("8974"))) {
            barInfo.name = "BAD";
        }
    } else if (barInfo.type == RadioType) {
        QString installableRadio = appName.split("radio.").last().remove(".omadm");
        if (_deviceRadio != "" && installableRadio != _deviceRadio) {
            barInfo.name = "BAD";
        }
    }
    return barInfo;
}
