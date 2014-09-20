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
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDataStream>
#include <QDebug>
#include <QCoreApplication>
#include <QMessageBox>
#include "ports.h"
#include "fs/qnx6.h"
#include "fs/rcfs.h"
#include "fs/ifs.h"

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
        device->seek(offset);
        QByteArray header = device->read(4);

        if (header == QByteArray("rimh", 4))
            type = FS_RCFS;
        else if (header == QByteArray::fromHex("EB109000"))
            type = FS_QNX6;
        else if (header == QByteArray::fromHex("FE0300EA"))
            type = FS_IFS;
        else
            type = FS_UNKNOWN;
    }
};

#define PACKED_FILE_OS      (1 << 0)
#define PACKED_FILE_RADIO   (1 << 1)
#define PACKED_FILE_PINLIST (1 << 2)

class AutoloaderWriter: public QFile {
    Q_OBJECT
public:
    AutoloaderWriter(QList<QFileInfo> selectedInfo)
        : _infos(selectedInfo)
    {
    }
    void create(QString name) {
        // Find potential file
        QString append = ".exe";
        for (int f = 0; QFile::exists(name + append); f++) {
            append = QString("-%1.exe").arg(QString::number(f));
        }
        // Start the autoloader as a cap file
        QFile::copy(capPath(), name + append);
        setFileName(name + append);
        open(QIODevice::WriteOnly | QIODevice::Append);
        // This code is used as a separator
        write(QByteArray::fromBase64("at9dFE5LT0dJSE5JTk1TDRAMBRceERhTLUY8T0crSzk5OVNOT1FNT09RTU9RSEhwnNXFl5zVxZec1cWX").constData(), 60);
        // This is a placeholder for a password
        write(QByteArray(80, 0), 80);

        QByteArray dataHeader;
        QNXStream dataStream(&dataHeader, QIODevice::WriteOnly);
        dataStream << (quint64)_infos.count();
        quint64 counter = pos() + 64;
        foreach (QFileInfo info, _infos)
        {
            dataStream << counter;
            counter += info.size();
        }
        for (int i = _infos.count(); i < 6; i++)
            dataStream << (qint64)0;
        write(dataHeader);
        _read = 100 * pos();
        _maxSize = counter;
        foreach (QFileInfo file, _infos)
            appendFile(file.filePath());
        close();
    }

    void appendFile(QString fileName) {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        while (!file.atEnd())
        {
            QByteArray tmp = file.read(BUFFER_LEN);
            if (tmp.size() < 0)
                break;
            _read += 100 * write(tmp);
            emit newProgress((int)(_read / _maxSize));
        }
        file.close();
    }
signals:
    void newProgress(int percent);
private:
    qint64 _read, _maxSize;
    QList<QFileInfo> _infos;
};

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
    void die(QString message = "") {
        qDebug() << "Tool terminated early due to unforseen circumstances.";
        if (extracting) {
            foreach (QIODevice* dev, devHandle) {
                // QIODevice's are automatically closed.
                if (dev != nullptr) {
                    delete dev;
                    dev = nullptr;
                }
            }
            devHandle.clear();
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
        if (!message.isEmpty())
            QMessageBox::information(nullptr, "Error", message);
        emit finished();
    }

    void processSplitBar() {
        // And it seems we need a wrapper for splitting too.
        // Splitting is just extracting .signed, so lets integrate those after
        splitting = true;
        processExtractBar();
        foreach(QIODevice* dev, devHandle) {
            // QIODevice's are automatically closed.
            if (dev != nullptr) {
                delete dev;
                dev = nullptr;
            }
        }

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
