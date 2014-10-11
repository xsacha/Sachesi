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
#include "installer.h"
#include "backupinfo.h"
#endif
#include "carrierinfo.h"
#include "appworld.h"
#ifdef BOOTLOADER_ACCESS
#include "boot.h"
#endif

// TODO: Make extraction handle decent % tracking for QNX FS
// Need help: Check and improve the USB Loader (Boot).
// TODO: Use CircleProgress in every progress (Extract) section. Pass a class to QML that contains file count, current and total progress
// TODO: Window {} not working on Android. Maybe special QML files required?
// Need testing: Check PolicyRestrictions somehow?
// Personal: policy_block_backup_and_restore, policy_backup_and_restore
// Enterprise: policy_disable_devmode, policy_log_submission, policy_block_computer_access_to_device

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);

    QApplication app(argc, argv);
    app.setOrganizationName("Qtness");
    app.setOrganizationDomain("qtness.com");
    app.setApplicationName("Sachesi");
    app.setApplicationVersion("1.9.4");

    // Install translator by locale language string
    QTranslator appTranslator;
    if (appTranslator.load(QString(":/translations/%1.qm")
                                  .arg(QLocale::languageToString(QLocale().language())))
            ) {
        app.installTranslator(&appTranslator);
    }
    // Use system proxy except where not possible
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();

    // Do we have a suitable place to store files that the user will be able to find?
    if (!checkCurPath()) {
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Could not find a suitable storage path.\nPlease report this."));
        return 0;
    }

    // *** Static QML Variables that describe the environment
    // Useful as QML is unable to detect this natively

    // Send the version across. Preferably via the .pro
    context->setContextProperty("version", QVariant::fromValue(QApplication::applicationVersion()));
    // Check if we have at least Qt 5.3 available. If not, do some workarounds for bugs.
    context->setContextProperty("qt_new", QVariant::fromValue(QT_VERSION > QT_VERSION_CHECK(5, 3, 0)));
    // Check if this is a Blackberry device. There are some restrictions such as no InstallNet
    context->setContextProperty("blackberry", QVariant::fromValue(
                                #ifdef BLACKBERRY
                                    1
                                #else
                                    0
                                #endif
                                    ));
    // Check if this is a mobile device as they often do not have enough space.
    context->setContextProperty("mobile", QVariant::fromValue(
                                #if defined(BLACKBERRY) || defined(ANDROID)
                                    1
                                #else
                                    0
                                #endif
                                    ));

    // *** C++ Classes that are passed to the QML pages.
    // Heavy lifting to be done by the compiled and feature-packed language.
#ifdef BLACKBERRY
    MainNet p;
#else
    InstallNet i;
    context->setContextProperty("i", &i);
    MainNet p(&i);
#endif
#ifdef BOOTLOADER_ACCESS
    Boot b;

    QObject::connect(&b, SIGNAL(started()), &b, SLOT(search()));
    QObject::connect(&b, SIGNAL(finished()), &b, SLOT(exit()));
    QObject::connect(&i, SIGNAL(newPassword(QString)), &b, SLOT(newPassword(QString)));
    b.start();
    context->setContextProperty("b", &b); // Boot
#endif
    AppWorld world;
    CarrierInfo info;

    // Set contexts for the classes
    context->setContextProperty("p", &p); // MainNet
    context->setContextProperty("download", p.currentDownload);
    context->setContextProperty("carrierinfo",  &info);
    context->setContextProperty("appworld",  &world);

    // *** Register types for the QML language to understand types used by C++, when passed
#ifndef BLACKBERRY
    qmlRegisterType<BackupInfo>("BackupTools", 1, 0, "BackupInfo");
#endif
    qmlRegisterType<Apps>("AppLibrary", 1, 0, "Apps");
    qmlRegisterType<AppWorldApps>("AppWorldLibrary", 1, 0, "AppWorldApps");

#if defined(_WIN32) && defined(STATIC)
    engine.addImportPath("qrc:/qml/");
#endif

    // *** Now let's try to show the QML file and check for errors
    QScopedPointer<QQmlComponent> comp(new QQmlComponent(&engine));
    comp->loadUrl(QUrl("qrc:/qml/generic/Title.qml"));
    if (comp->status() == QQmlComponent::Error) {
        QMessageBox::information(nullptr, "Error", qPrintable(comp->errorString()), QMessageBox::Ok);
        return 0;
    }
    QQuickWindow *window = qobject_cast<QQuickWindow *>(comp->create());
    window->show();

    int ret = app.exec();

#ifdef BOOTLOADER_ACCESS
    b.quit();
    b.wait(1000);
#endif
    delete window;
    return ret;
}
