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
        blitzCheck(barFile);
}

void BlitzInfo::blitzCheck(QString name)
{
    BarType fileType = NotInstallableType;
    // Check if it's a 'hidden' file as we use these for temporary file downloads.
    if (QFileInfo(name).fileName().startsWith('.'))
        return;

    QuaZipFile manifest(name, "META-INF/MANIFEST.MF", QuaZip::csSensitive);
    if (!manifest.open(QIODevice::ReadOnly))
        return;
    QString appName, type;
    while (!manifest.atEnd()) {
        QByteArray newLine = manifest.readLine();
        if (newLine.startsWith("Package-Name:")  || newLine.startsWith("Patch-Package-Name:")) {
            appName = newLine.split(':').last().simplified();
            if (newLine.startsWith("Patch") && type == "system") {
                if (appName.contains("radio"))
                    fileType = RadioType;
                else
                    fileType = OSType;
            }
        }
        else if (newLine.startsWith("Package-Type:")  || newLine.startsWith("Patch-Package-Type:")) {
            type = newLine.split(':').last().simplified();
            if (type == "system" && fileType == NotInstallableType)
                fileType = OSType;
            else if (type != "patch")
                break;
        }
        else if (newLine.startsWith("System-Type:")) {
            if (newLine.split(':').last().simplified() == "radio")
                fileType = RadioType;
            break;
        }
    }
    if (fileType != OSType && fileType != RadioType)
        return;

    if (fileType == OSType) {
        osCount++;
        QString installableOS = appName.split("os.").last().remove(".desktop").replace("verizon", "factory");
        if (_deviceOS == "" || installableOS == _deviceOS || (installableOS.contains("8974") && _deviceOS.contains("8974"))) {
            osIsSafe = true;
        }
    } else if (fileType == RadioType) {
        radioCount++;
        QString installableRadio = appName.split("radio.").last().remove(".omadm");
        if (_deviceRadio == "" || installableRadio == _deviceRadio) {
            radioIsSafe = true;
        }
    }
    return;
}
