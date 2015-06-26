// Copyright (C) 2014 Sacha Refshauge

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 3.0 for more details.

// A copy of the GPL 3.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official GIT repository and contact information can be found at
// http://github.com/xsacha/Sachesi

#define DEBUG_LOG 0

#include "installer.h"
#include "ports.h"
#include <QAbstractListModel>
#include <QDebug>
#include <QMessageBox>
#include <QNetworkInterface>

InstallNet::InstallNet( QObject* parent) : QObject(parent),
    device(nullptr), manager(nullptr), reply(nullptr), cookieJar(nullptr),
    _wrongPass(false), _loginBlock(false),
    _state(0), _dlBytes(0), _dlTotal(0), _dgProgress(-1), _curDGProgress(-1),
    _completed(false), _extractInstallZip(false), _allowDowngrades(false), _installing(false), _restoring(false), _backing(false),
    _hadPassword(true), currentBackupZip(nullptr), _zipFile(nullptr)
{
#ifdef _MSC_VER
    WSAStartup(MAKEWORD(2,0), &wsadata);
#endif
    connectTimer = new QTimer();
    connectTimer->setInterval(3000);
    connectTimer->start();
    connect(connectTimer, SIGNAL(timeout()), this, SLOT(login()));
    QSettings settings("Qtness","Sachesi");
    connect(&_back, SIGNAL(curModeChanged()), this, SIGNAL(backStatusChanged()));
    connect(&_back, SIGNAL(curSizeChanged()), this, SIGNAL(backCurProgressChanged()));
    connect(&_back, SIGNAL(numMethodsChanged()), this, SIGNAL(backMethodsChanged()));

    logFile = new QTemporaryFile();
    logFile->open(); // This will autoclose and autoremove by default when ~InstallNet

    QByteArray hashedPass = settings.value("pass", "").toByteArray();

    if (hashedPass.isEmpty()) {
        _password = "";
    } else {
        int passSize = QByteArray::fromBase64(hashedPass.left(4))[0];
        hashedPass = QByteArray::fromBase64(hashedPass.mid(4));

        char * decPass = new char[passSize+1];
        for (int i = 0; i < passSize; i++) {
            decPass[i] = hashedPass[i] ^ ((0x40 + 5 * i - passSize) % 127);
        }
        decPass[passSize] = 0;
        _password = QString(decPass);
        delete decPass;
    }
    emit newPassword(_password);
    login();
}
InstallNet::~InstallNet()
{
#ifdef _MSC_VER
    WSACleanup();
#endif
}

QNetworkRequest InstallNet::setData(QString page, QString contentType) {
    QNetworkRequest request = QNetworkRequest();
    request.setUrl(QUrl("https://" + _ip + "/cgi-bin/" + page));
    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "QNXWebClient/1.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/" + contentType);
    return request;
}

QNetworkReply* InstallNet::postQuery(QString page, QString contentType, const QUrlQuery& query) {
    reply = manager->post(setData(page, contentType), query.encodedQuery());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(restoreError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished()), this, SLOT(restoreReply()));
    return reply;
}

QNetworkReply* InstallNet::getQuery(QString page, QString contentType) {
    reply = manager->get(setData(page, contentType));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(restoreError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished()), this, SLOT(restoreReply()));
    return reply;
}

void InstallNet::scanProps()
{
    if (checkLogin())
    {
        QUrlQuery postData;
        postData.addQueryItem("Get Dynamic Properties","Get Dynamic Properties");
        postQuery("dynamicProperties.cgi", "x-www-form-urlencoded", postData);
    }
}

BarInfo InstallNet::checkInstallableInfo(QString name, bool blitz)
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
        if (newLine.startsWith("Package-Name:") || newLine.startsWith("Patch-Package-Name:")) {
            appName = newLine.split(':').last().simplified();
            if (newLine.startsWith("Patch") && type == "system") {
                if (appName.contains("radio"))
                    barInfo.type = RadioType;
                else
                    barInfo.type = OSType;
            }
        }
        else if (newLine.startsWith("Package-Type:") || newLine.startsWith("Patch-Package-Type:")) {
            type = newLine.split(':').last().simplified();
            if (type == "system" && barInfo.type == NotInstallableType)
                barInfo.type = OSType;
        }
        else if (newLine.startsWith("Package-Version:")) {
            barInfo.version = newLine.split(':').last().simplified();
        }
        else if (newLine.startsWith("Package-Id:")) {
            barInfo.packageid = newLine.split(':').last().simplified();
        }
        else if (newLine.startsWith("System-Type:")) {
            if (newLine.split(':').last().simplified() == "radio")
                barInfo.type = RadioType;
        } else if (newLine.startsWith("Archive-Asset-Name:")) {
            // A sign we need to get out of here when it is listing assets
            break;
        }
    }

    // Check if we are about to make a huge mistake!
    if (barInfo.type == OSType) {
        if (!_allowDowngrades && isVersionNewer(device->os, barInfo.version, true)) {
            setNewLine(QString("OS %1 skipped. Newer version is installed(%2)").arg(barInfo.version).arg(device->os));
            barInfo.type = NotInstallableType;
            return barInfo;
        }

        QString installableOS = appName.split("os.").last().remove(".desktop").replace("verizon", "factory");
        if (_knownConnectedOSType != "" && installableOS != _knownConnectedOSType && !(installableOS.contains("8974") && _knownConnectedOSType.contains("8974"))) {
            if (blitz) {
                barInfo.type = NotInstallableType;
                return barInfo;
            } else {
                int choice = QMessageBox::critical(nullptr, "WARNING", "The OS file you have selected to install is for a different device!\nOS Type: " + installableOS + "\nYour Device: " + _knownConnectedOSType, "Ignore Warning [Stupid]", "Skip OS", "Cancel Install", 2);
                if (choice == 1) {
                    barInfo.type = NotInstallableType;
                    return barInfo;
                }
                if (choice == 2) {
                    barInfo.name = "EXIT";
                    return barInfo;
                }
            }
        }
    } else if (barInfo.type == RadioType) {
        if (!_allowDowngrades && isVersionNewer(device->radio, barInfo.version, true)) {
            setNewLine(QString("Radio %1 skipped. Newer version is installed(%2)").arg(barInfo.version).arg(device->radio));
            barInfo.type = NotInstallableType;
            return barInfo;
        }
        QString installableRadio = appName.split("radio.").last().remove(".omadm");
        if (_knownConnectedRadioType != "" && installableRadio != _knownConnectedRadioType) {
            if (blitz) {
                barInfo.type = NotInstallableType;
                return barInfo;
            } else {
                int choice = QMessageBox::critical(nullptr, "WARNING", "The Radio file you have selected to install is for a different device!\nRadio Type: " + installableRadio + "\nYour Device: " + _knownConnectedRadioType, "Ignore Warning [Stupid]", "Skip Radio", "Cancel Install", 2);
                if (choice == 1) {
                    barInfo.type = NotInstallableType;
                    return barInfo;
                }
                if (choice == 2) {
                    barInfo.name = "EXIT";
                    return barInfo;
                }
            }
        }
    }

    // Check if we are upgrading firmware or just installing apps.
    if (type == "system") {
        // Only if installing
        setNewLine(QString("<b>Installing ") + (barInfo.type == OSType ? "OS: " : "Radio: " ) + barInfo.version + "</b>");
        setFirmwareUpdate(true);
    } else if (!type.isEmpty())
        barInfo.type = AppType;

    manifest.close();
    return barInfo;
}

