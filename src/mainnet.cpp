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

#include "apps.h"
#include "mainnet.h"
#include "splitter.h"
#include "ports.h"
#include <QStringList>
#include <QMessageBox>

MainNet::MainNet(InstallNet *installer, QObject* parent)
    : QObject(parent)
    , _i(installer)
    , replydl(nullptr)
    , currentDownload(nullptr)
    , _updateMessage("")
    , _softwareRelease("")
    , _versionRelease("")
    , _downloading(false)
    , _hasPotentialLinks(false)
    , _scanning(0)
    , _currentId(0)
    , _maxId(0)
    , _splitting(0)
    , _splitProgress(0)
    , _dlProgress(-1)
{
    manager = new QNetworkAccessManager();
}

MainNet::~MainNet()
{
}

void MainNet::splitAutoloader(QUrl url, int options) {
    if (url.isEmpty())
        return;
    QString fileName = url.toLocalFile();
    _options = options;

    QFileInfo fileInfo(fileName);
    _splitting = 1; emit splittingChanged();
    splitThread = new QThread();
    Splitter* splitter = new Splitter(fileName, _options);
    splitter->moveToThread(splitThread);
    if (fileInfo.suffix() == "exe")
        connect(splitThread, SIGNAL(started()), splitter, SLOT(processSplitAutoloader()));
    else
        connect(splitThread, SIGNAL(started()), splitter, SLOT(processSplitBar()));
    connect(splitter, SIGNAL(finished()), splitThread, SLOT(quit()));
    connect(splitter, SIGNAL(finished()), this, SLOT(cancelSplit()));
    connect(splitter, SIGNAL(progressChanged(int)), this, SLOT(setSplitProgress(int)));
    connect(splitThread, SIGNAL(finished()), splitter, SLOT(deleteLater()));
    connect(splitThread, SIGNAL(finished()), splitThread, SLOT(deleteLater()));
    splitThread->start();
}

void MainNet::combineAutoloader(QList<QUrl> selectedFiles)
{
    QList<QFileInfo> splitFiles;
    foreach(QUrl url, selectedFiles) {
        QFileInfo fileInfo = QFileInfo(url.toLocalFile());
        if (fileInfo.isDir())
        {
            QStringList suffixOnly = fileInfo.absoluteDir().entryList(QStringList("*.signed"));
            foreach (QString suffix, suffixOnly) {
                splitFiles.append(QFileInfo(fileInfo.absoluteFilePath() + "/" + suffix));
            }
        } else if (fileInfo.suffix() == "signed")
            splitFiles.append(fileInfo);
    }
    if (splitFiles.isEmpty())
        return;
    splitThread = new QThread;
    _splitting = 2; emit splittingChanged();
    Splitter* splitter = new Splitter(splitFiles);
    splitter->moveToThread(splitThread);
    connect(splitThread, SIGNAL(started()), splitter, SLOT(processCombine()));
    connect(splitter, SIGNAL(finished()), splitThread, SLOT(quit()));
    connect(splitter, SIGNAL(finished()), this, SLOT(cancelSplit()));
    connect(splitter, SIGNAL(progressChanged(int)), this, SLOT(setSplitProgress(int)));
    connect(splitThread, SIGNAL(finished()), splitter, SLOT(deleteLater()));
    connect(splitThread, SIGNAL(finished()), splitThread, SLOT(deleteLater()));

    // Download required files.
    if (!QFileInfo(capPath()).exists())
    {
        QString capUrl = "http://ppsspp.mvdan.cc/cap3.11.0.11.exe";
        QNetworkAccessManager* mNetworkManager = new QNetworkAccessManager(this);
        QObject::connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(capNetworkReply(QNetworkReply*)));
        mNetworkManager->get(QNetworkRequest(capUrl));
        _splitting = 5; emit splittingChanged();
        return;
    }
    splitThread->start();
}

void MainNet::capNetworkReply(QNetworkReply* reply) {
    if(reply->error() == QNetworkReply::NoError) {
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt() == 200) {
            QFile capFile(capPath());
            capFile.open(QIODevice::WriteOnly);
            capFile.write(reply->readAll());
            capFile.close();
            _splitting = 2; emit splittingChanged();
            splitThread->start();
            return;
        }
    }
    splitThread->deleteLater();
}

