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

#include "ports.h"
// For portability between platforms and Qt versions.
// Clears up the code in the more important files.

#ifndef BLACKBERRY
#include <QApplication>
#include <QMessageBox>
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QStandardPaths>
#endif
#include <QDir>

QPair<QString,QString> getFamilyFromDevice(int device, bool specialQ30) {
    QPair<QString,QString> ret = {"", ""};
    switch(device) {
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
        ret = {"qc8960.factory_sfi_hybrid_qc8x30", "qc8930.wtr5"};
        break;
    case Q30Family:
        if (specialQ30)
            ret = {"qc8974.factory_sfi", "qc8974.wtr2"};
        else
            ret = {"qc8960.factory_sfi_hybrid_qc8974", "qc8974.wtr2"};
        break;
    case Q10Family:
        ret = {"qc8960.factory_sfi", "qc8960.wtr"};
        break;
    }
    return ret;
}

QString capPath(bool temp) {
#ifndef BLACKBERRY
    QSettings ini(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
    QString capPath = QFileInfo(ini.fileName()).absolutePath();
    if (temp) {
        // On Windows this folder may not exist even if the .ini is meant to be there! So make it when writing temp file.
        QDir(capPath).mkdir(".");
        return capPath + "/.cap.exe";
    }
    return capPath + "/cap.exe";
#else
    return "cap.exe";
#endif
}

bool isVersionNewer(QString first, QString second, bool orSame) {
    QStringList firstVer = first.split('.');
    QStringList secondVer = second.split('.');
    if (firstVer.count() < 4)
        return false;
    for (int i = 0; i < 4; i++) {
        int newBuild = firstVer.at(i).toInt();
        int oldBuild = secondVer.at(i).toInt();
        if (newBuild > oldBuild)
            return true;
        if (newBuild < oldBuild)
            return false;
    }
    return orSame;
}

#ifndef BLACKBERRY
QFileDialog* selectFiles(QString title, QString dir, QString nameString, QString nameExt) {
    QFileDialog* finder = new QFileDialog();
    finder->setWindowTitle(title);
    finder->setDirectory(dir);
    finder->setNameFilter(nameString + "(" + nameExt + ")");
    QListView *l = finder->findChild<QListView*>("listView");
    if (l)
        l->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QTreeView *t = finder->findChild<QTreeView*>();
    if (t)
        t->setSelectionMode(QAbstractItemView::ExtendedSelection);
    return finder;
}
#endif

QString getSaveDir() {
#ifdef BLACKBERRY
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    return "/accounts/1000/shared/misc/Sachesi/";
#else
    return QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + "/Sachesi/";
#endif
#else
    QString writable = QDir::currentPath() + "/";

    if (QFileInfo(writable).isWritable())
        return writable;

    return QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
#endif
}

bool checkCurPath()
{
    QDir dir;
#ifdef BLACKBERRY
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QString path = "/accounts/1000/shared/misc/Sachesi/";
#else
    QString path = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + "/Sachesi/";
#endif
    dir.mkpath(path);
    QDir::setCurrent(path);
#elif defined(ANDROID)
    // Iterate through removable to local
    QString sdcard = "/sdcard1/Sachesi/";
    dir.mkpath(sdcard);
    if (!dir.exists(sdcard)) {
        sdcard = "/sdcard/Sachesi";
        dir.mkpath(sdcard);
    }

    QDir::setCurrent(sdcard);
#else
    QDir::setCurrent(QApplication::applicationDirPath());
#if defined(__APPLE__)
    dir = QDir(QDir::currentPath());
    // Use .app path instead of binary path. Should really use a different method.
    if (dir.absolutePath().endsWith("Contents/MacOS")) {
        while (!dir.absolutePath().endsWith(".app"))
            dir.cdUp();
        dir.cdUp();
    }
    QDir::setCurrent(dir.absolutePath());
#endif
#endif

    return true;
}

void openFile(QString name) {
    QDesktopServices::openUrl(QUrl::fromLocalFile(name));
}

void writeDisplayFile(QString type, QString writeText) {
    QDir(getSaveDir()).mkpath(".");
    QFile displayFile(getSaveDir() + "/" + type + ".txt");

    if (!displayFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    displayFile.write(writeText.toUtf8());
    openFile(displayFile.fileName());
    displayFile.close();
}

