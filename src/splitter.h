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

#include <QObject>
#include <QBuffer>
#include <QStringList>
#include <QDir>
#include <QDataStream>
#include <QDebug>
#include <QCoreApplication>
#ifndef BLACKBERRY
#include <QMessageBox>
#endif
#include "ports.h"
#include "fs/qnx6.h"
#include "fs/rcfs.h"
#include "fs/ifs.h"
#include "autoloaderwriter.h"

enum QFileSystemType {
    FS_UNKNOWN = 0,
    FS_RCFS = 1,
    FS_QNX6 = 2,
    FS_IFS = 4,
};

struct ProgressInfo {
    int unique;
    qint64 curSize;
    qint64 maxSize;
    double progress;
};

struct PartitionInfo {
    qint64 offset;
    qint64 size;
    QFileSystemType type;
    QIODevice* dev;
    QString image;
    // We are grabbing a partition of a container or image
    PartitionInfo(QIODevice* file, qint64 loc, qint64 size = 0)
        : offset(loc)
        , size(size)
        , dev(file)
    {
        detectType(dev);
    }

    void detectType(QIODevice* device) {
        // Check what sort of image we are dealing with
        if (!device->isSequential())
            device->seek(offset);
        QByteArray header = device->read(4);

        if (header == QByteArray("rimh", 4))
            type = FS_RCFS;
        else if (header == QByteArray::fromHex("EB109000"))
            type = FS_QNX6;
        else if (header == QByteArray::fromHex("FE0300EA")  // Qualcomm
              || header == QByteArray::fromHex("FE0100EA")) // OMAP
            type = FS_IFS;
        else
            type = FS_UNKNOWN;
    }
};

#define PACKED_FILE_USER    (1 << 0)
#define PACKED_FILE_OS      (1 << 1)
#define PACKED_FILE_RADIO   (1 << 2)
#define PACKED_FILE_IFS     (1 << 3)
#define PACKED_FILE_PINLIST (1 << 4)

class Splitter: public QObject {
    Q_OBJECT

public:
    Splitter(QString file) : selectedFile(file) { reset(); }
    Splitter(QString file, int options) : option(options), selectedFile(file)  { reset(); }
    Splitter(QStringList files) : selectedFiles(files)  { reset(); }
    Splitter(QList<QFileInfo> info) : selectedInfo(info)  { reset(); }
    ~Splitter() { }
    bool extractApps, extractImage;
    int extractTypes;
public slots:
    void reset() {
        kill = false;
        read = 0; maxSize = 1; emit progressChanged(0);
        extractApps = false;
        extractImage = false;
        extractTypes = 0;
        extracting = false;
        splitting = false;
    }

    void killSplit() {
        kill = true;
        die();
    }
    void cleanDevHandle() {
        foreach (QIODevice* dev, devHandle) {
            // QIODevice's are automatically closed.
            if (dev != nullptr) {
                dev->deleteLater();
                dev = nullptr;
            }
        }
        devHandle.clear();
    }

    void die(QString message = "") {
        qDebug() << "Tool terminated early due to unforseen circumstances.";
        if (extracting) {
            cleanDevHandle();
        }
        if (splitting)
        {
            for (int i = 0; i < tmpFile.count(); i++)
            {
                if (tmpFile.at(i)->isOpen())
                {
                    tmpFile.at(i)->close();
                    delete tmpFile.at(i);
                    tmpFile.removeAt(i);
                }
            }
            tmpFile.clear();
        }
        extracting = false;
        combining = false;
        splitting = false;
#ifndef BLACKBERRY
        if (!message.isEmpty())
            QMessageBox::information(nullptr, "Error", message);
#endif
        emit finished();
    }

    void processSplitBar() {
        // And it seems we need a wrapper for splitting too.
        // Splitting is just extracting .signed, so lets integrate those after
        splitting = true;
        processExtractBar();
        cleanDevHandle();

        emit finished();
    }

    void processExtractBar();

    void processSplitAutoloader() {
        // Ditto with the above split bar
        splitting = true;
        processExtractAutoloader();
        emit finished();
    }

    void processExtractAutoloader();

    void processCombine() {
        if (selectedInfo.count() > 6) {
            QMessageBox::information(nullptr, "Error", "Autoloaders can only have a maximum of 6 signed files.");
            return;
        }
        combining = true;
        QFileInfo largestInfo;
        qint64 largestSize = 0;
        foreach (QFileInfo info, selectedInfo) {
            if (info.size() > largestSize) {
                largestSize = info.size();
                largestInfo = info;
            }
        }
        // Create new Autoloader object
        AutoloaderWriter newAutoloader(selectedInfo);
        connect(&newAutoloader, &AutoloaderWriter::newProgress, [=](int percent) { emit this->progressChanged(percent); });
        newAutoloader.create(largestInfo.absolutePath() + "/" + largestInfo.completeBaseName());
        emit finished();
    }

    void processExtractSigned();
    void processExtract(QIODevice* dev, qint64 signedSize, qint64 signedPos);
    void processExtractType();
    QFileSystem* createTypedFileSystem(QString name, QIODevice* dev, QFileSystemType type, qint64 offset = 0, qint64 size = 0, QString baseDir = ".");

    void processExtractWrapper();

    // Old, compatibility
    quint64 updateProgress(qint64 delta) {
        if (delta < 0)
            return 0;
        read += 100 * delta;
        emit progressChanged((int)(read / maxSize));
        return delta;
    }
    int newProgressInfo(qint64 size) {
        progressInfo.append(ProgressInfo());
        progressInfo.last().maxSize = size;
        return progressInfo.count() - 1;
    }

    void updateCurProgress(int unique, qint64 bytes, qint64 delta) {
        // New, unused
        if (progressInfo.count() <= unique)
            return;
        progressInfo[unique].curSize = bytes;
        progressInfo[unique].progress = (double)(100*progressInfo[unique].curSize) / (double)progressInfo[unique].maxSize;

        // Old, compatibility
        read += 100 * delta;
        emit progressChanged((int)(read / maxSize));
    }

signals:
    void finished();
    void progressChanged(int progress);
    void error(QString err);
private:
    bool kill;
    quint64 read, maxSize;
    bool splitting;
    bool combining;
    bool extracting;
    int option;
    QString selectedFile;
    QStringList selectedFiles;
    QList<QFileInfo> selectedInfo;
    QList<QFile*> tmpFile;

    // New
    QList<ProgressInfo> progressInfo;
    QList<PartitionInfo> partitionInfo;
    QList<QIODevice*> devHandle;
};