void MainNet::extractImage(int type, int options)
{
    _options = options;
    _type = type;
    QString filter = "*.exe *.signed";
    if (type == 0 && _options & 1)
        filter += " *.rcfs";
    if (type == 2 || (type == 0 && _options & 2))
        filter += " *.qnx6";
    if (type == 0 && _options & 4)
        filter += " *.ifs";
    FileSelect finder = selectFiles("Extract Image", getSaveDir(), "Filesystem Containers", filter);
#ifdef BLACKBERRY
    QObject::connect(finder, SIGNAL(fileSelected(const QStringList&)), this, SLOT(extractImageSlot(const QStringList&)));
#else
    if (finder->exec())
        extractImageSlot(finder->selectedFiles());
    finder->deleteLater();
#endif
}

void MainNet::extractImageSlot(const QStringList& selectedFiles)
{
    if (selectedFiles.empty())
        return;
    // TODO: Actually detect file by inspection
    QSettings settings("Qtness","Sachesi");
    QFileInfo fileInfo(selectedFiles.first());
    if (_type == 2 && fileInfo.size() < 500 * 1024 * 1024) {
        QString errorMsg = "You can only extract apps from debrick OS images.";
        if (fileInfo.size() < 120 * 1024 * 1024)
            errorMsg.append("\nThis appears to be a Radio file. Radios have no apps.");
        QMessageBox::information(nullptr, "Warning", errorMsg, QMessageBox::Ok);
        return;
    }
    settings.setValue("splitDir", fileInfo.absolutePath());
    _splitting = 3 + (_type == 2); emit splittingChanged();
    splitThread = new QThread;
    Splitter* splitter = new Splitter(selectedFiles.first());
    switch (_type) {
    case 1:
        splitter->extractImage = true;
        break;
    case 2:
        splitter->extractApps = true;
        break;
    case 0:
    default:
        break;
    }
    splitter->extractTypes = _options;
    splitter->moveToThread(splitThread);
    if (fileInfo.suffix() == "ifs")
        connect(splitThread, SIGNAL(started()), splitter, SLOT(processExtractBoot()));
    else if (fileInfo.suffix() == "rcfs")
        connect(splitThread, SIGNAL(started()), splitter, SLOT(processExtractRCFS()));
    else if (fileInfo.suffix() == "qnx6")
        connect(splitThread, SIGNAL(started()), splitter, SLOT(processExtractQNX6()));
    else if (fileInfo.suffix() == "exe")
        connect(splitThread, SIGNAL(started()), splitter, SLOT(processExtractAutoloader()));
    else if (fileInfo.suffix() == "signed")
        connect(splitThread, SIGNAL(started()), splitter, SLOT(processExtractSigned()));
    else // Bar, Zip
        connect(splitThread, SIGNAL(started()), splitter, SLOT(processExtractBar()));
    connect(splitter, SIGNAL(finished()), splitThread, SLOT(quit()));
    connect(splitter, SIGNAL(finished()), this, SLOT(cancelSplit()));
    connect(splitter, SIGNAL(progressChanged(int)), this, SLOT(setSplitProgress(int)));
    connect(splitThread, SIGNAL(finished()), splitter, SLOT(deleteLater()));
    connect(splitThread, SIGNAL(finished()), splitThread, SLOT(deleteLater()));
    splitThread->start();
}

void MainNet::cancelSplit()
{
    _splitting = 0;
    emit splittingChanged();
}

void MainNet::abortSplit()
{
    killSplit();
    cancelSplit();
}

void MainNet::grabLinks(int downloadDevice)
{
    writeDisplayFile("updates.txt", convertLinks(downloadDevice, "Links have been converted to work on your selected device.\n\n").toLocal8Bit());
}