void InstallNet::install(QList<QUrl> files)
{
    _installInfo.clear();
    setFirmwareUpdate(false);
    QList<QString> filenames;

    // Zip install
    if (files.count() == 1 && files.first().toLocalFile().endsWith(".zip")) {
        // First, ensure it is a real zip
        QFile testZip(files.first().toLocalFile());
        testZip.open(QIODevice::ReadOnly);
        if (testZip.read(2).toHex() != "504b") {
            QMessageBox::information(nullptr, "Error", "The selected .zip file is, in fact, not a zip file.");
            return;
        }
        testZip.close();
        // A collection of bars
        _extractInstallZip = true; emit extractInstallZipChanged();
        QuaZip zip(files.first().toLocalFile());
        zip.open(QuaZip::mdUnzip);

        QuaZipFile file(&zip);
        QFileInfo zipInfo(zip.getZipName());

        QString baseName = zipInfo.absolutePath() + "/" + zipInfo.completeBaseName();

        if (!QDir(baseName).mkpath("."))
            QMessageBox::information(nullptr, "Error", "Was unable to extract the zip container.");

        for(bool f=zip.goToFirstFile(); f; f=zip.goToNextFile()) {
            QString thisFile = baseName + "/" + zip.getCurrentFileName().split('/').last();
            if (QFile::exists(thisFile)) {
                QuaZipFileInfo info;
                zip.getCurrentFileInfo(&info);
                if (QFile(thisFile).size() == info.uncompressedSize) {
                    filenames.append(thisFile);
                    continue;
                } else {
                    QFile(thisFile).remove();
                }
            }
            if (!file.open(QIODevice::ReadOnly))
                continue;
            // Check we have a zip
            if (file.read(2).toHex() == "504b") {
                QFile writeFile(thisFile);
                if (!writeFile.open(QIODevice::WriteOnly)) {
                    file.close();
                    continue;
                }
                writeFile.write(QByteArray::fromHex("504b"));
                while (!file.atEnd()) {
                    qApp->processEvents();
                    writeFile.write(file.read(8192000));
                }
                filenames.append(thisFile);
            }
            file.close();
        }
        _extractInstallZip = false; emit extractInstallZipChanged();
    } else {

        // Grab file names (first pass)
        foreach(QUrl url, files)
        {
            if (!url.isLocalFile())
                continue;
            QString name = url.toLocalFile();
            if (QFileInfo(name).isDir())
            {
                QStringList barFiles = QDir(name).entryList(QStringList("*.bar"));
                foreach (QString barFile, barFiles) {
                    filenames.append(name + "/" + barFile);
                }
            } else {
                filenames.append(name);
            }
        }
    }

    // Detect Blitz files (second pass)
    BlitzInfo blitz(filenames, _knownConnectedOSType, _knownConnectedRadioType);

    if (blitz.isBlitz()) {
        setNewLine(QString("%1 Blitz detected. %2 OSes and %3 Radios")
                   .arg(blitz.isSafe() ? "Safe " : "Unsafe")
                   .arg(blitz.osCount)
                   .arg(blitz.radioCount));
        if (_knownConnectedRadioType == "" && blitz.radioCount > 1) {
            QMessageBox::critical(nullptr, "Error", "Your device is reporting no Radio. The blitz install is unable to detect the correct Radio for your system and cannot continue.");
            return;
        } else if (_knownConnectedOSType == "" && blitz.osCount > 1) {
            QMessageBox::critical(nullptr, "Error", "Your device is reporting no OS. The blitz install is unable to detect the correct OS for your system and cannot continue.");
            return;
        }
        if (!blitz.isSafe()) {
            QMessageBox::critical(nullptr, "Error", QString("The blitz file does not contain compatible firmware.") + (!blitz.osIsSafe ? "\nNo compatible OS" : "") + (!blitz.radioIsSafe ? "\nNo compatible Radio" : ""));
            return;
        }
    }

    int skipCount = 0;
    // Detect everything (third pass)
    foreach(QString barFile, filenames)
    {
        BarInfo info = checkInstallableInfo(barFile, blitz.isBlitz());
        if (info.name == "EXIT")
            return setNewLine("Install aborted.");
        else if (info.type == AppType) {
            foreach(Apps* app, _appList) {
                if (!_allowDowngrades && info.packageid == app->packageId()) {
                    if (isVersionNewer(app->version(), info.version, true)) {
                        setNewLine(QString("%1 was skipped. Version %2 already installed").arg(QFileInfo(info.name).completeBaseName()).arg(app->version()));
                        info.type = NotInstallableType;
                    }

                    break;
                }
            }
        }
        if (info.type != NotInstallableType)
            _installInfo.append(info);
        else
            skipCount++;
    }
    if (_installInfo.isEmpty()) {
        setNewLine(QString("None of the selected files were installable. Skipped %2.")
                   .arg(skipCount));
        return;
    }
    setNewLine(QString("Installing <b>%1</b> .bar(s).%2.")
               .arg(_installInfo.count())
               .arg(skipCount > 0 ? QString(" Skipped %1").arg(skipCount) : ""));
    install();
}

void InstallNet::install()
{
    setInstalling(true);
    if (checkLogin())
    {
        QUrlQuery postData;
        _downgradePos = 0;
        _downgradeInfo.clear();
        foreach (auto filePair, _installInfo)
            _downgradeInfo.append(filePair.name);

        emit dgPosChanged();
        emit dgMaxPosChanged();
        _dlDoneBytes = 0;
        _dlOverallTotal = 0;
        for(QString filename : _downgradeInfo)
            _dlOverallTotal += QFile(filename).size();
        if (_dlOverallTotal * 1.5 > device->freeSpace) {
            QMessageBox::critical(nullptr, "Free Space", QString("Sachesi has determined you may not have enough free space on your device to continue this update.\n"
                                                                 "It is estimated that you would need %1 GB but you only have %2 GB. Please free up some space.")
                                  .arg(QString::number((_dlOverallTotal * 1.5) / 1024 / 1024 / 1024, 'g', 3))
                                  .arg(QString::number((device->freeSpace * 1.0) / 1024 / 1024 / 1024, 'g', 3)));
            setNewLine("Install aborted. No free space.");
            setInstalling(false);
            return;
        }

        postData.addQueryItem("mode", _firmwareUpdate ? "os" : "bar");
        postData.addQueryItem("size", QString::number(_dlOverallTotal));
        postQuery("update.cgi", "x-www-form-urlencoded", postData);
    }
}

void InstallNet::uninstall(QStringList packageids, bool firmwareUpdate)
{
    Q_UNUSED(firmwareUpdate) // Dangerous!
    // Tested with OS and it removed the old OS. Not entirely what I wanted.
    if (packageids.isEmpty())
        return;
    setInstalling(true);
    if (checkLogin())
    {
        QUrlQuery postData;
        _installInfo.clear();
        _downgradePos = 0;
        _downgradeInfo = packageids;
        emit dgPosChanged();
        emit dgMaxPosChanged();
        postData.addQueryItem("mode", "app");
        postData.addQueryItem("size", "0");

        postQuery("update.cgi", "x-www-form-urlencoded", postData);
    }
}

