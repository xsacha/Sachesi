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
// Also a make-shift utility file

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

// TODO: Maybe name families by radio
enum DeviceFamily {
    UnknownFamily = 0,
    Z30Family,
    OMAPFamily,
    Z10Family,
    Z3Family,
    Q30Family,
    Q10Family,
};

static QStringList dev[] = {
    // 0 = Z30 (A Series) + Classic
    QStringList() << "STA 100-1" << "STA 100-2" << "STA 100-3" << "STA 100-4" << "STA 100-5" << "STA 100-6" << "Classic AT/T" << "Classic Verizon" << "Classic ROW" << "Classic NA",
    QStringList() << "8C00240A" << "8D00240A" << "8E00240A" << "8F00240A" << "9500240A" << "B500240A" << "9400270A" << "9500270A" << "9600270A" << "9700270A",
    // 1 = Z10 (L Series) OMAP
    QStringList() << "STL 100-1",
    QStringList() << "4002607",
    // 2 = Z10 (L Series) Qualcomm + P9982  (TK Series)
    QStringList() << "STL 100-2" << "STL 100-3" << "STL 100-4" << "STK 100-1" << "STK 100-2",
    QStringList() << "8700240A" << "8500240A" << "8400240A" << "A500240A" << "A600240A",
    // 3 = Z3  (J Series) + Cafe
    QStringList() << "STJ 100-1" << "Cafe NA" << "Cafe Europe/ME/Asia" << "Cafe ROW" << "Cafe AT/T" << "Cafe LatinAm" << "Cafe Verizon",
    QStringList() << "04002E07" << "87002A07" << "8C002A07" << "9600240A" << "9700240A" << "9C00240A" << "A700240A",
    // 4 = Passport / Q30 (W Series)
    QStringList() << "SQW 100-1" << "SQW 100-2" << "SQW 100-3" << "SQW 100-4" << "Passport Wichita",
    QStringList() << "87002C0A" << "85002C0A" << "84002C0A" << "86002C0A" << "8C002C0A",
    // 5 = Q5 (R Series) + Q10 (N Series) + P9983 (QK Series)
    QStringList() << "SQR 100-1" << "SQR 100-2" << "SQR 100-3" << "SQN 100-1" << "SQN 100-2" << "SQN 100-3" << "SQN 100-4" << "SQN 100-5" << "SQK 100-1" << "SQK 100-2",
    QStringList() << "84002A0A" << "85002A0A" << "86002A0A" << "8400270A" << "8500270A" << "8600270A" << "8C00270A" << "8700270A"  << "8F00270A" << "8E00270A",
    // 6 = Dev Alpha
    QStringList() << "Alpha A" << "Alpha B" << "Alpha C",
    QStringList() << "4002307" << "4002607" << "8D00270A",
    // 7 = Ontario Series
    QStringList() << "Ontario NA" << "Ontario Verizon" << "Ontario Sprint" << "Ontario ROW" << "China",
    QStringList() << "AE00240A" << "AF00240A" << "B400240A" << "B600240A" << "BC00240A",
};

QPair<QString,QString> getFamilyFromDevice(int device, bool specialQ30);
QString capPath(bool temp = false);
#ifndef BLACKBERRY
QFileDialog* selectFiles(QString title, QString dir, QString nameString, QString nameExt);
#endif
QString getSaveDir();
bool checkCurPath();
void openFile(QString name);
void writeDisplayFile(QString type, QString writeText);
bool isVersionNewer(QString first, QString second, bool orSame);

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
