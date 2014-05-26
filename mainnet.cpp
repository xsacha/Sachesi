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

#include "mainnet.h"
#include <QFileDialog>
#include <QListView>
#include <QTreeView>
#include <QMessageBox>
#include "splitter.h"
#include <QDesktopServices>

#ifdef BLACKBERRY
#define SAVE_DIR settings.value("splitDir", "/accounts/1000/shared/misc/Sachesi/").toString()
#include <bb/cascades/pickers/FilePicker>
#include <bb/system/Clipboard>
#else
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#define SAVE_DIR settings.value("splitDir", QStandardPaths::standardLocations(QStandardPaths::DesktopLocation)).toString()
#else
#include <QDesktopServices>
#define SAVE_DIR settings.value("splitDir", QDesktopServices::storageLocation(QDesktopServices::DesktopLocation)).toString()
#endif
#endif

MainNet::MainNet( QObject* parent) : QObject(parent)
{
    manager = new QNetworkAccessManager();
    _softwareRelease = "";
    _versionRelease = "";
    _versionOS = "";
    _versionRadio = "";
    _scanning = false;
    _currentId = 0; _maxId = 0;
    _splitting = 0;
	_splitProgress = 0;
    _downloading = false;
    _dlProgress = -1;
    QSettings settings("Qtness","Sachesi");
#ifdef BLACKBERRY
	setAdvanced(true);
#else
    setAdvanced(settings.value("advanced", false).toBool());
#endif
    reply = NULL;
    replydl = NULL;
}

MainNet::~MainNet()
{
}