bool InstallNet::uninstallMarked()
{
    QStringList marked;
    bool firmwareUpdate = false;
    for(Apps* app : _appList) {
        if (app->isMarked()) {
            marked.append(app->packageId());
            app->setIsMarked(false);
            if (app->type() != "application")
                firmwareUpdate = true;
            app->setType("");
        }
    }
    if (marked.isEmpty())
        return false;
    uninstall(marked, firmwareUpdate);
    return true;
}

// For finding Blackberry Link backup path
/*#ifdef _WIN32
    QFile linkSettings(QDir::homePath()+"/AppData/Roaming/Research In Motion/BlackBerry 10 Desktop/Settings.config");
    linkSettings.open(QIODevice::WriteOnly);
    QXmlStreamReader xml(&linkSettings);
    for (xml.readNext(); !xml.atEnd(); xml.readNext()) {
        if (xml.isStartElement()) {
            if (xml.name() == "Configuration" && xml.attributes().count() > 1 && xml.attributes().at(0).value() == "BackupFolderLocation") {
                dir = xml.attributes().at(1).value().toString();
            }
        }
    }
    linkSettings.close();
#endif*/

void InstallNet::backup()
{
    setBacking(true);
    if (checkLogin())
    {
        currentBackupZip = new QuaZip(_backupFileName);
        currentBackupZip->setZip64Enabled(true);
        currentBackupZip->open(QuaZip::mdCreate);
        if (!currentBackupZip->isOpen()) {
            QMessageBox::critical(nullptr, "Error", "Unable to write backup. Please ensure you have permission to write to " + _backupFileName);
            delete currentBackupZip;
            currentBackupZip = nullptr;
            setBacking(false);
            return;
        }

        QuaZipFile* manifest;
        manifest = new QuaZipFile(currentBackupZip);
        QuaZipNewInfo newInfo("Manifest.xml");
        newInfo.setPermissions(QFileDevice::Permission(0x7774));
        manifest->open(QIODevice::WriteOnly, newInfo);
        QString manifestXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<BlackBerry_Backup><Version>3.0</Version><Client platform=\"SachESI\" osversion=\"Microsoft Windows NT 6.1.7601 Service Pack 1\" dtmversion=\"2.0.0.0\"/>"
                "<SourceDevice pin=\"" + device->pin + "\"><Platform type=\"QNX\" version=\"10.0.0.0\"/></SourceDevice>"
                "<QnxOSDevice><Archives>";
        foreach(BackupCategory* cat, _back.categories) {
            if (_back.modeString().contains(cat->id))
                manifestXML.append("<Archive id=\"" + cat->id + "\" name=\"" + cat->name + "\" count=\"" + cat->count + "\" bytesize=\"" + cat->bytesize + "\" keyid=" + device->bbid + "\" perimetertype=" + cat->perimetertype + "\"/>");
        }
        manifestXML.append("</Archives></QnxOSDevice></BlackBerry_Backup>");
        manifest->write(manifestXML.toStdString().c_str());
        manifest->close();
        delete manifest;

        QUrlQuery postData;
        //postData.addQueryItem("action", "backup");
        if (_back.rev() == 2) {
            /*QString packageXML = "<Packages>";
            packageXML += "<Package category=\"app\" pkgid=\"gYABgGhMIKEe6t-zx-otuOtK1JM\" type=\"data\"/>";
            packageXML += "<Package category=\"app\" pkgid=\"andrBnWwO_pMnqtLJ4heAlnaufQ\" type=\"data\"/>";
            packageXML += "</Packages>";
            QNetworkRequest request = setData("backup.cgi?opt=rev2&mode=" + _back.modeString(), "x-www-form-urlencoded");
            reply = manager->post(request, packageXML.toLatin1());

            //connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(installProgress(qint64,qint64)));
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                    this, SLOT(restoreError(QNetworkReply::NetworkError)));
            connect(reply, SIGNAL(finished()), this, SLOT(restoreReply()));*/
            postData.addQueryItem("mode", _back.modeString());
            postData.addQueryItem("opt", "rev2");
            postQuery("backup.cgi", "x-www-form-urlencoded", postData);
        } else {
            postData.addQueryItem("mode", _back.modeString());
            postQuery("backup.cgi", "x-www-form-urlencoded", postData);
        }
    }
}

void InstallNet::backup(QUrl url, int options)
{
    if (!url.isLocalFile())
        return;
    _backupFileName = url.toLocalFile();
    if(!_backupFileName.endsWith(".bbb"))
        _backupFileName.append(".bbb");
    if (_backupFileName.isEmpty())
        return;
    _back.setMode(options);
    _back.setCurMode(0);
    backup();
}

void InstallNet::backupQuery() {
    if (checkLogin())
    {
        QUrlQuery postData;
        //postData.addQueryItem("action", "backup");
        postData.addQueryItem("query", "list");
        if (_back.rev() == 2)
            postData.addQueryItem("opt", "rev2"); // Per-app backups
        postQuery("backup.cgi", "x-www-form-urlencoded", postData);
    }
}

void InstallNet::restore()
{
    setRestoring(true);
    if (checkLogin())
    {
        QUrlQuery postData;
        postData.addQueryItem("action", "restore");
        postData.addQueryItem("mode", _back.modeString());
        postData.addQueryItem("totalsize", QString::number(_back.maxSize()));
        postQuery("backup.cgi", "x-www-form-urlencoded", postData);
    }
}

void InstallNet::restore(QUrl url, int options)
{
    if (!url.isLocalFile())
        return;
    _backupFileName = url.toLocalFile();
    if (!QFile::exists(_backupFileName))
        return;
    currentBackupZip = new QuaZip(_backupFileName);
    currentBackupZip->open(QuaZip::mdUnzip);
    if (!currentBackupZip->isOpen()) {
        QMessageBox::critical(nullptr, "Error", "Could not open backup file.");
        delete currentBackupZip;
        currentBackupZip = nullptr;
        return;
    }
    for (int i = 0; i < _back.numMethods(); i++) {
        if (options & (1 << i)) {
            // We want to restore this file
            currentBackupZip->setCurrentFile(QString("Archive/" + _back.stringFromMode(i) + ".tar"));
            if (!currentBackupZip->hasCurrentFile()) {
                // But this file doesn't exist?
                options &= ~(1 << i);
            } else {
                // Set the size from this file
                QuaZipFileInfo info;
                currentBackupZip->getCurrentFileInfo(&info);
                _back.setCurMaxSize(i, info.uncompressedSize);
                qint64 startSize = (_back.maxSize() > 1) ? _back.maxSize() : 0;
                _back.setMaxSize(startSize + info.uncompressedSize);
            }
        }
    }
    if (!options)
        return;
    _back.setMode(options);
    _back.setCurMode(0);
    restore();
}

