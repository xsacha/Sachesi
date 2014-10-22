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

#pragma once

// For portability between platforms and Qt versions.
// Clears up the code in the more important files.

#ifndef BLACKBERRY
#include <QFileDialog>
#include <QListView>
#include <QTreeView>
#include <QDesktopServices>
#endif
#include <QSettings>
#include <QUrl>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QUrl>
#else
#include <QUrlQuery>
#define encodedQuery query(QUrl::FullyEncoded).toUtf8
#endif

QString capPath(bool temp = false);
#ifndef BLACKBERRY
QFileDialog* selectFiles(QString title, QString dir, QString nameString, QString nameExt);
#endif
QString getSaveDir();
bool checkCurPath();
void openFile(QString name);
void writeDisplayFile(QString type, QString writeText);

// These may not be entirely necessary but there have been issues in the past
#define qSafeFree(x) \
    if (x != nullptr) { \
        x->deleteLater(); \
        x = nullptr; \
    }
#define qIoSafeFree(x) \
    if (x != nullptr) { \
        if (x->isOpen()) \
            x->close(); \
        x->deleteLater(); \
        x = nullptr; \
    }
#define ioSafeFree(x) \
    if (x != nullptr) { \
        if (x->isOpen()) \
            x->close(); \
        delete x; \
        x = nullptr; \
    }
