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
#include <QtQml>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QQmlApplicationEngine>

#include "mainnet.h"
#ifndef BLACKBERRY
#include "install.h"
#include "backupinfo.h"
#include "apps.h"
#include "ports.h"
#endif
#ifdef BOOTLOADER_ACCESS
#include "boot.h"
#endif

// TODO: Make extraction less hacky.
// TODO: Make extraction handle decent % tracking
// TODO: Check and improve the USB Loader (Boot).

Q_DECL_EXPORT int main(int argc, char *argv[])
{
#ifdef Q_WS_X11
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);
#endif
    QApplication app(argc, argv);
    app.setOrganizationName("Qtness");
    app.setOrganizationDomain("qtness.com");
    app.setApplicationName("Sachesi");

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();
    MainNet p;

#ifndef BLACKBERRY
    InstallNet i;
    context->setContextProperty("i", &i);
#endif

    if (!checkCurPath())
        return 0;

#ifdef BOOTLOADER_ACCESS
    Boot b;
    context->setContextProperty("b", &b);

    QObject::connect(&b, SIGNAL(started()), &b, SLOT(search()));
    QObject::connect(&b, SIGNAL(finished()), &b, SLOT(exit()));
    QObject::connect(&i, SIGNAL(newPassword(QString)), &b, SLOT(newPassword(QString)));
    b.start();
#endif

    context->setContextProperty("p", &p);
#ifndef BLACKBERRY
    qmlRegisterType<BackupInfo>("BackupTools", 1, 0, "BackupInfo");
    qmlRegisterType<Apps>("AppLibrary", 1, 0, "Apps");
#endif

    engine.addImportPath("qrc:/qml/");
    QQmlComponent* comp = new QQmlComponent(&engine);
    comp->loadUrl(QUrl("qrc:/qml/generic/Title.qml"));
    if (comp->status() == QQmlComponent::Error) {
        QMessageBox::information(NULL, "Error", qPrintable(comp->errorString()), QMessageBox::Ok);
    } else {
        QQuickWindow *window = qobject_cast<QQuickWindow *>(comp->create());
        window->show();
    }

    int ret = app.exec();
#ifdef BOOTLOADER_ACCESS
    b.quit();
    b.wait(1000);
#endif
    return ret;
}