void InstallNet::wipe() {
    if (QMessageBox::critical(nullptr, "Loss of data", "Are you sure you want to wipe your device?\nThis will result in a permanent loss of data.", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
        return;
    QUrlQuery postData;
    postData.addQueryItem("wipe", "wipe");
    postQuery("wipe.cgi", "x-www-form-urlencoded", postData);
}

void InstallNet::startRTAS() {
    QUrlQuery postData;
    postData.addQueryItem("wipe", "start_rtas");
    postQuery("wipe.cgi", "x-www-form-urlencoded", postData);
}

void InstallNet::newPin(QString pin) {
    QUrlQuery postData;
    postData.addQueryItem("wipe", "pin");
    postData.addQueryItem("newpin", pin.left(8));
    postQuery("wipe.cgi", "x-www-form-urlencoded", postData);
}

void InstallNet::resignNVRAM() {
    QUrlQuery postData;
    postData.addQueryItem("wipe", "re_sign");
    postQuery("wipe.cgi", "x-www-form-urlencoded", postData);
}

void InstallNet::factorywipe() {
    if (QMessageBox::critical(nullptr, "Loss of data", "Are you sure you want to wipe your device?\nThis will result in a permanent loss of data.", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
        return;
    QUrlQuery postData;
    postData.addQueryItem("wipe", "wipe");
    postData.addQueryItem("factorywipe", "1");
    postData.addQueryItem("nopoweroff", "1");
    postQuery("wipe.cgi", "x-www-form-urlencoded", postData);
}

void InstallNet::reboot() {
    QUrlQuery postData;
    postData.addQueryItem("reset", "true");
    postQuery("reset.cgi", "x-www-form-urlencoded", postData);
}

void InstallNet::getPIN() {
    QUrlQuery postData;
    postData.addQueryItem("wipe", "getpin");
    postQuery("wipe.cgi", "x-www-form-urlencoded", postData);
}

void InstallNet::dumpLogs() {
    QUrlQuery postData;
    postData.addQueryItem("facility", "dumplog");
    postQuery("support.cgi", "x-www-form-urlencoded", postData);
}

void InstallNet::setActionProperty(QString name, QString value) {
    QUrlQuery postData;
    postData.addQueryItem("action", "set");
    postData.addQueryItem("name", name);
    postData.addQueryItem("value", value);
    postQuery("dynamicProperties.cgi", "x-www-form-urlencoded", postData);
}

void InstallNet::login()
{
    if (_loginBlock || _wrongPass)
        return;

    // How did we get here?
    if (_state) {
        checkLogin();
        return;
    }

    QStringList ips;

    int flags = QNetworkInterface::IsUp | QNetworkInterface::IsRunning | QNetworkInterface::CanBroadcast | QNetworkInterface::CanMulticast;
    foreach(QNetworkInterface inter, QNetworkInterface::allInterfaces())
    {
        if ((inter.flags() & (flags | QNetworkInterface::IsLoopBack)) == flags)
        {
            foreach(QNetworkAddressEntry addr, inter.addressEntries())
            {
                if (addr.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    QList<quint8> addrParts;
                    foreach(QString addrString, addr.ip().toString().split('.'))
                        addrParts.append(addrString.toInt());

                    if (addrParts.at(0) > 100) {
                        int masked = addrParts.at(3) - 1;
                        if (masked == 0)
                            masked = 128;
                        ips.append(QString("%1.%2.%3.%4").arg(addrParts.at(0)).arg(addrParts.at(1)).arg(addrParts.at(2)).arg(masked));
                    }
                }
            }
        }
    }
    ips.removeDuplicates();

    // Keep the user updated with how many potential devices we are dealing with here.
    setPossibleDevices(ips.count());
    if (ips.isEmpty())
        return;

    if (manager == nullptr) {
        manager = new SslNetworkAccessManager();
        manager->setProxy(QNetworkProxy::NoProxy);
    }
    if (cookieJar == nullptr) {
        cookieJar = new QNetworkCookieJar(this);
        manager->setCookieJar(cookieJar);
    }
    for(QString ip_addr : ips) {
        QNetworkRequest request;
        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "QNXWebClient/1.0");
        request.setAttribute(QNetworkRequest::CustomVerbAttribute, ip_addr);
        request.setUrl(QUrl("http://"+ip_addr+"/cgi-bin/discovery.cgi"));
        QNetworkReply* replyTemp = manager->get(request);
        connect(replyTemp, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(restoreError(QNetworkReply::NetworkError)));
        connect(replyTemp, SIGNAL(finished()), this, SLOT(discoveryReply()));
    }
}

void InstallNet::discoveryReply() {
    QNetworkReply* replyTemp = (QNetworkReply*)sender();
    QNetworkRequest request = replyTemp->request();
    QString ip_addr = request.attribute(QNetworkRequest::CustomVerbAttribute).toString();
    // Just to prevent fighting between two devices
    if (_state /*&& ip_addr != _ip*/)
        return;
    QByteArray data = replyTemp->readAll();
    //qDebug() << "Message:\n" << QString(data).simplified().left(3500);
    QXmlStreamReader xml(data);
    xml.readNextStartElement(); // RimTabletResponse
    xml.readNextStartElement();
    if (xml.name() == "DeviceCharacteristics") {
        // Valid device
        setIp(ip_addr);
        setState(1);
        if (device != nullptr)
            device->deleteLater();
        device = new DeviceInfo(); emit deviceChanged();
        while (!xml.atEnd())
        {
            xml.readNext();
            if (xml.isStartElement())
            {
                if (xml.name() == "BbPin") {
                    device->setPin(QString::number(xml.readElementText().toInt(),16).toUpper());
                } else if (xml.name() == "SystemMachine") {
                    device->setName(xml.readElementText().replace("_", " "));
                    setNewLine(QString("Connected to %1 at %2.").arg(device->name).arg(_ip));
                } else if (xml.name() == "OsType") {
                    if (xml.readElementText() == "BlackBerry PlayBook OS") {
                        device->setName("Playbook QNX6.6.0");
                        setNewLine(QString("Connected to Playbook at %1.").arg(_ip));
                    }
                } else if (xml.name() == "PlatformVersion") {
                    device->setOs(xml.readElementText());
                } else if (xml.name() == "ModelName") {
                    device->setHw(xml.readElementText());
                } else if (xml.name() == "MinSupportedProtocolVersion") {
                    // Min comes before max
                    if (xml.readElementText().toInt() == 1)
                        device->setProtocol(1);
                } else if (xml.name() == "MaxSupportedProtocolVersion") {
                    if (xml.readElementText().toInt() >= 3)
                        device->setProtocol(3);
                } else if (xml.name() == "DeveloperModeEnabled") {
                    device->setDevMode(xml.readElementText().toUInt());
                } else if (xml.name() == "OobeCompleted") {
                    device->setSetupComplete(xml.readElementText().toUInt());
                    if (!device->setupComplete) {
                        QMessageBox::information(nullptr, tr("Warning"), tr("You have not completed setup on your device. Due to this issue, it will not allow communication."));
                    }
                }
            }
        }
        emit deviceChanged();
        setCompleted(false);
        // Don't even attempt because it will kick us
        if (!device->setupComplete)
            checkLogin();
    }
    sender()->deleteLater();
}

bool InstallNet::checkLogin() {
    if (!_state)
        return false;

    if (!_completed) {
        getQuery(QString("login.cgi?request_version=%1").arg(QString::number(device->protocol)), "x-www-form-urlencoded");
        return false;
    }
    return true;
}

void InstallNet::installProgress(qint64 pread, qint64)
{
    if (pread == 0)
        return;
    _dlBytes = 50*pread;
    setCurDGProgress(qMin((int)50, (int)(_dlBytes / _dlTotal)));
}

void InstallNet::backupProgress(qint64 pread, qint64)
{
    _back.setCurSize(100*pread);
    _back.setProgress(qMin((int)100, (int)(_back.curSize() / _back.curMaxSize())));
}

void InstallNet::restoreProgress(qint64 pwrite, qint64) {
    _back.setCurSize(100*pwrite);
    _back.setProgress(qMin((int)100, (int)(_back.curSize() / _back.curMaxSize())));
}

void InstallNet::backupFileFinish()
{
    _zipFile->close();
    _zipFile->deleteLater();
    _zipFile = nullptr;

    _back.setCurMode(1);

    QUrlQuery postData;
    postData.addQueryItem("action", "backup");
    if (_back.curMode() != "complete") {
        _back.setProgress(0);
        postData.addQueryItem("type", _back.curMode());
    }
    postQuery("backup.cgi", "x-www-form-urlencoded", postData);
}

QPair<QString,QString> InstallNet::getConnected(int downloadDevice, bool specialQ30) {
    if (downloadDevice == 0) {
        if (device != nullptr && device->hw != "" && device->hw != "Unknown") {
            return qMakePair(_knownConnectedOSType, _knownConnectedRadioType);
        }
        return {"", ""};
    }
    return getFamilyFromDevice(downloadDevice, specialQ30);
}

void InstallNet::determineDeviceFamily()
{
    QString radio = _knownConnectedRadioType;
    if (radio == "qc8960.wtr5") {
        device->setHwFamily(Z30Family);
    } else if (radio == "m5730") {
        device->setHwFamily(OMAPFamily);
    } else if (radio == "qc8960") {
        device->setHwFamily(Z10Family);
    } else if (radio == "qc8930.wtr5") {
        device->setHwFamily(Z3Family);
    } else if (radio == "qc8960.wtr") {
        device->setHwFamily(Q10Family);
    } else if (radio == "qc8974.wtr2") {
        device->setHwFamily(Q30Family);
    } else {
        device->setHwFamily(UnknownFamily);
    }
}

void InstallNet::restoreReply()
{
    if (reply == nullptr)
        return;

    QByteArray data = reply->readAll();
#if DEBUG_LOG
    for (int s = 0; s < data.size(); s+=3500) qDebug() << "Message:\n" << QString(data).simplified().mid(s, 3500);
#endif
    if (data.size() == 0) {
        if (_restoring) {
            QMessageBox::information(nullptr, "Restore Error", "There was an error loading the backup file.\nThe device encountered an unrecoverable bug.\nIt is not designed to restore this backup.");
            qIoSafeFree(_zipFile);
            ioSafeFree(currentBackupZip);
            setRestoring(false);
        }
    }
    QUrlQuery postData;
    QString element;
    QXmlStreamReader xml(data);
    xml.readNextStartElement(); // RimTabletResponse
    xml.readNextStartElement();
    QString hwid;
    if (xml.name() == "Rev") {
        xml.readNextStartElement();
        xml.readNextStartElement();
    }

    if (xml.name() == "AuthChallenge")
    { // We need to verify
        QString salt, challenge;
        int iCount = 0;
        while (xml.readNextStartElement()) {
            element = xml.readElementText();
            if (xml.name() == "Salt")
                salt = element;
            else if (xml.name() == "Challenge")
                challenge = element;
            else if (xml.name() == "ICount")
                iCount = element.toInt();
            else if (xml.name() == "ErrorDescription")
            {
                setLoginBlock(true);
                return;
            }
        }
        QByteArray saltHex(salt.toLatin1());
        QByteArray challenger(challenge.toLatin1());
        QByteArray result = HashPass(challenger, QByteArray::fromHex(saltHex), iCount);

        QNetworkRequest request = reply->request();
        request.setUrl(QUrl("https://"+ _ip +":443/cgi-bin/login.cgi?challenge_data=" + result.toHex().toUpper() + "&request_version=" + QString::number(device->protocol)));

        reply = manager->get(request);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(restoreError(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(finished()), this, SLOT(restoreReply()));
    }
    else if (xml.name() == "Auth")
    { // We are authenticated
        while (!xml.atEnd())
        {
            xml.readNext();
            if (xml.isStartElement())
            {
                QString name = xml.name().toString();
                if (name == "Error")
                { //No we aren't!
                    return;
                }
                else if (name == "Status")
                {
                    if (xml.readElementText() == "Denied")
                    {
                        if (!data.contains("Attempts>0")) {
                            setWrongPass(true);
                            setCompleted(false);
                            setState(false);
                        } else {
                            setLoginBlock(true);
                            setCompleted(false);
                            setState(false);
                        }
                        return;
                    }
                }
            }
        }
        setCompleted(true);

        if (_installing)
            install();
        else if (_backing)
            backup();
        else if (_restoring)
            restore();
        else /*if (_hadPassword)*/
            scanProps();
        // This can take up to 40 seconds to respond and all communication on device is Blocking!
        // backupQuery();
    }
    else if (xml.name() == "DynamicProperties")
    {
        for (xml.readNext(); !xml.atEnd(); xml.readNext()) {
            if (xml.isStartElement())
            {
                QString name = xml.name().toString();

                if (name == "ErrorDescription")
                {
                    QMessageBox::information(nullptr, "Error", xml.readElementText(), QMessageBox::Ok);
                } else if (name == "DeviceSoftware") {
                    // About to get the apps
                    _appList.clear();
                    _appRemList.clear();
                } else if (name == "Application") {
                    Apps* newApp = new Apps();
                    while(!xml.atEnd())
                    {
                        xml.readNext();
                        if (xml.isStartElement()) {
                            if (xml.name() == "Name")
                            {
                                QString longName = xml.readElementText();
                                newApp->setName(longName);
                                if (longName.contains(".test"))
                                    longName = longName.split(".test").first();
                                else if (longName.contains(".andrB"))
                                    longName = longName.split(".andrB").first();
                                else
                                    longName = longName.split(".gY").first();

                                QStringList newLineParts = longName.split('.');
                                if (newLineParts.last().isEmpty())
                                    newLineParts.removeLast();
                                longName = "";
                                for (int i = 0; i < newLineParts.size() - 1; i++)
                                    longName += newLineParts.at(i) + ".";
                                longName += "<b>" + newLineParts.last() + "</b>";
                                newApp->setObjectName(newLineParts.last().toLower());
                                newApp->setFriendlyName(longName);
                            }
                            else if (xml.name() == "Type")
                                newApp->setType(xml.readElementText());
                            else if (xml.name() == "PackageId")
                                newApp->setPackageId(xml.readElementText());
                            else if (xml.name() == "PackageVersionId")
                                newApp->setVersionId(xml.readElementText());
                            else if (xml.name() == "PackageVersion")
                                newApp->setVersion(xml.readElementText());
                            else if (xml.name() == "Fingerprint")
                                newApp->setChecksum(xml.readElementText());
                        } else if (xml.isEndElement() && xml.name() == "Application")
                            break;
                    }
                    if (newApp->type() != "")
                        _appList.append(newApp);
                    else
                        _appRemList.append(newApp);
                    if (newApp->type() == "os") {
                        _knownConnectedOSType = newApp->name().split("os.").last().remove(".desktop").replace("verizon", "factory").replace("qc8974.factory_sfi","qc8960.factory_sfi_hybrid_qc8974");
                        device->setOs(newApp->version());
                    } else if (newApp->type() == "radio") {
                        _knownConnectedRadioType = newApp->name().split("radio.").last().remove(".omadm");
                        device->setRadio(newApp->version());
                    }
                }
                // These give the wrong result some times. Installed apps are a better indication.
                // Although they also sometimes don't match Settings -> About (eg. from a Core/Radio Autoloader).
                // else if (name == "PlatformVersion")
                // else if (name == "RadioVersion")
                else if (name == "BatteryLevel")
                    device->setBattery(xml.readElementText().toInt());
                else if (name == "HardwareID") {
                    // If the firmware reports the device as unknown (eg. Dev Alpha on 10.3), show the Hardware ID
                    if (device->hw == "Unknown") {
                        hwid = xml.readElementText().remove(0, 2);
                        // If we already know the name, make it nicer
                        if (hwid == "8d00270a")
                            hwid = "Alpha C";
                        else if (hwid == "4002607")
                            hwid = "Alpha B";
                        device->setHw(hwid);
                    }
                } else if (name == "Bbid") {
                    device->setBbid(xml.readElementText());
                }
                else if (name == "DeviceName")
                {
                    // name that the user calls their phone
                    device->setFriendlyName(xml.readElementText());
                }
                else if (name == "PolicyRestrictions") {
                    device->setRestrictions(xml.readElementText().simplified());
                }
                else if (xml.name() == "RefurbDate") {
                    device->setRefurbDate(xml.readElementText());
                } else if (xml.name() == "FreeApplicationSpace") {
                    device->setFreeSpace(xml.readElementText().toLongLong());
                } else if (xml.name() == "BoardSerialNumber") {
                    device->setBsn(xml.readElementText());
                }
            }
        }
        // We do have the radio type but we don't entirely trust it. The user could have installed anything or nothing!
        QString temporaryRadioType = _knownConnectedRadioType;
        _knownConnectedRadioType = "";
        // Not future-proof, but will work for most. Families # hardcoded to 5
        for (int i = 1; i < (5 * 2) && _knownConnectedRadioType.isEmpty(); i+=2) {
            for (int j = 0; j < dev[i].count(); j++) {
                if (dev[i][j] == hwid.toUpper()) {
                    // Q30 OS status doesn't matter, so we set 0
                    _knownConnectedRadioType = getFamilyFromDevice(j + 1, 0).second;
                    break;
                }
            }
        }
        // Well, our detection failed, so let's trust the current system.
        if (_knownConnectedRadioType.isEmpty()) {
            _knownConnectedRadioType = temporaryRadioType;
        }
        // Now we can work out the real family
        determineDeviceFamily();
        std::sort(_appList.begin(), _appList.end(),
                  [=](const Apps* i, const Apps* j) {
            if (i->type() != "application" && j->type() == "application")
                return true;
            if (j->type() != "application" && i->type() == "application")
                return false;
            return (i->objectName() < j->objectName());
        }
        );
        emit deviceChanged();
        emit appListChanged();
    }
    else if (xml.name() == "RTASChallenge") {
        QFile rtasData(getSaveDir() + "rtasdata.txt");
        rtasData.open(QIODevice::WriteOnly | QIODevice::Text);
        rtasData.write(QByteArray("Use these values for RLT:\n\n"));
        for (xml.readNext(); !xml.atEnd(); xml.readNext()) {
            if (xml.isStartElement())
            {
                if (xml.name() == "Challenge")
                    rtasData.write(QString("Challenge: "+xml.readElementText()+"\n").toLocal8Bit());
                else if (xml.name() == "ProcessorInfo")
                    rtasData.write(QString("Processor Info: "+xml.readElementText()+"\n").toLocal8Bit());
                else if (xml.name() == "ProcessorId")
                    rtasData.write(QString("Processor Id: "+xml.readElementText()+"\n").toLocal8Bit());
                else if (xml.name() == "BSN")
                    rtasData.write(QString("BSN: "+xml.readElementText()+"\n").toLocal8Bit());
                else if (xml.name() == "IMEI")
                    rtasData.write(QString("IMEI: "+xml.readElementText()+"\n\n").toLocal8Bit());
                else if (xml.name() == "Log")
                    rtasData.write(QString(xml.readElementText()+"\n").toLocal8Bit());
            }
        }
        setCompleted(false);
        setState(false);
        QMessageBox::information(nullptr, "Success", "RTAS has been started.\nSachesi will now terminate its connection.", QMessageBox::Ok);
        openFile(rtasData.fileName());
        rtasData.close();
    }
    else if (xml.name() == "DevicePIN") {
        while (!xml.atEnd())
        {
            xml.readNext();
            if (xml.isStartElement())
            {
                if (xml.name() == "PIN") {
                    device->setPin(xml.readElementText().split('X').last());
                    emit deviceChanged();
                }
            }
        }
    }
    else if (xml.name() == "Wipe") {
        for (xml.readNext(); !xml.atEnd(); xml.readNext()) {
            if (xml.isStartElement())
            {
                if (xml.name() == "ErrorDescription") {
                    QMessageBox::critical(nullptr, "Error", xml.readElementText(), QMessageBox::Ok);
                }
            }
        }
    }
    else if (xml.name() == "re_pin" || xml.name() == "re_sign" || xml.name() == "start_rtas") {
        for (xml.readNext(); !xml.atEnd(); xml.readNext()) {
            if (xml.isStartElement())
            {
                if (xml.name() == "Log") {
                    QMessageBox::critical(nullptr, "Error", xml.readElementText(), QMessageBox::Ok);
                }
            }
        }
    }
    else if (xml.name() == "DeleteRequest")
    {
        postData.addQueryItem("type", "bar");
        postData.addQueryItem("packageid", _downgradeInfo.at(_downgradePos));
        postQuery("update.cgi", "x-www-form-urlencoded", postData);
    }
    else if (xml.name() == "DeleteProgress")
    {
        _downgradePos++;
        emit dgPosChanged();
        postData.addQueryItem("type", "bar");
        // Send another packageid if more to delete or update.
        if (_downgradePos == _downgradeInfo.count())
            postData.addQueryItem("status", "success");
        else
            postData.addQueryItem("packageid", _downgradeInfo.at(_downgradePos));
        postQuery("update.cgi", "x-www-form-urlencoded", postData);
    }
    else if (xml.name() == "UpdateStart")
    {
        if (_installInfo.count()) {
            compressedFile = new QFile(_downgradeInfo.at(_downgradePos));
            compressedFile->open(QIODevice::ReadOnly);
            _dlBytes = 0;
            _dlTotal = compressedFile->size();

            BarInfo info = _installInfo.at(_downgradePos);
            if (info.type == OSType)
                setCurInstallName(info.version + " Core OS");
            else if (info.type == RadioType)
                setCurInstallName(info.version + " Radio");
            else {
                // TODO: Extract naming from bar too, if possible
                setCurInstallName(QFileInfo(info.name).completeBaseName());
            }

            if (info.type == RadioType)
                reply = manager->post(setData("update.cgi?type=radio", "octet-stream"), compressedFile);
            else
                reply = manager->post(setData("update.cgi?type=bar", "octet-stream"), compressedFile);

            compressedFile->setParent(reply);
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                    this, SLOT(restoreError(QNetworkReply::NetworkError)));
            connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(installProgress(qint64,qint64)));
            connect(reply, SIGNAL(finished()), this, SLOT(restoreReply()));
        }
        else {
            postData.addQueryItem("type", "bar");
            postData.addQueryItem("packageid", _downgradeInfo.at(_downgradePos));
            reply = manager->post(setData("update.cgi", "x-www-form-urlencoded"), postData.encodedQuery());
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                    this, SLOT(restoreError(QNetworkReply::NetworkError)));
            connect(reply, SIGNAL(finished()), this, SLOT(restoreReply()));
        }
    }
    else if (xml.name() == "UpdateSend")
    {
        for (xml.readNext(); !xml.atEnd(); xml.readNext()) {
            if (xml.isStartElement()) {
                if (xml.name() == "Status") {
                    if (xml.readElementText() == "Error") {
                        return;
                    }
                } else if (xml.name() == "ErrorDescription") {
                    QMessageBox::critical(nullptr, "Error", xml.readElementText(), QMessageBox::Ok);
                }
            }
        }
        postQuery("update.cgi", "x-www-form-urlencoded", postData);
    }
    else if (xml.name() == "UpdateProgress")
    {
        bool inProgress = false;
        while (xml.readNextStartElement()) {
            element = xml.readElementText();
            if (xml.name() == "Status")
            {
                if (element == "InProgress")
                    inProgress = true;
                else if (element == "Success")
                {
                    inProgress = false;
                    _dlDoneBytes += 100 * _dlTotal;
                    compressedFile->close();
                    if (_downgradePos == _downgradeInfo.count() - 1)
                    {
                        postData.addQueryItem("status","success");
                        reply = manager->post(setData("update.cgi", "x-www-form-urlencoded"), postData.encodedQuery());
                    }
                    else
                    {
                        _downgradePos++;
                        emit dgPosChanged();
                        compressedFile = new QFile(_downgradeInfo.at(_downgradePos));
                        compressedFile->open(QIODevice::ReadOnly);
                        _dlBytes = 0;
                        _dlTotal = compressedFile->size();

                        BarInfo info = _installInfo.at(_downgradePos);
                        if (info.type == OSType)
                            setCurInstallName(info.version + " Core OS");
                        else if (info.type == RadioType)
                            setCurInstallName(info.version + " Radio");
                        else {
                            setCurInstallName(QFileInfo(info.name).completeBaseName());
                        }

                        QNetworkRequest request;
                        if (info.type == RadioType)
                            request = setData("update.cgi?type=radio", "octet-stream");
                        else
                            request = setData("update.cgi?type=bar", "octet-stream");
                        request.setHeader(QNetworkRequest::ContentLengthHeader, compressedFile->size());
                        request.setAttribute(QNetworkRequest::DoNotBufferUploadDataAttribute, true);
                        reply = manager->post(request, compressedFile);
                        compressedFile->setParent(reply);
                        connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(installProgress(qint64,qint64)));
                    }
                }
                else if (element == "Error")
                {
                    QString errorText = QString(data).split("ErrorDescription>").at(1);
                    errorText.chop(2);
                    setNewLine("<br>While sending: " + _downgradeInfo.at(_downgradePos).split("/").last());
                    setNewLine("&nbsp;&nbsp;Error: " + errorText);
                    setInstalling(false);
                    setCurDGProgress(-1);
                }
            }
            else if (xml.name() == "Progress")
            {
                if (_downgradePos == (_downgradeInfo.count() - 1))
                    setCurDGProgress(50 + element.toInt()/2);
                else
                    setCurDGProgress(inProgress ? (50 + element.toInt()/2) : 0);

                bool resend = false;
                if (device->os.startsWith("2.")) // Playbook OS support
                    resend = !data.contains("100");
                else
                    resend = inProgress;
                if (resend)  // No 100%!
                {
                    reply = manager->post(setData("update.cgi", "x-www-form-urlencoded"), postData.encodedQuery());
                }
            }
        }
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(restoreError(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(finished()), this, SLOT(restoreReply()));
    }
    else if (xml.name() == "Backup" || xml.name() == "BackupGet") {
        for (xml.readNext(); !xml.atEnd(); xml.readNext())
        {
            if (xml.isStartElement()) {
                if (xml.name() == "Status") {
                    if (xml.readElementText() == "Error") {
                        if (_backing) {
                            setBacking(false);
                            if (currentBackupZip != nullptr) {
                                currentBackupZip->close();
                                delete currentBackupZip;
                                currentBackupZip = nullptr;
                                QFile::remove(_backupFileName);
                            }
                        } else
                            _hadPassword = false;
                    }
                } else if (xml.name() == "ErrorDescription") {
                    QMessageBox::information(nullptr, "Error", xml.readElementText().remove("HTTP_COOKIE="), QMessageBox::Ok);
                }
            }
        }
    }
    else if (xml.name() == "UpdateEnd") {
        setInstalling(false);
        setNewLine("Completed Update.");
        setCurDGProgress(-1);
    }
    else if (xml.name() == "BackupCheck")
    {
        if (_back.curMode() != "complete") {
            //postData.addQueryItem("action", "backup");
            postData.addQueryItem("type", _back.curMode());
            reply = manager->post(setData("backup.cgi", "x-www-form-urlencoded"), postData.encodedQuery());
            _zipFile = new QuaZipFile(currentBackupZip);
            QuaZipNewInfo newInfo("Archive/" + _back.curMode() + ".tar");
            newInfo.setPermissions(QFileDevice::Permission(0x7774));
            _zipFile->open(QIODevice::WriteOnly, newInfo);
            connect(reply, SIGNAL(downloadProgress(qint64,qint64)),this, SLOT(backupProgress(qint64, qint64)));
            connect(reply, &QNetworkReply::readyRead, [=]() {
                _zipFile->write(reply->readAll());
            });
            connect(reply, SIGNAL(finished()), this, SLOT(backupFileFinish()));
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                    this, SLOT(restoreError(QNetworkReply::NetworkError)));
        } else {
            postData.addQueryItem("status", "success");
            postQuery("backup.cgi", "x-www-form-urlencoded", postData);
            currentBackupZip->close();
            delete currentBackupZip;
            currentBackupZip = nullptr;
            setBacking(false);
        }
    }
    else if (xml.name() == "BackupList")
    {
        _back.clearModes();
        while(!xml.atEnd() && !xml.hasError()) {
            QXmlStreamReader::TokenType token = xml.readNext();
            if(token == QXmlStreamReader::StartElement && xml.attributes().count() > 3) {
                if (xml.attributes().at(0).name() == "id")
                    _back.addMode(xml.attributes());
                else if (xml.attributes().at(0).name() == "pkgid")
                    _back.addApp(xml.attributes());
            }
        }
        _back.sortApps();
    }
    else if (xml.name() == "BackupStart")
    {
        if (data.contains("Error")) {
            setBacking(false);
            setRestoring(false);
            return;
        }

        postData.addQueryItem("query", "activity");
        if (_back.rev() == 2)
            postData.addQueryItem("opt", "rev2");
        postQuery("backup.cgi", "x-www-form-urlencoded", postData);
    }
    else if (xml.name() == "BackupStartActivity")
    {
        postData.addQueryItem("type", _back.curMode());

        if (_back.rev() == 2) {
            postData.addQueryItem("opt", "rev2");

            //if (_back.curMode() == "app") {
                // Select app by pkgid:
                //postData.addQueryItem("pkgid", "gYABgGhMIKEe6t-zx-otuOtK1JM");
                // Select apps by pkgtype (system, bin, data):
                //postData.addQueryItem("pkgtype", "data");
            //}
        }

        reply = manager->post(setData("backup.cgi", "x-www-form-urlencoded"), postData.encodedQuery());

        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(restoreError(QNetworkReply::NetworkError)));

        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement()) {
                if (xml.name() == "Status" && xml.readElementText() == "InProgress") {
                    connect(reply, SIGNAL(finished()), this, SLOT(restoreReply()));
                }
                else if (xml.name() == "TotalSize")
                {
                    _back.setMaxSize(xml.readElementText().toLongLong());
                    _zipFile = new QuaZipFile(currentBackupZip);
                    QuaZipNewInfo newInfo("Archive/" + _back.curMode() + ".tar");
                    newInfo.setPermissions(QFileDevice::Permission(0x7774));
                    _zipFile->open(QIODevice::WriteOnly, newInfo);
                    connect(reply, &QNetworkReply::readyRead, [=]() {
                        _zipFile->write(reply->readAll());
                    });
                    connect(reply, SIGNAL(finished()), this, SLOT(backupFileFinish()));
                    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),this, SLOT(backupProgress(qint64, qint64)));
                }
                else {
                    for (int i = 0; i < _back.numMethods(); i++) {
                        // This is usually App, Media and Settings but make it future-proof
                        if (xml.name().compare(_back.stringFromMode(i), Qt::CaseInsensitive) == 0) {
                            _back.setCurMaxSize(i, xml.readElementText().toLongLong());
                        }
                    }
                }
            }
        }
    }
    else if (xml.name() == "RestoreStart")
    {
        if (data.contains("Error")) {
            setBacking(false);
            setRestoring(false);
            return;
        }
        restoreSendFile();
    }
    else if (xml.name() == "RestoreSend")
    {
        qIoSafeFree(_zipFile);
        _back.setCurMode(1);
        if (_back.curMode() == "complete") {
            postData.addQueryItem("status", "success");
            postQuery("backup.cgi", "x-www-form-urlencoded", postData);

            setRestoring(false);
            currentBackupZip->close();
            delete currentBackupZip;
            currentBackupZip = nullptr;
        } else {
            restoreSendFile();
        }
    }
}

