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

#include <QApplication>
#include <QStandardPaths>
#include <QMessageBox>

QString capPath() {
    QSettings ini(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
    QString capPath = QFileInfo(ini.fileName()).absolutePath();
    return capPath + "/cap.exe";
}

FileSelect selectFiles(QString title, QString dir, QString nameString, QString nameExt) {
    FileSelect finder = new QFileDialog();
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

QString getSaveDir() {
#ifdef BLACKBERRY
    QString writable = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Sachesi/";
#else
    QString writable = QDir::currentPath() + "/";
#endif
    if (QFileInfo(writable).isWritable())
        return writable;

    return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
}

bool checkCurPath()
{
#ifdef BLACKBERRY
    QDir dir;
    QString path = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + "/Sachesi/";
    dir.mkpath(path);
    QDir::setCurrent(path);
#endif

    QString curPath = QDir::currentPath();
    // Use .app path instead of binary path. Should really use a different method.
#ifdef __APPLE__
    if (curPath.endsWith("Contents/MacOS"))
        QDir::setCurrent(QApplication::applicationDirPath()+"/../../../");
#endif
    if (curPath.endsWith(".tmp") || curPath.endsWith(".zip") || curPath.endsWith("/system32")) {
        QMessageBox::critical(nullptr, "Error", "Cannot be run from within a zip.\n Please extract first.");
        return false;
    }

    return true;
}

void openFile(QString name) {
    // This could get more complicated
    QDesktopServices::openUrl(QUrl::fromLocalFile(name));
}

void writeDisplayFile(QString name, QByteArray data) {
    QFile displayFile(getSaveDir() + "/" + name);

    if (!displayFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    displayFile.write(data);
    openFile(displayFile.fileName());
    displayFile.close();

#ifdef BLACKBERRY
    // Clipboard has dependency on Qt4
    //bb::system::Clipboard clipboard;
    //clipboard.clear();
    //clipboard.insert("text/plain", data);

    // Cascades code is not working
/*#if defined(BLACKBERRY)
    QVariantMap data;
    data["title"] = "Links";
    bb::cascades::Invocation* invocation = bb::cascades::Invocation::create(
                bb::cascades::InvokeQuery::create().invokeActionId("bb.action.SHARE").metadata(data).uri("file:///accounts/1000/shared/misc/updates.txt").invokeTargetId(
                                "sys.pim.remember.composer"));
    connect(invocation, SIGNAL(finished()), invocation, SLOT(deleteLater()));
*/
    #endif
}