void MainNet::splitAutoloader(int options)
{
    QSettings settings("Qtness","Sachesi");
	_options = options;
#ifdef BLACKBERRY
	bb::cascades::pickers::FilePicker* filePicker = new bb::cascades::pickers::FilePicker();
	filePicker->setDirectories(QStringList() << SAVE_DIR);
	filePicker->setFilter(QStringList() << "*.exe" << "*.bar" << "*.zip");
	filePicker->setTitle("Split Autoloader");
	filePicker->setMode(bb::cascades::pickers::FilePickerMode::Picker);
	filePicker->open();
	QObject::connect(filePicker, SIGNAL(fileSelected(const QStringList&)), this, SLOT(splitAutoloaderSlot(const QStringList&)));
#else
    QFileDialog finder;
    finder.setDirectory(SAVE_DIR);
    finder.setWindowTitle("Split Autoloader");
    finder.setNameFilter("Signed Container (*.exe *.bar *.zip)");
    if (finder.exec())
    {
        if (finder.selectedFiles().empty())
            return;
        QFileInfo fileInfo(finder.selectedFiles().first());
        settings.setValue("splitDir", fileInfo.absolutePath());
        _splitting = 1; emit splittingChanged();
        splitThread = new QThread();
        Splitter* splitter = new Splitter(finder.selectedFiles().first(), _options);
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
#endif
}

void MainNet::splitAutoloaderSlot(const QStringList& fileNames) {
    QSettings settings("Qtness","Sachesi");
	if (fileNames.isEmpty())
		return;
	QFileInfo fileInfo(fileNames.first());
    settings.setValue("splitDir", fileInfo.absolutePath());
    _splitting = 1; emit splittingChanged();
    splitThread = new QThread();
    Splitter* splitter = new Splitter(fileNames.first(), _options);
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

void MainNet::combineFolder()
{
    QSettings settings("Qtness","Sachesi");
    QFileDialog finder;
    finder.setFileMode(QFileDialog::Directory);
    finder.setWindowTitle("Combine Folder of Signed");
    finder.setDirectory(SAVE_DIR);
    finder.setNameFilter("Signed Images (*.signed)");
    QListView *l = finder.findChild<QListView*>("listView");
    if (l)
        l->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QTreeView *t = finder.findChild<QTreeView*>();
    if (t)
        t->setSelectionMode(QAbstractItemView::ExtendedSelection);
    if (finder.exec())
        combineAutoloader(finder.selectedFiles());
}

void MainNet::combineFiles()
{
    QSettings settings("Qtness","Sachesi");
#ifdef BLACKBERRY
	bb::cascades::pickers::FilePicker* filePicker = new bb::cascades::pickers::FilePicker();
	filePicker->setDirectories(QStringList() << SAVE_DIR);
	filePicker->setFilter(QStringList() << "*.signed");
	filePicker->setTitle("Combine Signed Files");
	filePicker->setMode(bb::cascades::pickers::FilePickerMode::Picker);
	filePicker->open();
	QObject::connect(filePicker, SIGNAL(fileSelected(const QStringList&)), this, SLOT(combineAutoloader(const QStringList&)));
#else
    QFileDialog finder;
    finder.setWindowTitle("Combine Signed Files");
    finder.setDirectory(SAVE_DIR);
    finder.setNameFilter("Signed Images (*.signed)");
    QListView *l = finder.findChild<QListView*>("listView");
    if (l)
        l->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QTreeView *t = finder.findChild<QTreeView*>();
    if (t)
        t->setSelectionMode(QAbstractItemView::ExtendedSelection);
    if (finder.exec())
        combineAutoloader(finder.selectedFiles());
#endif
}

void MainNet::combineAutoloader(QStringList selectedFiles)
{
    QSettings settings("Qtness","Sachesi");
        QFileInfo fileInfo(selectedFiles.first());
        settings.setValue("splitDir", fileInfo.absolutePath());
        QStringList splitFiles = QStringList();
        foreach(QString fileName, selectedFiles) {
            if (QFileInfo(fileName).isDir())
            {
                QStringList suffixOnly = QDir(fileName).entryList(QStringList("*.signed"));
                foreach (QString suffix, suffixOnly) {
                    splitFiles.append(fileName + "/" + suffix);
                }
            } else if (fileName.endsWith(".signed"))
                splitFiles.append(fileName);
        }
        if (splitFiles.isEmpty())
            return;
        settings.setValue("splitDir", QFileInfo(selectedFiles.first()).absolutePath());
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
#ifdef BLACKBERRY
		QFileInfo capFile("/accounts/1000/shared/misc/Sachesi/cap.exe");
#else
        QSettings ini(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
        QFileInfo capFile(QFileInfo(ini.fileName()).absolutePath()+"/Sachesi/cap.exe");
#endif
        if (!capFile.exists())
        {
            QString capUrl = "http://ppsspp.mvdan.cc/PPSSPP-0.8.1-s.bar";
            QNetworkAccessManager* mNetworkManager = new QNetworkAccessManager(this);
            QObject::connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(capNetworkReply(QNetworkReply*)));
            /*QNetworkReply* reply =*/ mNetworkManager->get(QNetworkRequest(capUrl));
            _splitting = 5; emit splittingChanged();
            return;
        }
        splitThread->start();
}

void MainNet::capNetworkReply(QNetworkReply* reply) {
    if(reply->error() == QNetworkReply::NoError) {
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt() == 200) {
            QSettings ini(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
#ifdef BLACKBERRY
            QString capPath = "/accounts/1000/shared/misc/";
#else
            QString capPath = QFileInfo(ini.fileName()).absolutePath();
#endif
            QDir capDir(capPath); capDir.mkpath("Sachesi");
            QFile capFile(capPath+"/Sachesi/cap.exe");
            capFile.open(QIODevice::WriteOnly);
            capFile.write(reply->readAll());
            capFile.close();
        } else
            splitThread->deleteLater();
    } else
        splitThread->deleteLater();
    _splitting = 2; emit splittingChanged();
    splitThread->start();
}

void MainNet::extractImage(int type, int options)
{
    QSettings settings("Qtness","Sachesi");
	_options = options;
	_type = type;
#ifdef BLACKBERRY
	bb::cascades::pickers::FilePicker* filePicker = new bb::cascades::pickers::FilePicker();
	filePicker->setDirectories(QStringList() << SAVE_DIR);
	QStringList filter;
	filter << "*.exe" << "*.signed";
    if (type == 0 && _options & 1)
		filter << "*.rcfs";
    if (type == 2 || (type == 0 && _options & 2))
        filter << " *.qnx6";
	filePicker->setFilter(filter);
	filePicker->setTitle("Extract Image");
	filePicker->setMode(bb::cascades::pickers::FilePickerMode::Picker);
	filePicker->open();
	QObject::connect(filePicker, SIGNAL(fileSelected(const QStringList&)), this, SLOT(extractImageSlot(const QStringList&)));
#else
    QFileDialog finder;
    finder.setWindowTitle("Extract Image");
    finder.setDirectory(SAVE_DIR);
    QString filter = "*.exe *.signed";
    if (type == 0 && _options & 1)
        filter += " *.rcfs";
    if (type == 2 || (type == 0 && _options & 2))
        filter += " *.qnx6";
    finder.setNameFilter("Filesystem Containers ("+ filter +")");
    if (finder.exec())
    {
        if (finder.selectedFiles().empty())
            return;
        QFileInfo fileInfo(finder.selectedFiles().first());
        if (_type == 2 && fileInfo.size() < 500 * 1024 * 1024) {
            QString errorMsg = "You can only extract apps from debrick OS images.";
            if (fileInfo.size() < 50 * 1024 * 1024)
                errorMsg.append("\nThis appears to be a Radio file. Radios have no apps.");
            QMessageBox::information(NULL, "Warning", errorMsg, QMessageBox::Ok);
            return;
        }
        settings.setValue("splitDir", fileInfo.absolutePath());
        _splitting = 3 + (_type == 2); emit splittingChanged();
        splitThread = new QThread;
        Splitter* splitter = new Splitter(finder.selectedFiles().first());
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
        if (fileInfo.suffix() == "rcfs")
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
#endif
}

void MainNet::extractImageSlot(const QStringList& selectedFiles)
{
    QSettings settings("Qtness","Sachesi");
        if (selectedFiles.empty())
            return;
        QFileInfo fileInfo(selectedFiles.first());
        if (_type == 2 && fileInfo.size() < 500 * 1024 * 1024) {
            QString errorMsg = "You can only extract apps from debrick OS images.";
            if (fileInfo.size() < 50 * 1024 * 1024)
                errorMsg.append("\nThis appears to be a Radio file. Radios have no apps.");
            QMessageBox::information(NULL, "Warning", errorMsg, QMessageBox::Ok);
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
        if (fileInfo.suffix() == "rcfs")
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
    _splitting = 0; emit splittingChanged();
}

void MainNet::abortSplit()
{
    if (splitThread)
        splitThread->quit();
    cancelSplit();
}

void MainNet::grabLinks()
{
#ifdef BLACKBERRY
    bb::system::Clipboard clipboard;
    clipboard.clear();
    clipboard.insert("text/plain", _links.toLocal8Bit());
    return;
#endif

#ifdef ANDROID
    QSettings settings("Qtness","Sachesi");
    QFile updates(SAVE_DIR + "/updates.txt");
#else
    QFile updates("updates.txt");
#endif
    if (!updates.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    updates.write(_links.toLatin1());
    updates.close();

    QProcess wordpad;
#ifdef _WIN32
    wordpad.startDetached("explorer updates.txt");
#elif defined(__APPLE__)
    wordpad.startDetached("open updates.txt");
/*#elif defined(BLACKBERRY)
    QVariantMap data;
    data["title"] = "Links";
    bb::cascades::Invocation* invocation = bb::cascades::Invocation::create(
                bb::cascades::InvokeQuery::create().invokeActionId("bb.action.SHARE").metadata(data).uri("file:///accounts/1000/shared/misc/updates.txt").invokeTargetId(
                                "sys.pim.remember.composer"));
    connect(invocation, SIGNAL(finished()), invocation, SLOT(deleteLater()));
*/
#elif defined(ANDROID)
    QDesktopServices::openUrl(QUrl::fromLocalFile(SAVE_DIR + "/updates.txt"));
#else
    wordpad.startDetached("xdg-open updates.txt");
#endif
}

void MainNet::grabPotentialLinks(QString softwareRelease, QString osVersion) {
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(softwareRelease.toLocal8Bit());
    QString hashval = QString(hash.result().toHex());

    QStringList parts = osVersion.split('.');
    int build = parts.last().toInt() + 1;
    QString radioVersion = "";
    for (int i = 0; i < 3; i++)
        radioVersion += parts.at(i) + ".";
    radioVersion += QString::number(build);

    QString potentialText = QString(
                "* Operating Systems *\n"
                "OMAP Devices (STL 100-1)\n"
                "Debrick OS: http://cdn.fs.sl.blackberry.com/fs/qnx/production/" + hashval + "/com.qnx.coreos.qcfm.os.factory.desktop/" + osVersion + "/winchester.factory_sfi.desktop-" + osVersion + "-nto+armle-v7+signed.bar\n"
                "Core OS   : http://cdn.fs.sl.blackberry.com/fs/qnx/production/" + hashval + "/com.qnx.coreos.qcfm.os.factory/" + osVersion + "/winchester.factory_sfi-" + osVersion + "-nto+armle-v7+signed.bar\n"
                "\n"
                "Qualcomm Devices (Everyone else)\n"
                "Debrick OS: http://cdn.fs.sl.blackberry.com/fs/qnx/production/" + hashval + "/com.qnx.coreos.qcfm.os.qc8960.factory_sfi.desktop/" + osVersion + "/qc8960.factory_sfi.desktop-" + osVersion + "-nto+armle-v7+signed.bar\n"
                "Core OS   : http://cdn.fs.sl.blackberry.com/fs/qnx/production/" + hashval + "/com.qnx.coreos.qcfm.os.qc8960.factory_sfi/" + osVersion + "/qc8960.factory_sfi-" + osVersion + "-nto+armle-v7+signed.bar\n"
                "\n\n"
                "* Radios *\n"
                "Z10 (-1): http://cdn.fs.sl.blackberry.com/fs/qnx/production/" + hashval + "/com.qnx.qcfm.radio.m5730/" + radioVersion + "/m5730-" + radioVersion + "-nto+armle-v7+signed.bar\n"
                "Z10 (-2/3/4) and Z5 (Porsche): http://cdn.fs.sl.blackberry.com/fs/qnx/production/" + hashval + "/com.qnx.qcfm.radio.qc8960/" + radioVersion + "/qc8960-" + radioVersion + "-nto+armle-v7+signed.bar\n"
                "Z30: http://cdn.fs.sl.blackberry.com/fs/qnx/production/" + hashval + "/com.qnx.qcfm.radio.qc8960.wtr5/" + radioVersion + "/qc8960.wtr5-" + radioVersion + "-nto+armle-v7+signed.bar\n"
                "Q10: http://cdn.fs.sl.blackberry.com/fs/qnx/production/" + hashval + "/com.qnx.qcfm.radio.qc8960.wtr/" + radioVersion + "/qc8960.wtr-" + radioVersion + "-nto+armle-v7+signed.bar\n"
                "Z3 (Jakarta): http://cdn.fs.sl.blackberry.com/fs/qnx/production/" + hashval + "/com.qnx.qcfm.radio.qc8930.wtr5/" + radioVersion + "/qc8930.wtr5-" + radioVersion + "-nto+armle-v7+signed.bar\n"
                "");
#ifdef BLACKBERRY
    bb::system::Clipboard clipboard;
    clipboard.clear();
    clipboard.insert("text/plain", potentialText.toLocal8Bit());
    return;
#endif

#ifdef ANDROID
    QSettings settings("Qtness","Sachesi");
    QFile updates(SAVE_DIR + "/versionlookup.txt");
#else
    QFile updates("versionlookup.txt");
#endif
    if (!updates.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    updates.write(potentialText.toLocal8Bit());
    updates.close();

    QProcess wordpad;
#ifdef _WIN32
    wordpad.startDetached("explorer versionlookup.txt");
#elif defined(__APPLE__)
    wordpad.startDetached("open versionlookup.txt");
/*#elif defined(BLACKBERRY)
    QVariantMap data;
    data["title"] = "Links";
    bb::cascades::Invocation* invocation = bb::cascades::Invocation::create(
                bb::cascades::InvokeQuery::create().invokeActionId("bb.action.SHARE").metadata(data).uri("file:///accounts/1000/shared/misc/updates.txt").invokeTargetId(
                                "sys.pim.remember.composer"));
    connect(invocation, SIGNAL(finished()), invocation, SLOT(deleteLater()));
*/
#elif defined(ANDROID)
    QDesktopServices::openUrl(QUrl::fromLocalFile(SAVE_DIR + "/versionlookup.txt"));
#else
    wordpad.startDetached("xdg-open versionlookup.txt");
#endif
}

void MainNet::downloadLinks()
{
    if (dlProgress() < 0 && !downloading())
    {
        if (_currentFile.isOpen())
        {
            _currentFile.close();
            _currentFile.remove();
            if (replydl != NULL)
            {
                replydl->abort();
                replydl = NULL;
            }
            setDownloading(false);
            return;
        }
        setDLProgress(0);
        _currentId = 0; emit currentIdChanged();
        _maxId = _sizes.count(); emit maxIdChanged();
        _dlBytes = 0;
        QDir firmware(_versionRelease);
        firmware.mkpath(".");
        QStringList fileList(_links.split( QChar('\n') ));
        _currentFile.setFileName(_versionRelease + "/" + fileList.at(_currentId).split("/").last());
        emit currentFileChanged();
        while (_currentFile.exists() && _currentFile.size() == _sizes.at(_currentId) && _currentId < fileList.count())
        {
            _dlBytes += _sizes.at(_currentId);
            _currentId++; emit currentIdChanged();
            _currentFile.setFileName(_versionRelease + "/" + fileList.at(_currentId).split("/").last());
            emit currentFileChanged();
        }
        if (_currentFile.size() != _sizes.at(_currentId))
            _currentFile.remove();
        _currentFile.open(QIODevice::WriteOnly);
        setDownloading(true);
        QNetworkRequest request;
        request.setUrl(QUrl(fileList.at(_currentId)));
        replydl = manager->get(request);
        connect(replydl, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(abortDL(QNetworkReply::NetworkError)));
        connect(replydl, SIGNAL(readyRead()), this, SLOT(downloadLinks()));
        connect(replydl, SIGNAL(finished()), this, SLOT(downloadFinish()));
    }
    else if (downloading()) {
        QByteArray data = replydl->readAll();
        if (_dlBytes == 0)
        {
            if (data.startsWith("<?xml")) {
                QMessageBox box(QMessageBox::Critical, "Error", "You must be a RIM employee to download this file.");
                box.exec();
                _currentFile.close();
                _currentFile.remove();
                if (replydl != NULL)
                {
                    replydl->abort();
                    replydl = NULL;
                }
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
    if (!downloading())
        return;
    if (_dlBytes == _dlTotal)
    {
        _currentFile.close();
        setDownloading(false);
        setDLProgress(-1);
    }
    else if (_currentId != (_maxId - 1))
    {
        _currentFile.close();

        QStringList fileList(_links.split( QChar('\n') ));
        _currentId++; emit currentIdChanged();
        _currentFile.setFileName(_versionRelease + "/" + fileList.at(_currentId).split("/").last());
        emit currentFileChanged();
        while (_currentFile.exists() && _currentFile.size() == _sizes.at(_currentId) && _currentId < fileList.count())
        {
            _dlBytes += _sizes.at(_currentId);
            _currentId++; emit currentIdChanged();
            _currentFile.setFileName(_versionRelease + "/" + fileList.at(_currentId).split("/").last());
            emit currentFileChanged();
        }
        if (_currentFile.size() != _sizes.at(_currentId))
            _currentFile.remove();
        _currentFile.open(QIODevice::WriteOnly);
        QNetworkRequest request;
        request.setUrl(QUrl(fileList.at(_currentId)));
        replydl = manager->get(request);
        connect(replydl, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(abortDL(QNetworkReply::NetworkError)));
        connect(replydl, SIGNAL(readyRead()), this, SLOT(downloadLinks()));
        connect(replydl, SIGNAL(finished()), this, SLOT(downloadFinish()));
    }
}

void MainNet::abortDL(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    if (_currentFile.isOpen())
    {
        _currentFile.close();
        _currentFile.remove();
        if (replydl != NULL)
        {
            replydl->abort();
            replydl = NULL;
        }
    }
    setDownloading(false);
}

QString MainNet::NPCFromLocale(int carrier, int country) {
    QString homeNPC;
    homeNPC.sprintf("%03d%03d%d",carrier, country, carrier ? 30 : 60);
    return homeNPC;
}

QString MainNet::HWIDFromVariant(int variant) {
    QString id;
    // Device
    // 0 = Z30 (A Series)
    // 1 = Z10 (N Series)
    // 2 = Z5  (K Series)
    // 3 = Z3  (J Series)
    // 4 = Q30 (W Series)
    // 5 = Q10 (L Series)
    // 6 = Q5  (R Series)
    // 7 = B Series
    // 8 = Cafe Series
    // 9 = Dev Alpha
    // 10 = Playbook
    switch (variant)
    {
    case 101:
        id = "D00A106"; // Playbook (4G)
        break;
    case 100:
        id = "6001A06"; // Playbook (Wifi)
        break;
    case 92:
        id = "8D00270A"; // Dev Alpha C
        break;
    case 91:
        id = "4002607"; // Dev Alpha B
        break;
    case 90:
        id = "4002307"; // Dev Alpha A
        break;
    case 81:
        id = "8C002A07"; // SQC 100-2
        break;
    case 80:
        id = "87002A07"; // SQC 100-1
        break;
    case 74:
        id = "9C00240A"; // STB 100-5
        break;
    case 73:
        id = "AC00240A"; // STB 100-4
        break;
    case 72:
        id = "A700240A"; // STB 100-3
        break;
    case 71:
        id = "9600240A"; // STB 100-2
        break;
    case 70:
        id = "9700240A"; // STB 100-1
        break;
    case 62:
        id = "86002A0A"; // SQR 100-3
    case 61:
        id = "85002A0A"; // SQR 100-2
        break;
    case 60:
        id = "84002A0A"; // SQR 100-1
        break;
    case 54:
        id = "8700270A"; // SQN 100-5
        break;
    case 53:
        id = "8C00270A"; // SQN 100-4
        break;
    case 52:
        id = "8600270A"; // SQN 100-3
        break;
    case 51:
        id = "8500270A"; // SQN 100-2
        break;
    case 50:
        id = "8400270A"; // SQN 100-1
        break;
    case 43:
        id = "87002C0A"; // SQW Variant D
        break;
    case 42:
        id = "86002C0A"; // SQW Variant C
        break;
    case 41:
        id = "85002C0A"; // SQW Variant B
        break;
    case 40:
        id = "84002C0A"; // SQW Variant A
        break;
    case 30:
        id = "04002E07"; // STJ 100-1
        break;
    case 21:
        id = "A600240A"; // STK 100-2
        break;
    case 20:
        id = "A500240A"; // STK 100-1
        break;
    case 13:
        id = "8400240A"; // STL 100-4
        break;
    case 12:
        id = "8500240A"; // STL 100-3
        break;
    case 11:
        id = "8700240A"; // STL 100-2
        break;
    case 10:
        id = "4002607"; // STL 100-1 / DAB
        break;
	case 5:
		id = "B500240a"; // STA 100-6
		break;
    case 4:
        id = "9500240A"; // STA 100-5
        break;
    case 3:
        id = "8F00240A"; // STA 100-4
        break;
    case 2:
        id = "8E00240A"; // STA 100-3
        break;
    case 1:
        id = "8D00240A"; // STA 100-2
        break;
    case 0:
    default:
        id = "8C00240A"; // STA 100-1
        break;
    }
    return id;
}

void MainNet::downloadPotentialLink(QString softwareRelease, QString osVersion) {
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(softwareRelease.toLocal8Bit());
    QDesktopServices::openUrl("http://cdn.fs.sl.blackberry.com/fs/qnx/production/"
                              + QString(hash.result().toHex())
                              + "/com.qnx.coreos.qcfm.os.qc8960.factory_sfi.desktop/"
                              + osVersion + "/qc8960.factory_sfi.desktop-" + osVersion + "-nto+armle-v7+signed.bar");
}

void MainNet::reverseLookup(QString carrier, QString country, int device, int variant, int server, QString OSver)
{
    _softwareRelease = "Asking server..."; emit softwareReleaseChanged();
    setScanning(true);
    QString id = HWIDFromVariant(device * 10 + variant);
    QString homeNPC = NPCFromLocale(carrier.toInt(), country.toInt());
    QString requestUrl;

    switch (server)
    {
    case 1:
        requestUrl = "https://cse.beta.sl.eval.blackberry.com/cse/srVersionLookup/";
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
                        "<pin>0x2FFFFFB3</pin><bsn>1140011878</bsn><imei>004402242176786</imei><id>0x"+id+"</id><isBootROMSecure>true</isBootROMSecure>"
                    "</hardware>"
                    "<network>"
                        "<vendorId>0x0</vendorId><homeNPC>0x"+homeNPC+"</homeNPC><currentNPC>0x"+homeNPC+"</currentNPC><ecid>0x1</ecid>"
                    "</network>"
                    "<software><currentLocale>en_US</currentLocale><legalLocale>en_US</legalLocale>"
					"<osVersion>"+OSver+"</osVersion><omadmEnabled>false</omadmEnabled></software></clientProperties>"
                    "</srVersionLookupRequest>";
    QNetworkRequest request;
    request.setRawHeader("Content-Type", "text/xml;charset=UTF-8");
    request.setUrl(QUrl(requestUrl));
    reply = manager->post(request, query.toUtf8());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(serverError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished()), this, SLOT(reverseLookupReply()));
}

void MainNet::reverseLookupReply() {
    QByteArray data = reply->readAll();
    //for (int i = 0; i < data.size(); i += 3000) qDebug() << data.mid(i, 3000);
    QXmlStreamReader xml(data);
    while(!xml.atEnd() && !xml.hasError()) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "softwareReleaseVersion")
                _softwareRelease = xml.readElementText(); emit softwareReleaseChanged();
        }
        xml.readNext();
    }
    setScanning(false);
}

void MainNet::updateDetailRequest(QString delta, QString carrier, QString country, int device, int variant, int mode, int server/*, int version*/)
{
    setScanning(true);
    QString up, requestUrl;

    switch (server)
    {
    case 1:
        requestUrl = "https://cse.beta.sl.eval.blackberry.com/cse/updateDetails/";
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
	requestUrl += "2.2.0/";

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

    QString id = HWIDFromVariant(device * 10 + variant);

    QNetworkRequest request;
    request.setRawHeader("Content-Type", "text/xml;charset=UTF-8");
    request.setUrl(QUrl(requestUrl));

    QString query;
    /*if (version == 2) // 1.0.0 (legacy)
    {
        query = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<updateDetailRequest version=\"1.0.0\" authEchoTS=\"1342754372525\">"
                "<deviceId><pin>0x25D81234</pin><serialNumber>1128121361</serialNumber></deviceId>"
                "<clientProperties><hardware><id>0x"+id+"</id><isBootROMSecure>true</isBootROMSecure></hardware>"
                "<network><vendorId>0x0</vendorId><homeNPC>822349872</homeNPC></network>"
                "<software><currentLocale>en_US</currentLocale><legalLocale>en_US</legalLocale><osVersion>10.0.0.0</osVersion><radioVersion>10.0.0.0</radioVersion></software></clientProperties>"
                "<currentTransport>WIFI</currentTransport><updateDirectives><allowPatching type=\"REDBEND\">true</allowPatching><upgradeMode>"+up+"</upgradeMode><provideBundleDescriptions>true</provideBundleDescriptions><directive type=\"allowOSDowngrades\">true</directive></updateDirectives>"
                "</updateDetailRequest>";
    } else if (version == 1) { // 2.0.0 (new)
        query = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<updateDetailRequest version=\"2.0.0\" authEchoTS=\"1361763056140\">"
                    "<clientProperties>"
                        "<hardware>"
                            "<pin>0x25D81234</pin><bsn>1128121361</bsn><imei>004401139269240</imei><id>0x"+id+"</id><isBootROMSecure>true</isBootROMSecure>"
                        "</hardware>"
                        "<network>"
                            "<vendorId>0x0</vendorId><homeNPC>0x"+homeNPC+"</homeNPC><iccid>89014104255505565333</iccid><msisdn>15612133940</msisdn><imsi>310410550556533</imsi><ecid>0x0</ecid>"
                        "</network>"
                        "<software>"
                            "<currentLocale>en_US</currentLocale><legalLocale>en_US</legalLocale><osVersion>10.0.0.0</osVersion><radioVersion>10.0.0.0</radioVersion>"
                        "</software>"
                    "</clientProperties>"
                    "<updateDirectives><allowPatching type=\"REDBEND\">true</allowPatching><upgradeMode>"+up+"</upgradeMode><provideDescriptions>true</provideDescriptions><provideFiles>true</provideFiles><queryType>NOTIFICATION_CHECK</queryType></updateDirectives>"
                    "<resultPackageSetCriteria>"
                        "<softwareRelease softwareReleaseVersion=\"latest\" />"
                    "</resultPackageSetCriteria>"
                "</updateDetailRequest>";
    } else*/ { // 2.1.0 (newest)
        query = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
				"<updateDetailRequest version=\"2.2.0\" authEchoTS=\"1361763056140\">"
                    "<clientProperties>"
                        "<hardware>"
                            "<pin>0x2FFFFFB3</pin><bsn>1128121361</bsn><imei>004401139269240</imei><id>0x"+id+"</id><isBootROMSecure>true</isBootROMSecure>"
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
    }
    _error = ""; emit errorChanged();
    reply = manager->post(request, query.toUtf8());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(serverError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished()), this, SLOT(serverReply()));
}

void MainNet::serverReply()
{
    setScanning(false);
    QByteArray data = reply->readAll();
    //for (int i = 0; i < data.size(); i += 3000) qDebug() << data.mid(i, 3000);

    QXmlStreamReader xml(data);
    QString ver = "";
    QString desc = "";
    QString addr = "";
    QString apps = "";
    QString links = "";
    QString currentaddr = "";
    QList<int> sizes;
    while(!xml.atEnd() && !xml.hasError()) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "package")
            {
                QString name = xml.attributes().value("name").toString();
                int downloadSize = xml.attributes().value("downloadSize").toString().toInt();
                apps += name + "  " + QString::number(downloadSize/1048576) + "MB<br>";
                sizes.append(downloadSize);
                QString app_path = xml.attributes().value("path").toString();
                QString type = xml.attributes().value("type").toString();
                if (type == "system:os" || type == "system:desktop")
                    _versionOS = app_path.split('/').at(1);
                if (type == "system:radio")
                    _versionRadio = app_path.split('/').at(1);
                links += currentaddr + "/" + app_path + "\n";
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
    _versionRelease = ver; emit versionChanged();
    _description = desc; emit descriptionChanged();
    _url = addr; emit urlChanged();
    _applications = apps; emit applicationsChanged();
    _sizes = sizes;
    _dlTotal = 0;
    foreach(int i, _sizes) { _dlTotal += i; }
    _links = links;
    if (_dlTotal > 0)
    {
        _error = ""; emit errorChanged();
    }
}

void MainNet::serverError(QNetworkReply::NetworkError err)
{
    setScanning(false);
    QString errormsg;
    errormsg.setNum(err);
    errormsg.prepend("Error: ");
    _error = QString::number(err); emit errorChanged();
    _versionRelease = ""; _versionOS = ""; _versionRadio = ""; emit versionChanged();

}

void MainNet::setDLProgress(const int &progress) { _dlProgress = progress; emit dlProgressChanged(); }
void MainNet::setAdvanced(const bool &advanced) { QSettings settings("Qtness","Sachesi"); settings.setValue("advanced", advanced); _advanced = advanced; emit advancedChanged(); }
void MainNet::setScanning(const bool &scanning) { _scanning = scanning; emit scanningChanged(); }
void MainNet::setDownloading(const bool &downloading) { _downloading = downloading; emit downloadingChanged(); }
void MainNet::setSplitProgress(const int &progress) { if (_splitProgress > 1000) _splitProgress = 0; else _splitProgress = progress; emit splitProgressChanged(); }

QString MainNet::softwareRelease() const { return _softwareRelease; }
QString MainNet::versionRelease() const { return _versionRelease; }
QString MainNet::versionOS()      const { return _versionOS; }
QString MainNet::versionRadio()   const { return _versionRadio; }
QString MainNet::description()    const { return _description; }
QString MainNet::url()            const { return _url; }
QString MainNet::applications()   const { return _applications; }
QString MainNet::error()          const { return _error; }
bool    MainNet::advanced()       const { return _advanced; }
bool    MainNet::scanning()       const { return _scanning; }
bool    MainNet::downloading()    const { return _downloading; }
int     MainNet::dlProgress()     const { return _dlProgress; }
int     MainNet::currentId()      const { return _currentId; }
int     MainNet::maxId()          const { return _maxId; }
int     MainNet::splitting()      const { return _splitting; }
int     MainNet::splitProgress()  const { return _splitProgress; }
QString MainNet::currentFile()    const { QString ret = _currentFile.fileName().split("/").last(); if (ret.length() > 30) ret.truncate(30); return ret; }