void InstallNet::restoreSendFile() {
    currentBackupZip->setCurrentFile(QString("Archive/" + _back.curMode() + ".tar"));
    _zipFile = new QuaZipFile(currentBackupZip);
    _zipFile->open(QIODevice::ReadOnly);
    QNetworkRequest request = setData("backup.cgi?action=restore&type="+_back.curMode()+"&size="+_back.curMaxSize(), "octet-stream");
    request.setHeader(QNetworkRequest::ContentLengthHeader, _zipFile->size());
    request.setAttribute(QNetworkRequest::DoNotBufferUploadDataAttribute, true);
    reply = manager->post(request, _zipFile);
    _zipFile->setParent(reply);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(restoreError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(restoreProgress(qint64, qint64)));
    connect(reply, SIGNAL(finished()), this, SLOT(restoreReply()));
}

void InstallNet::resetVars()
{
    qSafeFree(manager);
    qSafeFree(reply);
    qSafeFree(cookieJar);
    setCompleted(false);
    setInstalling(false);
    setRestoring(false);
    setBacking(false);
    qIoSafeFree(_zipFile);
    ioSafeFree(currentBackupZip);
    if (device != nullptr) {
        device->deleteLater();
        device = nullptr;
        emit deviceChanged();
    }
    setState(0);
    setCurDGProgress(-1);
    _dlBytes = 0;
    _dlTotal = 0;
}