void MainNet::grabPotentialLinks(QString softwareRelease, QString osVersion) {
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(softwareRelease.toLocal8Bit());
    QString hashval = QString(hash.result().toHex());

    QStringList parts = osVersion.split('.');
    // Just a guess that the Radio is +1. In some carrier builds this isn't true.
    int build = parts.last().toInt() + 1;
    QString radioVersion = "";
    for (int i = 0; i < 3; i++)
        radioVersion += parts.at(i) + ".";
    radioVersion += QString::number(build);

    QString potentialText = QString("Potential OS and Radio links for SR" + softwareRelease + " (OS:" + osVersion + " + Radio:" + radioVersion + ")\n\n"
                                    "* Operating Systems *\n");

    // Lambda function to append link for signed bars
    // Arch hardcoded to armv7
    auto appendNewHeader = [&potentialText] (QString name, QString devices) {
        potentialText.append("\n" + name + ": " + devices + " (Debrick + Core OS)\n");
    };
    auto appendNewLink = [&potentialText, &hashval] (QString linkType, bool OS, bool OMAP, QString hwType, QString version) {
        if (!OS)
            potentialText.append(linkType + " Radio\n");
        potentialText.append("http://cdn.fs.sl.blackberry.com/fs/qnx/production/" + hashval + "/com.qnx." + (OS ? "coreos.qcfm.os." : "qcfm.radio."));

        if (OMAP) // Old Playbook style
            potentialText.append("factory" + hwType + "/" + version + "/winchester.factory_sfi" + hwType + "-" + version + "-nto+armle-v7+signed.bar\n");
        else
            potentialText.append(hwType + "/" + version + "/" + hwType + "-" + version + "-nto+armle-v7+signed.bar\n");
    };
    appendNewHeader("QC8974", "Blackberry Passport");
    appendNewLink("Debrick", true, false, "qc8974.factory_sfi.desktop", osVersion);
    appendNewLink("Core",    true, false, "qc8974.factory_sfi", osVersion);

    appendNewHeader("QC8960", "Blackberry Z3/Z10/Z30/Q5/Q10");
    appendNewLink("Debrick", true, false, "qc8960.factory_sfi.desktop", osVersion);
    appendNewLink("Core",    true, false, "qc8960.factory_sfi", osVersion);

    appendNewHeader("OMAP", "Blackberry Z10 STL 100-1");
    appendNewLink("Debrick", true, true, ".desktop", osVersion);
    appendNewLink("Core",    true, true, "", osVersion);

    potentialText.append("\n\n* Radios *\n");
    // Touch
    appendNewLink("Z30", false, false, "qc8960.wtr5", radioVersion);
    appendNewLink("Z10 (STL 100-1)",   false, false, "m5730", radioVersion);
    appendNewLink("Z10 (STL 100-2/3/4) and Porsche P9982", false, false, "qc8960", radioVersion);
    appendNewLink("Z3 (Jakarta)", false, false, "qc8930.wtr5", radioVersion);
    // QWERTY
    appendNewLink("Passport", false, false, "qc8974.wtr2", radioVersion);
    appendNewLink("Q5 and Q10", false, false, "qc8960.wtr", radioVersion);


    writeDisplayFile("versionLookup.txt", potentialText.toLocal8Bit());
}

QString MainNet::fixVariantName(QString name, QString replace, int type) {
    if (type == 0) { // OS
        QString osSignature = "com.qnx.coreos.qcfm.os.";
        QStringList components = name.split(osSignature);
        // Replace first type
        QString newPath = components[0] + osSignature;
        if (replace.startsWith("winchester")) // Old style
            newPath.append("factory");
        else
            newPath.append(replace);

        // Fetch version
        components = components[1].split("/");
        if (components.count() < 2) {
            QMessageBox::information(nullptr, "Error", "Was unable to convert the OS to your selected device! Falling back to original search results.");
            return name;
        }
        if (components[0].endsWith(".desktop"))
            newPath.append(".desktop");
        newPath.append(QString("/") + components[1] + "/" + replace);

        // Replace last type
        components = components[2].split("-");
        if (components[0].endsWith(".desktop"))
            newPath.append(".desktop");
        for (int i = 1; i < components.count(); i++)
            newPath.append(QString("-") + components[i]);

        return newPath;
    } else if (type == 1) { // Radio
        QString radioSignature = "com.qnx.qcfm.radio.";
        QStringList components = name.split(radioSignature);
        QString newPath = components[0] + radioSignature + replace + "/";

        // Fetch version
        components = components[1].split("/");
        if (components.count() < 2) {
            QMessageBox::information(nullptr, "Error", "Was unable to convert the Radio to your selected device! Falling back to original search results.");
            return name;
        }
        newPath.append(components[1] + "/" + replace);

        // Replace last type
        components = components[2].split("-");
        for (int i = 1; i < components.count(); i++)
            newPath.append(QString("-") + components[i]);

        return newPath;
    }
}

