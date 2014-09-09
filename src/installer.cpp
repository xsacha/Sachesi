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

#include "installer.h"
#include "ports.h"
#include <QAbstractListModel>
#include <QDebug>
#include <QMessageBox>
#include <QNetworkInterface>

InstallNet::InstallNet( QObject* parent) : QObject(parent),
    manager(nullptr), reply(nullptr), cookieJar(nullptr),
    _knownHWFamily(0), _wrongPass(false), _wrongPassBlock(false),
    _hadPassword(true), currentBackupZip(nullptr), _zipFile(nullptr)
{
    resetVars();
#ifdef _MSC_VER
    WSAStartup(MAKEWORD(2,0), &wsadata);
#endif
    connectTimer = new QTimer();
    connectTimer->setInterval(3000);
    connectTimer->start();
    connect(connectTimer, SIGNAL(timeout()), this, SLOT(login()));
    QSettings settings("Qtness","Sachesi");
    setIp(settings.value("ip","169.254.0.1").toString());
    connect(&_back, SIGNAL(curModeChanged()), this, SIGNAL(backStatusChanged()));
    connect(&_back, SIGNAL(curSizeChanged()), this, SIGNAL(backCurProgressChanged()));
    connect(&_back, SIGNAL(numMethodsChanged()), this, SIGNAL(backMethodsChanged()));

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
    request.setRawHeader("User-Agent", "QNXWebClient/1.0");
    request.setUrl(QUrl("https://" + _ip + "/cgi-bin/" + page));
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

BarInfo InstallNet::checkInstallableInfo(QString name)
{
    BarInfo barInfo = {name, "", NotInstallableType};
    QuaZipFile manifest(name, "META-INF/MANIFEST.MF", QuaZip::csSensitive);
    if (!manifest.open(QIODevice::ReadOnly))
        return barInfo;
    QString appName, type, system;
    while (!manifest.atEnd()) {
        QByteArray newLine = manifest.readLine();
        if (newLine.startsWith("Package-Name:")) {
            appName = newLine.split(':').last().simplified();
        }
        else if (newLine.startsWith("Package-Type:")) {
            type = newLine.split(':').last().simplified();
            if (type == "system" && barInfo.type == NotInstallableType)
                barInfo.type = OSType;
            else
                break;
        }
        else if (newLine.startsWith("Package-Version:")) {
            barInfo.version = newLine.split(':').last().simplified();
        }
        else if (newLine.startsWith("System-Type:")) {
            if (newLine.split(':').last().simplified() == "radio")
                barInfo.type = RadioType;
            break;
        }
    }

    // Check if we are about to make a huge mistake!
    if (barInfo.type == OSType) {
        QString installableOS = appName.split("os.").last().remove(".desktop");
        if (installableOS != _knownConnectedOSType) {
            int choice = QMessageBox::critical(nullptr, "WARNING", "The OS file you have selected to install is for a different device!\nOS Type: " + installableOS + "\nYour Device: " + _knownConnectedOSType, "Continue", "Skip", "Exit", 2);
            if (choice == 1) {
                barInfo.type = NotInstallableType;
                return barInfo;
            }
            if (choice == 2) {
                barInfo.name = "EXIT";
                return barInfo;
            }
        }
    } else if (barInfo.type == RadioType) {
        QString installableRadio = appName.split("radio.").last().remove(".omadm");
        if (installableRadio != _knownConnectedRadioType) {
            int choice = QMessageBox::critical(nullptr, "WARNING", "The Radio file you have selected to install is for a different device!\nRadio Type: " + installableRadio + "\nYour Device: " + _knownConnectedRadioType, "Continue", "Skip", "Exit", 2);
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

    // Check if we are upgrading firmware or just installing apps.
    if (type == "system") {
        // Only if installing
        setNewLine("<b>Installing " + system + ": " + barInfo.version +"</b>");
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
    foreach(QUrl url, files)
    {
        if (!url.isLocalFile())
            continue;
        QString name = url.toLocalFile();

        if (QFileInfo(name).isDir())
        {
            QStringList barFiles = QDir(name).entryList(QStringList("*.bar"));
            for(QString barFile : barFiles)
            {
                BarInfo info = checkInstallableInfo(name + "/" + barFile);
                if (info.name == "EXIT")
                    return setNewLine("Install aborted.");

                if (info.type != NotInstallableType)
                    _installInfo.append(info);
            }
        } else {
            BarInfo info = checkInstallableInfo(name);
            if (info.name == "EXIT")
                return setNewLine("Install aborted.");

            if (info.type != NotInstallableType)
                _installInfo.append(info);
        }
    }
    if (_installInfo.isEmpty())
        return;
    setNewLine(QString("Installing <b>%1</b> .bar(s).")
               .arg(_installInfo.count()));
    install();
}

void InstallNet::install()
{
    setInstalling(true);
    if (checkLogin())
    {
        QUrlQuery postData;
        int nfilesize = 0;
        _downgradePos = 0;
        _downgradeInfo.clear();
        for (auto filePair : _installInfo)
            _downgradeInfo.append(filePair.name);

        emit dgPosChanged();
        emit dgMaxPosChanged();
        for(QString filename : _downgradeInfo)
            nfilesize += QFile(filename).size();

        postData.addQueryItem("mode", _firmwareUpdate ? "os" : "bar");
        postData.addQueryItem("size", QString::number(nfilesize));
        postQuery("update.cgi", "x-www-form-urlencoded", postData);
    }
}

void InstallNet::uninstall(QStringList packageids)
{
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
    for(Apps* app : _appList) {
        if (app->isMarked()) {
            marked.append(app->packageId());
            app->setIsMarked(false);
            app->setType("");
        }
    }
    if (marked.isEmpty())
        return false;
    uninstall(marked);
    return true;
}

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
        manifest->open(QIODevice::WriteOnly, QuaZipNewInfo("Manifest.xml"), nullptr, 0, 8);
        QString manifestXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<BlackBerry_Backup><Version>3.0</Version><Client platform=\"SachESI\" osversion=\"Microsoft Windows NT 6.1.7601 Service Pack 1\" dtmversion=\"2.0.0.0\"/>"
                "<SourceDevice pin=\"" + _knownPIN + "\"><Platform type=\"QNX\" version=\"10.0.0.0\"/></SourceDevice>"
                "<QnxOSDevice><Archives>";
        foreach(BackupCategory* cat, _back.categories) {
            if (_back.modeString().contains(cat->id))
                manifestXML.append("<Archive id=\"" + cat->id + "\" name=\"" + cat->name + "\" count=\"" + cat->count + "\" bytesize=\"" + cat->bytesize + "\" perimetertype=" + cat->perimetertype + "\"/>");
        }
        manifestXML.append("</Archives></QnxOSDevice></BlackBerry_Backup>");
        manifest->write(manifestXML.toStdString().c_str());
        manifest->close();
        delete manifest;

        QUrlQuery postData;
        postData.addQueryItem("action", "backup");
        postData.addQueryItem("mode", _back.modeString());
        postQuery("backup.cgi", "x-www-form-urlencoded", postData);
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
        postData.addQueryItem("action", "backup");
        postData.addQueryItem("query", "list");
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
    if (_state || _wrongPassBlock)
        return;

    QStringList ips;

    int flags = QNetworkInterface::IsUp | QNetworkInterface::IsRunning | QNetworkInterface::CanBroadcast | QNetworkInterface::CanMulticast;
    foreach(QNetworkInterface inter, QNetworkInterface::allInterfaces())
    {
        // VMWare responds for some reason. Who else does?
        if (inter.humanReadableName().startsWith("VMware"))
            continue;
        if ((inter.flags() & flags) == flags && !inter.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            foreach(QNetworkAddressEntry addr, inter.addressEntries())
            {
                if (addr.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    QList<quint8> addrParts;
                    foreach(QString addrString, addr.ip().toString().split('.'))
                        addrParts.append(addrString.toInt());
                    if ((addrParts.last() % 4) != 2)
                        continue;
                    if (addrParts.at(0) == 169 && addrParts.at(1) == 254)
                        ips.append(QString("169.254.%1.%2").arg(addrParts.at(2)).arg(addrParts.at(3) - 1));
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
        request.setRawHeader("User-Agent", "QNXWebClient/1.0");
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
    //qDebug() << "Message:\n" << QString(data).simplified().left(3000);
    QXmlStreamReader xml(data);
    xml.readNextStartElement(); // RimTabletResponse
    xml.readNextStartElement();
    if (xml.name() == "DeviceCharacteristics") {
        // Valid device
        setIp(ip_addr);
        setState(1);
        while (!xml.atEnd())
        {
            xml.readNext();
            if (xml.isStartElement())
            {
                if (xml.name() == "BbPin") {
                    setKnownPIN(QString::number(xml.readElementText().toInt(),16).toUpper());
                } else if (xml.name() == "SystemMachine") {
                    setKnownName(xml.readElementText());
                    setNewLine(QString("Connected to %1 at %2.").arg(_knownName).arg(_ip));
                } else if (xml.name() == "OsType") {
                    if (xml.readElementText() == "BlackBerry PlayBook OS") {
                        setKnownName("Playbook_QNX6.6.0");
                        setNewLine(QString("Connected to Playbook at %1.").arg(_ip));
                    }
                } else if (xml.name() == "PlatformVersion") {
                    setKnownOS(xml.readElementText());
                } else if (xml.name() == "Power") {
                    setKnownBattery(xml.readElementText().toInt());
                } else if (xml.name() == "ModelName") {
                    setKnownHW(xml.readElementText());
                } else if (xml.name() == "MinSupportedProtocolVersion") {
                    // Min comes before max
                    if (xml.readElementText().toInt() == 1)
                        _knownProtocol = 1;
                } else if (xml.name() == "MaxSupportedProtocolVersion") {
                    if (xml.readElementText().toInt() >= 3)
                        _knownProtocol = 3;
                }
            }
        }
        setCompleted(false);
        checkLogin();
    }
    sender()->deleteLater();
}

bool InstallNet::checkLogin() {
    if (!_state)
        return false;

    if (!_completed) {
        getQuery(QString("login.cgi?request_version=%1").arg(_knownProtocol), "x-www-form-urlencoded");
        return false;
    }
    return true;
}

void InstallNet::installProgress(qint64 pread, qint64)
{
    if (pread == 0)
        return;
    _dlBytes = 50*pread;
    setDGProgress(qMin((int)50, (int)(_dlBytes / _dlTotal)));
}

void InstallNet::backupProgress(qint64 pread, qint64)
{
    _back.setCurSize(100*pread);
    _back.setProgress(qMin((int)100, (int)(_back.curSize() / _back.curMaxSize())));
}

void InstallNet::backupFileReady()
{
    if (reply->bytesAvailable() > 16384) {
        while (!reply->atEnd())
            _zipFile->write(reply->read(16384));
    }
}

void InstallNet::restoreProgress(qint64 pwrite, qint64) {
    _back.setCurSize(100*pwrite);
    _back.setProgress(qMin((int)100, (int)(_back.curSize() / _back.curMaxSize())));
}

void InstallNet::backupFileFinish()
{
    _zipFile->write(reply->readAll());
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

QPair<QString,QString> InstallNet::getConnected(int downloadDevice) {
    QPair<QString,QString> ret = {"", ""};
    if (downloadDevice == 0) {
        if (_knownHW != "" && _knownHW != "Unknown") {
            ret = qMakePair(_knownConnectedOSType, _knownConnectedRadioType);
        }
    } else {
        switch(downloadDevice) {
        case Z30Family:
            ret = {"qc8960.factory_sfi", "qc8960.wtr5"};
            break;
        case OMAPFamily:
            ret = {"winchester.factory_sfi", "m5730"};
            break;
        case Z10Family:
            ret = {"qc8960.factory_sfi", "qc8960"};
            break;
        case Z3Family:
            ret = {"qc8960.factory_sfi", "qc8930.wtr5"};
            break;
        case Q30Family:
            ret = {"qc8974.factory_sfi", "qc8974.wtr2"};
            break;
        case Q10Family:
            ret = {"qc8960.factory_sfi", "qc8960.wtr"};
            break;
        }
    }
    return ret;
}

void InstallNet::determineDeviceFamily()
{
    QString radio = _knownConnectedRadioType;
    if (radio == "qc8960.wtr5") {
        _knownHWFamily = Z30Family;
    } else if (radio == "m5730") {
        _knownHWFamily = OMAPFamily;
    } else if (radio == "qc8960") {
        _knownHWFamily = Z10Family;
    } else if (radio == "qc8930.wtr5") {
        _knownHWFamily = Z3Family;
    } else if (radio == "qc8960.wtr") {
        _knownHWFamily = Q10Family;
    } else if (radio == "qc8974.wtr2") {
        _knownHWFamily = Q30Family;
    } else {
        _knownHWFamily = UnknownFamily;
    }
}

void InstallNet::restoreReply()
{
    QByteArray data = reply->readAll();
    //for (int s = 0; s < data.size(); s+=3000) qDebug() << "Message:\n" << QString(data).simplified().mid(s, 3000);
    if (data.size() == 0) {
        if (_restoring) {
            QMessageBox::information(nullptr, "Restore Error", "There was an error loading the backup file.\nThe device encountered an unrecoverable bug.\nIt is not designed to restore this backup.");
            if (_zipFile != nullptr) {
                if (_zipFile->isOpen())
                    _zipFile->close();
                _zipFile->deleteLater();
                _zipFile = nullptr;
            }
            if (currentBackupZip != nullptr) {
                if (currentBackupZip->isOpen())
                    currentBackupZip->close();
                delete currentBackupZip;
                currentBackupZip = nullptr;
            }
            setRestoring(false);
        }
    }
    QUrlQuery postData;
    QString element;
    QXmlStreamReader xml(data);
    xml.readNextStartElement(); // RimTabletResponse
    xml.readNextStartElement();
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
                setWrongPassBlock(true);
                setCompleted(false);
                setState(false);
                return;
            }
        }
        QByteArray saltHex(salt.toLatin1());
        QByteArray challenger(challenge.toLatin1());
        QByteArray result = HashPass(challenger, QByteArray::fromHex(saltHex), iCount);

        QNetworkRequest request = reply->request();
        request.setUrl(QUrl("https://"+ _ip +":443/cgi-bin/login.cgi?challenge_data=" + result.toHex().toUpper() + "&request_version=" + QString::number(_knownProtocol)));

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
                        if (!data.contains("Attempts>0</"))
                            setWrongPass(true);
                        else {
                            setWrongPassBlock(true);
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
        else if (_hadPassword)
            scanProps();
        // This can take up to 5 seconds to respond and all communication on device is Blocking!
        // backupQuery();
        // For example: if Link is talking to the device at the same time, comms will fail and vice-versa
    }
    else if (xml.name() == "DynamicProperties")
    {
        _appList.clear();
        _appRemList.clear();
        for (xml.readNext(); !xml.atEnd(); xml.readNext()) {
            if (xml.isStartElement())
            {
                QString name = xml.name().toString();

                if (name == "ErrorDescription")
                {
                    QMessageBox::information(nullptr, "Error", xml.readElementText(), QMessageBox::Ok);
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
                                else
                                    longName = longName.split(".gY").first();
                                QStringList newLineParts = longName.split('.');
                                if (newLineParts.last().isEmpty())
                                    newLineParts.removeLast();
                                longName = "";
                                for (int i = 0; i < newLineParts.size() - 1; i++)
                                    longName += newLineParts.at(i) + ".";
                                longName += "<b>" + newLineParts.last() + "</b>";
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
                    if (newApp->type() == "os")
                        _knownConnectedOSType = newApp->name().split("os.").last().remove(".desktop");
                    else if (newApp->type() == "radio")
                        _knownConnectedRadioType = newApp->name().split("radio.").last().remove(".omadm");
                }
                else if (name == "PlatformVersion")
                    setKnownOS(xml.readElementText());
                else if (name == "RadioVersion")
                    setKnownRadio(xml.readElementText());
                else if (name == "BatteryLevel")
                    setKnownBattery(xml.readElementText().toInt());
                else if (name == "HardwareID") {
                    // If the firmware reports the device as unknown (eg. Dev Alpha on 10.3), show the Hardware ID
                    if (_knownHW == "Unknown") {
                        QString name = xml.readElementText().remove(0, 2);
                        // If we already know the name, make it nicer
                        if (name == "8d00270a")
                            name = "Alpha C";
                        else if (name == "4002607")
                            name = "Alpha B";
                        setKnownHW(name);
                    }
                }
                /* // DEPRECATED by discovery.cgi
                else if (name == "DeviceName")
                {
                    // name that the user calls their phone
                }*/
            }
        }
        // No need to show this list anymore, I think
        /*
        QString appInfoList = "<b>Currently Installed Applications:</b><br>";
        foreach (Apps* apps, _appList)
        {
            appInfoList.append("  " + apps->friendlyName() + "<br>");
        }
        setNewLine(appInfoList);*/
        determineDeviceFamily();
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
                if (xml.name() == "PIN")
                    _knownPIN = xml.readElementText().split('X').last();
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
                setCurInstallName("Sending " + info.version + " Core OS");
            else if (info.type == RadioType)
                setCurInstallName("Sending " + info.version + " Radio");
            else {
                // TODO: Extract naming from bar too, if possible
                QString literal_name = compressedFile->fileName().split('/').last();
                QStringList fileParts = literal_name.split('-',QString::SkipEmptyParts);
                setCurInstallName("Sending " + fileParts.at(0));
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
                    setCurInstallName(_curInstallName + " Sent.");
                    _downgradePos++;
                    emit dgPosChanged();
                    if (_downgradePos == _downgradeInfo.count())
                    {
                        postData.addQueryItem("status","success");
                        reply = manager->post(setData("update.cgi", "x-www-form-urlencoded"), postData.encodedQuery());
                        compressedFile->close();
                    }
                    else
                    {
                        compressedFile->close();
                        compressedFile = new QFile(_downgradeInfo.at(_downgradePos));
                        compressedFile->open(QIODevice::ReadOnly);
                        _dlBytes = 0;
                        _dlTotal = compressedFile->size();

                        BarInfo info = _installInfo.at(_downgradePos);
                        if (info.type == OSType)
                            setCurInstallName("Sending " + info.version + " Core OS");
                        else if (info.type == RadioType)
                            setCurInstallName("Sending " + info.version + " Radio");
                        else {
                            QString literal_name = compressedFile->fileName().split('/').last();
                            QStringList fileParts = literal_name.split('-',QString::SkipEmptyParts);
                            setCurInstallName("Sending " + fileParts.at(0));
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
                    setDGProgress(-1);
                    setCurDGProgress(-1);
                }
            }
            else if (xml.name() == "Progress")
            {
                if (_downgradePos == (_downgradeInfo.count() - 1))
                    setDGProgress(50 + element.toInt()/2);
                else
                    setDGProgress(inProgress ? (50 + element.toInt()/2) : 0);
                bool resend = false;
                if (_knownOS.startsWith("2.")) // Playbook OS support
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
        setDGProgress(-1);
        setCurDGProgress(-1);
    }
    else if (xml.name() == "BackupCheck")
    {
        if (_back.curMode() != "complete") {
            postData.addQueryItem("action", "backup");
            postData.addQueryItem("type", _back.curMode());
            reply = manager->post(setData("backup.cgi", "x-www-form-urlencoded"), postData.encodedQuery());
            _zipFile = new QuaZipFile(currentBackupZip);
            _zipFile->open(QIODevice::WriteOnly, QuaZipNewInfo("Archive/" + _back.curMode() + ".tar"), nullptr, 0, 8);
            connect(reply, SIGNAL(downloadProgress(qint64,qint64)),this, SLOT(backupProgress(qint64, qint64)));
            connect(reply, SIGNAL(readyRead()), this, SLOT(backupFileReady()));
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
            if(token == QXmlStreamReader::StartElement && xml.attributes().count() > 3 &&  xml.attributes().at(0).name() == "id") {
                _back.addMode(xml.attributes());
            }
        }
    }
    else if (xml.name() == "BackupStart")
    {
        if (data.contains("Error")) {
            setBacking(false);
            setRestoring(false);
            return;
        }
        postData.addQueryItem("action", "backup");
        postData.addQueryItem("type", _back.curMode());
        postQuery("backup.cgi", "x-www-form-urlencoded", postData);
    }
    else if (xml.name() == "BackupStartActivity")
    {
        postData.addQueryItem("type", _back.curMode());
        reply = manager->post(setData("backup.cgi", "x-www-form-urlencoded"), postData.encodedQuery());
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(restoreError(QNetworkReply::NetworkError)));

        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement()) {
                if (xml.name() == "Status" && xml.readElementText() == "InProgress") {
                    connect(reply, SIGNAL(finished()), this, SLOT(restoreReply()));
                }
                if (xml.name() == "Settings") {
                    _back.setCurMaxSize(0, xml.readElementText().toLongLong());
                }
                else if (xml.name() == "Media") {
                    _back.setCurMaxSize(1, xml.readElementText().toLongLong());
                }
                else if (xml.name().startsWith("App")) {
                    _back.setCurMaxSize(2, xml.readElementText().toLongLong());
                }
                else if (xml.name() == "TotalSize")
                {
                    _back.setMaxSize(xml.readElementText().toLongLong());
                    _zipFile = new QuaZipFile(currentBackupZip);
                    _zipFile->open(QIODevice::WriteOnly, QuaZipNewInfo("Archive/" + _back.curMode() + ".tar"), nullptr, 0, 8);
                    connect(reply, SIGNAL(readyRead()), this, SLOT(backupFileReady()));
                    connect(reply, SIGNAL(finished()), this, SLOT(backupFileFinish()));
                    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),this, SLOT(backupProgress(qint64, qint64)));
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
        if (_zipFile) {
            if (_zipFile->isOpen())
                _zipFile->close();
            _zipFile->deleteLater();
            _zipFile = nullptr;
        }
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
    if (manager != nullptr) {
        manager->deleteLater();
        manager = nullptr;
    }
    if (reply != nullptr) {
        reply->deleteLater();
        reply = nullptr;
    }
    if (cookieJar != nullptr) {
        cookieJar->deleteLater();
        cookieJar = nullptr;
    }
    setCompleted(false);
    setRestoring(false);
    setBacking(false);
    setInstalling(false);
    if (currentBackupZip != nullptr) {
        delete currentBackupZip;
        currentBackupZip = nullptr;
    }
    if (_zipFile != nullptr) {
        _zipFile->deleteLater();
        _zipFile = nullptr;
    }
    setKnownPIN("");
    setKnownOS("");
    setKnownRadio("N/A");
    setKnownBattery(-1);
    setState(0);
    setDGProgress(-1);
    setCurDGProgress(-1);
    _dlBytes = 0;
    _dlTotal = 0;
}

void InstallNet::restoreError(QNetworkReply::NetworkError error)
{
    if (error == 5) // On purpose
        return;

    // This is only if it's a discovery pong. Otherwise it will be empty string.
    QNetworkRequest request = ((QNetworkReply*)sender())->request();
    QString ip_addr = request.attribute(QNetworkRequest::CustomVerbAttribute).toString();
    // If it's a discovery pong, the IP will be 169.x.x.x (at least 9 chars)
    QString this_ip = (ip_addr.length()) >= 9 ? ip_addr : _ip;

    if (_state && ip_addr == _ip) {
        resetVars();
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
