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
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include "ports.h"

enum FileSystemType {
    FS_UNKNOWN = 0,
    FS_QNX6,
    FS_RCFS,
    FS_IFS,
};

#define PACKED_FILE_OS      (1 << 0)
#define PACKED_FILE_RADIO   (1 << 1)
#define PACKED_FILE_PINLIST (1 << 2)

// node.mode flags
#define QCFM_IS_COMPRESSED      ((1 << 22) | (1 << 23) | (1 << 24))
// lzo1x_decompress_safe
#define QCFM_IS_LZO_COMPRESSED   (1 << 23)
// ucl_nrv2b_decompress_8
#define QCFM_IS_UCL_COMPRESSED  ((1 << 22) | (1 << 23))
#define QCFM_IS_GZIP_COMPRESSED  (1 << 22)
#define QCFM_IS_DIRECTORY        (1 << 14)
#define QCFM_IS_SYMLINK          (1 << 13)

#define BUFFER_LEN (qint64)4096
// QNX6 maximum filename length
#define QNX6_MAX_CHARS 510

#define READ_TMP(x, y) x y; stream >> y;

class QNXStream : public QDataStream {
public:
    QNXStream(QIODevice * device)
    : QDataStream(device) {
        setByteOrder(QDataStream::LittleEndian);
    }
    QNXStream(QByteArray * a, QIODevice::OpenMode flags)
    : QDataStream(a, flags) {
        setByteOrder(QDataStream::LittleEndian);
        resetStatus();
    }
};

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
            append = QString("-%1.exe").arg(f);
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
        for (int i = _infos.count() - 1; i < 6; i++)
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

struct qinode {
    int size;
    QList<int> sectors;
    quint8 tiers;
    int time;
    quint16 perms;
};

struct rinode {
    int mode;
    QString name;
    int nameoffset;
    int offset;
    int size;
    int time;
    QString path_to;
    int chunks;
};