// Permanently converts the currentDownload object apps to the current 'Download Device'
// Important not to change this object during the, rather large, download!
void MainNet::fixApps(int downloadDevice) {
    QPair<QString,QString> results = _i->getConnected(downloadDevice);
    if (results.first == "" || results.second == "")
        return;

    for(Apps& app : currentDownload->apps) {
        if (app.type() == "os")
            app.setPackageId(fixVariantName(app.packageId(), results.first, 0));
        else if (app.type() == "radio")
            app.setPackageId(fixVariantName(app.packageId(), results.second, 1));
    }
}


// Creates a string with a list of URLs based on current 'Search Device'
// and converted to current 'Download Device'
QString MainNet::convertLinks(int downloadDevice, QString prepend)
{
    bool convert = true;
    QPair<QString,QString> results = _i->getConnected(downloadDevice);
    if (results.first == "" || results.second == "")
        convert = false;

    QString updated;
    for(Apps* app : _updateAppList) {
        if (!app->isMarked())
            continue;
        QString item = app->packageId();
        if (convert) {
            if (app->type() == "os")
                item = fixVariantName(item, results.first, 0);
            else if (app->type() == "radio")
                item = fixVariantName(item, results.second, 1);
        }

        updated.append(item + "\n");
    }

    if (convert)
        updated.prepend(prepend);
    return updated;
}

void MainNet::downloadLinks(int downloadDevice)
{
    if (_dlProgress < 0 && !_downloading)
    {
        if (_currentFile.isOpen())
        {
            _currentFile.close();
            _currentFile.remove();
            qNetSafeFree(replydl);
            setDownloading(false);
            return;
        }
        delete currentDownload;
        currentDownload = new DownloadInfo(_versionRelease);
        currentDownload->setApps(_updateAppList);
        fixApps(downloadDevice);
        _currentId = 0; emit currentIdChanged();
        // NOTE: Guide only! Maybe we should ask server for real filesizes beforehand
        _dlTotal = 0;
        _dlBytes = 0;
        _maxId = 0;
        for(int i = 0; i < currentDownload->apps.count(); i++) {
            QString fileName = currentDownload->baseDir + "/" + currentDownload->getUrl(i).split("/").last();
            // Either check signature or size to confirm?
            if (QFile::exists(fileName)) {
                currentDownload->apps.removeAt(i);
            } else {
                _dlTotal += currentDownload->getSize(i);
                _maxId++;
            }
        }
        if (_maxId == 0)
            return;
        emit maxIdChanged();
        setDLProgress(0);

        QDir firmware(currentDownload->baseDir);
        firmware.mkpath(".");
        _currentFile.setFileName(currentDownload->baseDir + "/" + currentDownload->getUrl(_currentId).split("/").last());
        emit currentFileChanged();
        _currentFile.open(QIODevice::WriteOnly);
        setDownloading(true);
        QNetworkRequest request;
        request.setUrl(QUrl(currentDownload->getUrl(_currentId)));
        replydl = manager->get(request);
        connect(replydl, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(abortDL(QNetworkReply::NetworkError)));
        connect(replydl, SIGNAL(readyRead()), this, SLOT(downloadLinks()));
        connect(replydl, SIGNAL(finished()), this, SLOT(downloadFinish()));
    }
    else if (_downloading) {
        QByteArray data = replydl->readAll();
        if (_dlBytes == 0)
        {
            if (data.startsWith("<?xml")) {
                QMessageBox::critical(nullptr, "Error", "You are restricted from downloading this file.");
                _currentFile.close();
                _currentFile.remove();
                qNetSafeFree(replydl);
                setDownloading(false);
                return;
            }
        }
        _dlBytes += data.size();
        setDLProgress((int)100.0*((double)(_dlBytes)/(double)_dlTotal));
        _currentFile.write(data);
    }
}
void MainNet::downloadFinish()
{
    if (!_downloading)
        return;

    if (_currentId != (_maxId - 1))
    {
        _currentFile.close();

        _currentId++; emit currentIdChanged();
        _currentFile.setFileName(currentDownload->baseDir + "/" + currentDownload->getUrl(_currentId).split("/").last());
        _currentFile.open(QIODevice::WriteOnly);
        QNetworkRequest request;
        request.setUrl(QUrl(currentDownload->getUrl(_currentId)));
        replydl = manager->get(request);
        connect(replydl, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(abortDL(QNetworkReply::NetworkError)));
        connect(replydl, SIGNAL(readyRead()), this, SLOT(downloadLinks()));
        connect(replydl, SIGNAL(finished()), this, SLOT(downloadFinish()));
    } else {
        // May not be 100% equal to _dlTotal because we switched out some files actually
        _currentFile.close();
        setDownloading(false);
        setDLProgress(-1);
        QDesktopServices::openUrl(QUrl(currentDownload->baseDir));
    }
}

