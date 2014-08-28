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

#include <QGuiApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QQmlApplicationEngine>

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

bool checkCurPath()
{
#ifdef BLACKBERRY
    QDir dir;
    dir.mkpath("/accounts/1000/shared/misc/Sachesi");
    QDir::setCurrent("/accounts/1000/shared/misc/Sachesi");
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

Q_DECL_EXPORT int main(int argc, char *argv[])
{
#ifdef Q_WS_X11
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);
#endif

    QGuiApplication app(argc, argv);
    app.setOrganizationName("Qtness");
    app.setOrganizationDomain("qtness.com");
    app.setApplicationName("Sachesi");

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();
    MainNet p;

#ifndef BLACKBERRY
    InstallNet i;
    context->setContextProperty("i",&i);
#endif

    if (!checkCurPath())
        return 0;

#ifdef BOOTLOADER_ACCESS
    Boot b;
    context->setContextProperty("b",&b);

    QObject::connect(&b, SIGNAL(started()), &b, SLOT(search()));
    QObject::connect(&b, SIGNAL(finished()), &b, SLOT(exit()));
    QObject::connect(&i, SIGNAL(newPassword(QString)), &b, SLOT(newPassword(QString)));
    b.start();
#endif

    context->setContextProperty("p",&p);
#ifndef BLACKBERRY
    qmlRegisterType<DropArea>("Drop", 1, 0, "DropArea");
    qmlRegisterType<BackupInfo>("BackupTools", 1, 0, "BackupInfo");
    qmlRegisterType<Apps>("AppLibrary", 1, 0, "Apps");
#endif

    engine.load(QUrl("qrc:/qml/generic/Title.qml"));

    QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    window->setMinimumHeight(440);
    window->setMinimumWidth(520);
#if 0
#ifdef SACHESI_GIT_VERSION
    window->setWindowTitle(QString("Sachesi ") + SACHESI_VERSION + "-" + SACHESI_GIT_VERSION);
#else
    window->setWindowTitle(QString("Sachesi ") + SACHESI_VERSION);
#endif
#endif

    window->show();

    int ret = app.exec();
#ifdef BOOTLOADER_ACCESS
    b.quit();
    b.wait(1000);
#endif
    return ret;
}