struct binode {
    int mode;
    QString name;
    int offset;
    // TODO: Sizes greater than 16-bit?
    qint16 size;
    int time;
    QString path_to;
    int chunks;
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
        signedFile = nullptr;
        currentZip = nullptr;
    }

    void killSplit() {
        kill = true;
        die();
    }
    void die(QString message = "") {
        qDebug() << "Tool terminated early due to unforseen circumstances.";
        if (extracting) {
            if (signedFile != nullptr) {
                if (signedFile->isOpen())
                    signedFile->close();
                delete signedFile;
                signedFile = nullptr;
            }
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
        splitting = true;
        scanBar();
        emit finished();
    }

    void processExtractBar() {
        extracting = true;
        scanBar();
        emit finished();
    }

    void scanBar() {
        QuaZip barFile(selectedFile);
        barFile.open(QuaZip::mdUnzip);
        foreach (QString signedName, barFile.getFileNameList()) {
            if (QFileInfo(signedName).suffix() == "signed") {
                signedFile = new QuaZipFile(&barFile);
                barFile.setCurrentFile(signedName);
                signedFile->open(QIODevice::ReadOnly);
                QString baseName = QFileInfo(selectedFile).canonicalPath() + "/" + signedName;
                if (splitting) {
                    read = 0;
                    maxSize = signedFile->size();
                    progressChanged(0);
                    QFile outputSigned(baseName);
                    outputSigned.open(QIODevice::WriteOnly);
                    outputSigned.resize(signedFile->size());
                    QByteArray tmp;
                    for (qint64 b = signedFile->size(); b > 0; ) {
                        qint64 read_len = qMin(BUFFER_LEN, b);
                        tmp = signedFile->read(read_len);
                        outputSigned.write(tmp);
                        b -= updateProgress(tmp.size());
                    }
                    outputSigned.close();
                } else if (extracting) {
                    progressChanged(0);
                    baseName.chop(7);
                    if (signedFile->size() > 1024*1024*5)
                        processExtract(baseName, signedFile->size(), 0);
                }
                signedFile->close();
                delete signedFile;
                signedFile = nullptr;
            }
        }
        barFile.close();
    }

    void processSplitAutoloader() {
        splitting = true;
        scanAutoloader();
        emit finished();
    }

    void processExtractAutoloader() {
        extracting = true;
        scanAutoloader();
        emit finished();
    }

    void scanAutoloader() {
        // We hardcode this only to speed it up. It isn't required and may cause issues later on.
#define START_CAP_SEARCH 0x400000
#define END_CAP_SEARCH 0x1000000
        signedFile = new QFile(selectedFile);
        signedFile->open(QIODevice::ReadOnly);
        read = 0;
        maxSize = 1;
        int findHeader = 0;
        signedFile->seek(START_CAP_SEARCH);
        for (int b = START_CAP_SEARCH; b < END_CAP_SEARCH; )
        {
            QByteArray tmp = signedFile->read(BUFFER_LEN);
            for (int i = 0; i < BUFFER_LEN - 12; i++) {
                if (tmp.at(i) == (char)0x9C && tmp.at(i+1) == (char)0xD5 && tmp.at(i+2) == (char)0xC5 && tmp.at(i+3) == (char)0x97 &&
                        tmp.at(i+8) == (char)0x9C && tmp.at(i+9) == (char)0xD5 && tmp.at(i+10) == (char)0xC5 && tmp.at(i+11) == (char)0x97) {
                    findHeader = b+i+20;
                }
            }
            b += tmp.size();
        }

        if (!findHeader) {
            return die("Was not a Blackberry Autoloader file.");
        }

        qint64 files;
        QList<qint64> offsets;
        QNXStream dataStream(signedFile);
        signedFile->seek(findHeader);

        // Search for offset table
        for (int attempts = 0; attempts < 32; attempts++) {
            qint64 tmp;
            dataStream >> tmp;
            if ((tmp - signedFile->pos()) < 500 && (tmp - signedFile->pos()) > -500) {
                signedFile->seek(signedFile->pos() - 16);
                dataStream >> files;
                break;
            }
        }

        if (files < 1 || files > 20) {
            return die("Unknown Blackberry Autoloader file.");
        }

        // Collect offsets
        for (int i = 0; i < files; i++) {
            offsets.append(0); dataStream >> offsets[i];
        }
        offsets.append(signedFile->size()); // End of file

        // Create sizes and files
        QString baseName = selectedFile;
        baseName.chop(4);
        // TODO: Use detection based on header. OS, Radio and PINList notify their header (but not RFOS?).
        // Not a priority as the size indication is 100% accurate as long as it is actually an Autoloader
        for (int i = 0; i < files; i++) {
            QString filename = baseName;
            qint64 size = offsets[i+1] - offsets[i];
            int type = 0;
            if (size > 1024*1024*120) {
                type = PACKED_FILE_OS;
                filename += QString("-OS.%1").arg(i);
            } else if (size > 1024*1024*5) {
                type = PACKED_FILE_RADIO;
                filename += QString("-Radio.%1").arg(i);
            } else {
                type = PACKED_FILE_PINLIST;
                filename += QString("-PINList.%1").arg(i);
            }
            if (splitting) {
                if (option & type)
                {
                    maxSize += size;
                    tmpFile.append(new QFile(filename+".signed"));
                }
                else
                    tmpFile.append(nullptr);
            } else if (extracting) {
                if (size > 1024*1024*5)
                    processExtract(filename, size, offsets[i]);
                // Deal with RCFS (OS+Radio), QNX6(OS), Apps(Large OS 500+MB)
            }
        }
        if (splitting) {
            // Write them out
            for (int i = 0; i < files; i++) {
                if (tmpFile.at(i) == nullptr)
                    continue;
                signedFile->seek(offsets[i]);
                tmpFile.at(i)->open(QIODevice::WriteOnly);
                tmpFile.at(i)->resize(offsets[i+1] - offsets[i]);
                QByteArray tmp;
                for (qint64 b = offsets[i]; b < offsets[i+1]; ) {
                    qint64 read_len = qMin(BUFFER_LEN, offsets[i+1] - b);
                    tmp = signedFile->read(read_len);
                    tmpFile.at(i)->write(tmp);
                    b += updateProgress(tmp.size());
                }
                tmpFile.at(i)->close();
            }
            foreach (QFile* file, tmpFile)
                    file->deleteLater();
            tmpFile.clear();
        }
        signedFile->close();
        delete signedFile;
        signedFile = nullptr;
    }

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
    qint64 findIndexFromSig(unsigned char* signature, int startFrom, int distanceFrom, unsigned int maxBlocks = -1, int num = 4) {
        if (startFrom != -1)
            signedFile->seek(startFrom);
        int readlen = BUFFER_LEN;
        while (!signedFile->atEnd() && (maxBlocks-- != 0))
        {
            QByteArray tmp = signedFile->read(readlen);
            if (tmp.size() < 0)
                break;
            for (int i = 0; i < tmp.size(); i++)
            {
                bool found = true;
                for (int j = 0; j < num; j++)
                    if ((unsigned char)tmp[i+j] != signature[j]) {
                        found = false;
                        break;
                    }
                if (found)
                    return (qint64)(signedFile->pos() - tmp.size() + i + distanceFrom);
            }
        }
        return 0;
    }

    qint64 findSector(qint64 sector, qint64 startPos) {
        return ((sector - sectorOffset) * sectorSize) + startPos;
    }
    qint64 findNode(int node, qint64 startPos) {
        return (startPos) + (0x80 * (node - 1));
    }

    qinode createNode(int node, qint64 startPos) {
        QNXStream stream(signedFile);
        qinode ind;
        qint64 base = findNode(node, startPos);
        signedFile->seek(base);
        stream >> ind.size;
        signedFile->seek(base + 0x10);
        stream >> ind.time;
        signedFile->seek(base + 0x20);
        stream >> ind.perms;
        stream.skipRawData(2);
        int sector;
        for (int i = 0; i < 16; i++)
        {
            stream >> sector;
            if (sector != -1)
                ind.sectors.append(sector);
        }
        // No sectors appears to be caused by empty files. These need to be extracted
        if (ind.sectors.count() == 0) {
            ind.sectors.append(0);
        }

        stream >> ind.tiers;
        return ind;
    }

    rinode createRNode(int offset, qint64 startPos) {
        QNXStream stream(signedFile);
        signedFile->seek(4 + offset + startPos);
        rinode ind;
        stream >> ind.mode >> ind.nameoffset >> ind.offset >> ind.size >> ind.time;
        signedFile->seek(ind.nameoffset + startPos);
        ind.name = QString(signedFile->readLine(QNX6_MAX_CHARS));
        if (ind.name == "")
            ind.name = ".";
        return ind;
    }

    binode createBNode(int offset, qint64 startPos) {
        QNXStream stream(signedFile);
        signedFile->seek(startPos + offset);
        binode ind;
        stream >> ind.mode >> ind.size >> ind.time;

        ind.name = QString(signedFile->readLine(QNX6_MAX_CHARS));
        if (ind.name == "")
            ind.name = ".";
        ind.offset = signedFile->pos() - startPos;

        return ind;
    }
    void extractManifest(int nodenum, qint64 startPos);
    void extractDir(int nodenum, QString basedir, qint64 startPos, int tier);
    void extractRCFSDir(int offset, int numNodes, QString basedir, qint64 startPos);
    void extractBootDir(int offset, int numNodes, QString basedir, qint64 startPos);
    int  processQStart(qint64 startPos, QString startDir);
    void processRStart(qint64 startPos, QString startDir);
    void processBStart(qint64 startPos, QString startDir, qint64 size);

    void processExtractSigned();
    void processExtract(QString baseName, qint64 signedSize, qint64 signedPos);
    void processExtractType(FileSystemType type = FS_UNKNOWN);

    QString generateNameFromRCFS(qint64 startPos);
    QString generateNameFromIFS(qint64 startPos, int count);

    QByteArray extractRCFSFile(qint64 node_offset, int node_size, int node_mode);

    quint64 updateProgress(qint64 delta) {
        if (delta < 0)
            return 0;
        read += 100 * delta;
        emit progressChanged((int)(read / maxSize));
        return delta;
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
    QIODevice* signedFile;
    quint16 sectorSize;   // For extracting
    quint16 sectorOffset; // For extracting
    QList<int> lfn;       // For extracting
    QuaZip* currentZip;   // For extracting apps
    QList<QString> manifestApps;
};