void InstallNet::restoreError(QNetworkReply::NetworkError error)
{
    if (error == 5 || error == 99) // On purpose or unreachable
        return;

    // This is only if it's a discovery pong. Otherwise it will be empty string.
    QNetworkRequest request = ((QNetworkReply*)sender())->request();
    QString ip_addr = request.attribute(QNetworkRequest::CustomVerbAttribute).toString();
    // If it's a discovery pong, the IP will be 169.x.x.x (at least 9 chars)
    QString this_ip = (ip_addr.length()) >= 9 ? ip_addr : _ip;

    if (_state && this_ip == _ip) {
        resetVars();
    } else if (_state) {
        return;
    }
    QString errString = QString("Communication Error: %1 (%2) from %3")
            .arg(error)
            .arg( ((QNetworkReply*)sender())->errorString() )
            .arg(this_ip);
    setNewLine(errString);
    qDebug() << errString;
}

void InstallNet::logadd(QString logtxt)
{
    Q_UNUSED(logtxt);
    return;
}

QByteArray InstallNet::HashPass(QByteArray challenge, QByteArray salt, int iterations)
{
    /* Create Hashed Password */
    QByteArray hashedData = QByteArray(_password.toLatin1());
    int count = 0;
    bool challenger = true;
    do {
        QByteArray buf(4 + salt.length() + hashedData.length(),0);
        QDataStream buffer(&buf,QIODevice::WriteOnly);
        buffer.setByteOrder(QDataStream::LittleEndian);
        buffer << qint32(count);
        buffer.writeRawData(salt, salt.length());
        buffer.writeRawData(hashedData, hashedData.length());
        if (!count) hashedData.resize(64);
        SHA512((const unsigned char*)buf.data(), buf.length(), (unsigned char *)hashedData.data());
        if ((count == iterations - 1) && challenger)
        {
            count = -1;
            challenger = false;
            hashedData.prepend(challenge);
        }
    } while (++count < iterations);
    return hashedData;
}