void MainNet::abortDL(QNetworkReply::NetworkError)
{
    if (_currentFile.isOpen())
    {
        _currentFile.close();
        _currentFile.remove();
        qNetSafeFree(replydl);
    }
    setDownloading(false);
}

QString MainNet::NPCFromLocale(int carrier, int country) {
    QString homeNPC;
    homeNPC.sprintf("%03d%03d%d", carrier, country, carrier ? 30 : 60);
    return homeNPC;
}

static QStringList dev[] = {
    // 0 = Z30 (A Series)
    QStringList() << "STA 100-1" << "STA 100-2" << "STA 100-3" << "STA 100-4" << "STA 100-5" << "STA 100-6",
    QStringList() << "8C00240A" << "8D00240A" << "8E00240A" << "8F00240A" << "9500240A" << "B500240A",
    // 1 = Z10 (L Series) OMAP
    QStringList() << "STL 100-1",
    QStringList() << "4002607",
    // 2 = Z10 (L Series) Qualcomm + P9982  (K Series)
    QStringList() << "STL 100-2" << "STL 100-3" << "STL 100-4" << "STK 100-1" << "STK 100-2",
    QStringList() << "8700240A" << "8500240A" << "8400240A" << "A500240A" << "A600240A",
    // 3 = Z3  (J Series)
    QStringList() << "STJ 100-1",
    QStringList() << "04002E07",
    // 4 = Passport / Q30 (W Series)
    QStringList() << "SQW 100-1" << "SQW 100-2" << "SQW 100-3" << "Variant D",
    QStringList() << "87002C0A" << "85002C0A" << "84002C0A" << "86002C0A",
    // 5 = Q5 (R Series) + Q10 (N Series)
    QStringList() << "SQR 100-1" << "SQR 100-2" << "SQR 100-3" << "SQN 100-1" << "SQN 100-2" << "SQN 100-3" << "SQN 100-4" << "SQN 100-5",
    QStringList() << "84002A0A" << "85002A0A" << "86002A0A" << "8400270A" << "8500270A" << "8600270A" << "8C00270A" << "8700270A",
    // 6 = B Series
    QStringList() << "STB 100-1" << "STB 100-2" << "STB 100-3" << "STB 100-4" << "STB 100-5",
    QStringList() << "9700240A" << "9600240A" << "A700240A" << "AC00240A" << "9C00240A",
    // 7 = Cafe Series
    QStringList() << "SQC 100-1" << "SQC 100-2",
    QStringList() << "87002A07" << "8C002A07",
    // 8 = Dev Alpha
    QStringList() << "Alpha A" << "Alpha B" << "Alpha C",
    QStringList() << "4002307" << "4002607" << "8D00270A",
    // 10 = Ontario Series
    QStringList() << "STO 100-1" << "STO 100-2" << "STO 100-3" << "STO 100-4",
    QStringList() << "AE00240A" << "AF00240A" << "B400240A" << "B600240A",
    // 11 = Classic
    QStringList() << "Classic Variant A" << "Classic Variant B" << "Classic Variant C" << " Classic Variant D",
    QStringList() << "9400270A" << "9500270A" << "9600270A" << "9700270A",
    // 12 = Khan
    QStringList() << "Khan Variant A" << "Khan Variant B",
    QStringList() << "8E00270A" << "8F00270A",
};

QString MainNet::nameFromVariant(unsigned int device, unsigned int variant) {
    Q_ASSERT(variantCount(device) > variant);
    return dev[device*2][variant];
}
QString MainNet::hwidFromVariant(unsigned int device, unsigned int variant) {
    Q_ASSERT(variantCount(device) > variant);
    return dev[device*2+1][variant];
}
unsigned int MainNet::variantCount(unsigned int device) {
    return dev[device*2].count();
}

