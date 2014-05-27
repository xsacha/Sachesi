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

#include <QApplication>
#include <QDeclarativeContext>
#include <QtDeclarative/qdeclarative.h>
#include "../qmlapplicationviewer/qmlapplicationviewer.h"
#include "mainnet.h"
#ifndef BLACKBERRY
#include "install.h"
#include "droparea.h"
#include "backupinfo.h"
#include "apps.h"
#endif
#ifdef BOOTLOADER_ACCESS
#include "boot.h"
#endif

// TODO: Make extraction less hacky.
// TODO: Make extraction handle decent % tracking
// TODO: Check and improve the USB Loader (Boot).
// TODO: Handle threading issues, if any, in Boot.

Q_DECL_EXPORT int main(int argc, char *argv[])
{
#ifdef Q_WS_X11
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);
#endif
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    QmlApplicationViewer viewer;
    MainNet p;
#ifndef BLACKBERRY
    InstallNet i;
#else
    QDir dir;
    dir.mkpath("/accounts/1000/shared/misc/Sachesi");
    QDir::setCurrent("/accounts/1000/shared/misc/Sachesi");
#endif
    QString curPath = QDir::currentPath();
    if (curPath.endsWith(".tmp") || curPath.endsWith(".zip") || curPath.endsWith("/system32")) {
        QMessageBox::critical(NULL, "Error", "Cannot be run from within a zip.\n Please extract first.");
        return 0;
    }
    // Workaround for Mavericks
#if defined(__APPLE__)
    QDir::setCurrent(QApplication::applicationDirPath()+"/../../../");
#endif
#ifdef BOOTLOADER_ACCESS
    Boot b;
    QThread thread;
    thread.connect(&thread, SIGNAL(started()), &b, SLOT(search()));
    thread.connect(&thread, SIGNAL(finished()), &b, SLOT(exit()));
    i.connect(&i, SIGNAL(newPassword(QString)), &b, SLOT(newPassword(QString)));
    b.moveToThread(&thread);
    thread.start();
#endif
    viewer.rootContext()->setContextProperty("p",&p);
#ifndef BLACKBERRY
    qmlRegisterType<DropArea>("Drop", 1, 0, "DropArea");
    qmlRegisterType<BackupInfo>("BackupTools", 1, 0, "BackupInfo");
    qmlRegisterType<Apps>("AppLibrary", 1, 0, "Apps");
    viewer.rootContext()->setContextProperty("i",&i);
#endif
#ifdef BOOTLOADER_ACCESS
    viewer.rootContext()->setContextProperty("b",&b);
#endif
    viewer.setSource(QUrl("qrc:/qml/generic/Title.qml"));
    viewer.setMinimumHeight(440);
    viewer.setMinimumWidth(520);
    viewer.setWindowTitle("Sachesi 1.2.1");

    QSettings settings("Qtness", "Sachesi");
    viewer.restoreGeometry(settings.value("geometry").toByteArray());
    viewer.show();

    int ret = app->exec();
#if !defined(_WIN32) && !defined(BLACKBERRY) && !defined(ANDROID)
    thread.quit();
    thread.wait(4000);
#endif
    return ret;
}