void InstallNet::disconnected()
{
    setState(0);
    setCompleted(false);
    setRestoring(false);
    setBacking(false);
    setInstalling(false);
}

void InstallNet::connected()
{
    requestConfigure();
}

QString InstallNet::appDeltaMsg()
{
    if (_appList.count() == 0)
        return "";
    QString delta = "<currentSoftware>";
    for (int i = 0; i < _appList.count(); i++) {
        delta.append("<package id=\"" + _appList.at(i)->packageId() + "\" name=\"" + _appList.at(i)->name() + "\" type=\"" + _appList.at(i)->type() + "\"><version id=\"" + _appList.at(i)->versionId() + "\">" + _appList.at(i)->version() + "</version><checkSum type=\"SHA512\">" + _appList.at(i)->checksum() + "</checkSum></package>");
    }
    delta.append("</currentSoftware>");
    return delta;
}

void InstallNet::exportInstalled()
{
    QFile installedTxt(getSaveDir() + "/installed.txt");
    installedTxt.open(QIODevice::WriteOnly | QIODevice::Text);
    installedTxt.write("Installed Applications:\n");
    for (int i = 0; i < _appList.count(); i++) {
        if (_appList.at(i)->type() != "") {
            QString appLine = _appList.at(i)->friendlyName().remove("<b>").remove("</b>").leftJustified(55);
            appLine.append(_appList.at(i)->version() + "\n");
            installedTxt.write(appLine.toStdString().c_str());
        }
    }
    if (_appRemList.count()) {
        installedTxt.write("\n\nRemoved Applications:\n");
        for (int i = 0; i < _appRemList.count(); i++) {
            QString appLine = _appRemList.at(i)->friendlyName().remove("<b>").remove("</b>").leftJustified(55);
            appLine.append(_appRemList.at(i)->version() + "\n");
            installedTxt.write(appLine.toStdString().c_str());
        }
    }
    openFile(installedTxt.fileName());
    installedTxt.close();
}

//Network Manager
QNetworkReply* SslNetworkAccessManager::createRequest(Operation op, const QNetworkRequest& req, QIODevice* outgoingData)
{
    QNetworkReply* reply = QNetworkAccessManager::createRequest(op, req, outgoingData);
    reply->ignoreSslErrors();
    return reply;
}