void MainNet::reverseLookup(int device, int variant, int server, QString OSver, bool skip)
{
    if (_scanning)
        return;

    _softwareRelease = "Asking server..."; emit softwareReleaseChanged();
    _hasPotentialLinks = false; emit hasPotentialLinksChanged();
    setScanning(1);
    QString id = hwidFromVariant(device, variant);
    QString requestUrl;

    switch (server)
    {
    case 1:
        requestUrl = "https://beta.sl.eval.blackberry.com/slscse/srVersionLookup/";
        break;
    case 0:
    default:
        requestUrl = "https://cs.sl.blackberry.com/cse/srVersionLookup/";
        break;
    }
    requestUrl += "2.0/";
    //
    //0x8d00240a
    QString query = "<srVersionLookupRequest version=\"2.0.0\" authEchoTS=\"1366644680359\">"
            "<clientProperties><hardware>"
            "<pin>0x2FFFFFB3</pin><bsn>1140011878</bsn><imei>004402242176786</imei><id>0x"+id+"</id>"
            "</hardware>"
            "<software><currentLocale>en_US</currentLocale><legalLocale>en_US</legalLocale>"
            "<osVersion>"+OSver+"</osVersion></software></clientProperties>"
            "</srVersionLookupRequest>";
    QNetworkRequest request;
    request.setRawHeader("Content-Type", "text/xml;charset=UTF-8");
    request.setUrl(QUrl(requestUrl));
    request.setAttribute(QNetworkRequest::CustomVerbAttribute, skip);
    QNetworkReply* reply = manager->post(request, query.toUtf8());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(serverError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished()), this, SLOT(reverseLookupReply()));
}

void MainNet::reverseLookupReply() {
    QNetworkReply* reply = (QNetworkReply*)sender();
    bool skip = reply->request().attribute(QNetworkRequest::CustomVerbAttribute).toBool();
    QString swRelease;
    QByteArray data = reply->readAll();
    //for (int i = 0; i < data.size(); i += 3000) qDebug() << data.mid(i, 3000);
    QXmlStreamReader xml(data);
    while(!xml.atEnd() && !xml.hasError()) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "softwareReleaseVersion") {
                swRelease = xml.readElementText();
            }
        }
        xml.readNext();
    }
    sender()->deleteLater();

    // Now verify the link
    if (swRelease.at(0).isDigit()) {
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(swRelease.toLocal8Bit());
        QString server = "http://cdn.fs.sl.blackberry.com/fs/qnx/production/";
        if (reply->url().host().startsWith("beta")) {
            server = "http://cdn.fs.sl.blackberry.com/fs/qnx/beta/";
        }
        QString url = server + QString(hash.result().toHex());
        QNetworkRequest request;
        request.setRawHeader("Content-Type", "text/xml;charset=UTF-8");
        request.setUrl(QUrl(url));
        request.setAttribute(QNetworkRequest::CustomVerbAttribute, swRelease);
        QNetworkReply* replyTmp = manager->get(request);
        if (skip)
            connect(replyTmp, SIGNAL(finished()), this, SLOT(confirmNewSRSkip()));
        else
            connect(replyTmp, SIGNAL(finished()), this, SLOT(confirmNewSR()));
    } else {
        _softwareRelease = swRelease; emit softwareReleaseChanged();
        setScanning(0);
    }
}

void MainNet::confirmNewSRSkip() {
    QNetworkReply* reply = (QNetworkReply*)sender();
    QString swRelease = reply->request().attribute(QNetworkRequest::CustomVerbAttribute).toString();
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 404) {
        _softwareRelease = "SR not in system"; emit softwareReleaseChanged();
    } else {
        _softwareRelease = swRelease; emit softwareReleaseChanged();
        _hasPotentialLinks = true; emit hasPotentialLinksChanged();
    }
    setScanning(0);
    sender()->deleteLater();
}

void MainNet::confirmNewSR()
{
    QNetworkReply* reply = (QNetworkReply*)sender();
    QString swRelease = reply->request().attribute(QNetworkRequest::CustomVerbAttribute).toString();
    _softwareRelease = swRelease; emit softwareReleaseChanged();
    // Seems to give 301 redirect if it's real
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 404) {
        _hasPotentialLinks = true; emit hasPotentialLinksChanged();
    }
    setScanning(0);
    sender()->deleteLater();
}

