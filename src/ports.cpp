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

FileSelect selectFiles(QString title, QString dir, QString nameString, QString nameExt) {
#ifdef BLACKBERRY
    FileSelect finder = new bb::cascades::pickers::FilePicker();
    finder->setTitle(title);
    finder->setDirectories(QStringList() << dir);
    finder->setFilter(nameExt.split(' '));
    finder->setMode(bb::cascades::pickers::FilePickerMode::Picker);
    finder->open();
#else
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
#endif
    return finder;
}

QString getSaveDir() {
    QSettings settings("Qtness", "Sachesi");
#ifdef BLACKBERRY
    QString writable = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Sachesi/";
#else
    QString writable = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
#endif
    QString path = settings.value("splitDir", writable).toString();
    if (!(QFileInfo(path).isDir() && QFileInfo(path).isWritable()))
    {
        settings.setValue("splitDir", writable);
        return writable;
    }

    return path;
}

bool checkCurPath()
{
#ifdef BLACKBERRY
    QDir dir;
    QString path = QStandardPaths::standardLocations(GenericDataLocation) + "/Sachesi/";
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