void MainNet::updateDetailRequest(QString delta, QString carrier, QString country, int device, int variant, int mode, int server)
{
    QString up, requestUrl;

    switch (server)
    {
    case 1:
        requestUrl = "https://beta.sl.eval.blackberry.com/slscse/updateDetails/";
        break;
    case 0:
    default:
        requestUrl = "https://cs.sl.blackberry.com/cse/updateDetails/";
        break;
    }
    /*
    switch(version)
    {
    case 2:
        requestUrl += "1.0.0/";
        break;
    case 1:
        requestUrl += "2.0.0/";
        break;
    case 0:
    default:
        requestUrl += "2.1.0/";
        break;
    }*/
    requestUrl += "2.2/";

    QString homeNPC = NPCFromLocale(carrier.toInt(), country.toInt());

    switch (mode)
    {
    case 1:
        up = "repair";
        break;
    case 0:
    default:
        up = "upgrade";
        break;
    }

    // We either selected 'Any' (if there was more than one variant) or we picked a specific variant.
    int start = (variant != 0) ? (variant - 1) : 0;
    int end   = (variant != 0) ? variant : variantCount(device);
    setScanning((variant != 0) ? 1 : variantCount(device));
    if (_scanning > 1)
        setMultiscan(true);
    for (int i = start; i < end; i++) {
        QString query = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<updateDetailRequest version=\"2.2.0\" authEchoTS=\"1361763056140\">"
                "<clientProperties>"
                "<hardware>"
                "<pin>0x2FFFFFB3</pin><bsn>1128121361</bsn><imei>004401139269240</imei><id>0x"+hwidFromVariant(device, i)+"</id><isBootROMSecure>true</isBootROMSecure>"
                "</hardware>"
                "<network>"
                "<vendorId>0x0</vendorId><homeNPC>0x"+homeNPC+"</homeNPC><iccid>89014104255505565333</iccid><msisdn>15612133940</msisdn><imsi>310410550556533</imsi><ecid>0x0</ecid>"
                "</network>"
                "<software>"
                "<currentLocale>en_US</currentLocale><legalLocale>en_US</legalLocale><osVersion>10.0.0.0</osVersion><radioVersion>10.0.0.0</radioVersion>"
                "</software>"
                "</clientProperties>"
                "<updateDirectives><allowPatching type=\"REDBEND\">true</allowPatching><upgradeMode>"+up+"</upgradeMode><provideDescriptions>true</provideDescriptions><provideFiles>true</provideFiles><queryType>NOTIFICATION_CHECK</queryType></updateDirectives>"
                "<pollType>manual</pollType>"
                "<resultPackageSetCriteria>"
                "<softwareRelease softwareReleaseVersion=\"latest\" />"
                "<releaseIndependent><packageType operation=\"include\">application</packageType></releaseIndependent>"
                "</resultPackageSetCriteria>" + delta +
                "</updateDetailRequest>";
        _error = ""; emit errorChanged();
        // Pass the variant in the request so it can be retrieved out-of-order
        QNetworkRequest request;
        request.setRawHeader("Content-Type", "text/xml;charset=UTF-8");
        request.setUrl(QUrl(requestUrl));
        request.setAttribute(QNetworkRequest::CustomVerbAttribute, nameFromVariant(device, i));
        QNetworkReply* reply = manager->post(request, query.toUtf8());
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(serverError(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(finished()), this, SLOT(serverReply()));
    }
}

void MainNet::serverReply()
{
    QNetworkReply *reply = (QNetworkReply *)sender();
    QByteArray data = reply->readAll();
    //for (int i = 0; i < data.size(); i += 3000) qDebug() << data.mid(i, 3000);
    showFirmwareData(data, reply->request().attribute(QNetworkRequest::CustomVerbAttribute).toString());
    sender()->deleteLater();
}

void MainNet::showFirmwareData(QByteArray data, QString variant)
{
    QXmlStreamReader xml(data);
    QString ver = "";
    QString os = "";
    QString radio = "";
    QString desc = "";
    QString addr = "";
    QString currentaddr = "";
    QList<Apps*> newApps;
    while(!xml.atEnd() && !xml.hasError()) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "package")
            {
                Apps* newApp = new Apps();
                newApp->setIsMarked(true);
                // Remember: this name *can* change
                newApp->setFriendlyName(xml.attributes().value("name").toString());
                // Remember: this size *can* change
                newApp->setSize(xml.attributes().value("downloadSize").toString().toInt());
                // Remember: this name *can* change
                newApp->setName(xml.attributes().value("path").toString());
                newApp->setVersion(xml.attributes().value("version").toString());
                newApp->setVersionId(xml.attributes().value("id").toString());
                QString type = xml.attributes().value("type").toString();
                if (type == "system:os" || type == "system:desktop") {
                    os = newApp->name().split('/').at(1);
                    newApp->setType("os");
                } else if (type == "system:radio") {
                    radio = newApp->name().split('/').at(1);
                    newApp->setType("radio");
                } else {
                    newApp->setType("application");
                }
                newApp->setPackageId(currentaddr + "/" + newApp->name());
                newApps.append(newApp);
            }
            else if (xml.name() == "friendlyMessage")
            {
                _error = xml.readElementText().split(QChar('.'))[0]; emit errorChanged();
            }
            else if (xml.name() == "fileSet")
            {
                currentaddr = xml.attributes().value("url").toString();
                if (addr == "")
                    addr = currentaddr;
            }
            else if (xml.name() == "softwareReleaseMetadata")
            {
                ver = xml.attributes().value("softwareReleaseVersion").toString();
            }
            else if (xml.name() == "bundle")
            {
                QString newver = xml.attributes().value("version").toString();
                if (ver == "" || (ver.split(".").last().toInt() < newver.split(".").last().toInt()))
                    ver = newver;
            }
            else if (xml.name() == "description" && desc == "")
            {
                desc = xml.readElementText();
            }
        }
        xml.readNext();
    }
    // Check if the version string is newer. Not sure if QString compare would work
    bool isNewer = !_multiscan || _multiscanVersion == "";
    if (!isNewer && ver != "") {
        QStringList newVer = ver.split('.');
        QStringList oldVer = _multiscanVersion.split('.');
        for (int i = 0; i < 4; i++) {
            int newBuild = newVer.at(i).toInt();
            int oldBuild = oldVer.at(i).toInt();
            if (newBuild > oldBuild)
                isNewer = true;
            if (newBuild != oldBuild)
                break;
        }
    }
    if (isNewer) {
        // Update software release versions
        if (_multiscan)
            _multiscanVersion = ver;
        _versionRelease = ver;
        // Server uses some funny order. Put in order of largest to smallest.
        std::sort(newApps.begin(), newApps.end(),
                  [=](const Apps* i, const Apps* j) { return i->size() > j->size(); });

        // Put this new list up for display
        if (_updateAppList.count()) {
            for (Apps* app: _updateAppList)
                delete app;
            _updateAppList.clear();
        }
        _updateAppList = newApps;
        // Connect every isMarkedChanged to the list signal
        for (Apps* app: _updateAppList) {
            connect(app, SIGNAL(isMarkedChanged()), this, SIGNAL(updateCheckedCountChanged()));
        }
        _updateMessage = QString("<b>Update %1 available for %2!</b><br>%3 %4<br><br>%5")
                .arg(ver)
                .arg(variant)
                .arg(os != "" ? QString("<b> OS: %1</b>").arg(os) : "")
                .arg(radio != "" ? QString("<b> Radio: %1</b>").arg(radio) : "")
                .arg(desc);
        emit updateMessageChanged();
        emit updateCheckedCountChanged();
        _error = ""; emit errorChanged();
    }
    setScanning(_scanning-1);
    if (_scanning <= 0)
        setMultiscan(false);
}

void MainNet::serverError(QNetworkReply::NetworkError err)
{
    setScanning(_scanning-1);
    // Only show error if we are doing single scan or multiscan version is empty.
    if (!_multiscan || (_multiscanVersion == "" && _scanning == 0)) {
        QString errormsg = QString("Error %1 (%2)")
                .arg(err)
                .arg( ((QNetworkReply*)sender())->errorString() );
        _error = errormsg;
        emit errorChanged();
        _updateMessage = ""; emit updateMessageChanged();
    }
    if (_scanning == 0)
        setMultiscan(false);
}

void MainNet::setDLProgress(const int &progress) { _dlProgress = progress; emit dlProgressChanged(); }
void MainNet::setMultiscan(const bool &multiscan) {
    _multiscan = multiscan; emit multiscanChanged();
    _multiscanVersion = ""; emit updateMessageChanged();
}
void MainNet::setScanning(const int &scanning) { _scanning = scanning; emit scanningChanged(); }
void MainNet::setDownloading(const bool &downloading) { _downloading = downloading; emit downloadingChanged(); }
void MainNet::setSplitProgress(const int &progress) { if (_splitProgress > 1000) _splitProgress = 0; else _splitProgress = progress; emit splitProgressChanged(); }

QString MainNet::currentFile() const { QString ret = _currentFile.fileName().split("/").last(); if (ret.length() > 30) ret.truncate(30); return ret; }
