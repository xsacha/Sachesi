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
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDataStream>
#include <QDebug>
#include <QSettings>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

#define PACKED_FILE_OS      (1 << 0)
#define PACKED_FILE_RADIO   (1 << 1)
#define PACKED_FILE_PINLIST (1 << 2)

#define QCFM_IS_COMPRESSED  (1 << 23)
#define QCFM_IS_DIRECTORY   (1 << 14)
#define QCFM_IS_SYMLINK     (1 << 13)
#define BUFFER_LEN (qint64)4096

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

class Splitter: public QObject {
    Q_OBJECT

public:
    Splitter(QString file) : kill(false), selectedFile(file) { reset(); }
    Splitter(QString file, int options) : kill(false), option(options), selectedFile(file)  { reset(); }
    Splitter(QStringList files) : kill(false), selectedFiles(files)  { reset(); }
    ~Splitter() { }
    bool extractApps, extractImage;
    int extractTypes;
public slots:
    void reset() {
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
    void die() {
        qDebug() << "Tool terminated early due to unforseen circumstances.";
        if (extracting) {
            if (signedFile) {
                if (signedFile->isOpen())
                    signedFile->close();
                delete signedFile;
            }
        }
        if (splitting)
        {
            for (int i = 0; i < tmpFile.count(); i++)
            {
                if (tmpFile.at(i)->isOpen())
                {
                    tmpFile.at(i)->close();
                    tmpFile.at(i)->remove();
                }
            }
            tmpFile.clear();
        }
        extracting = false;
        combining = false;
        splitting = false;
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
            if (signedName.endsWith("signed")) {
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
#define END_CAP_SEARCH 0x800000
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
                    findHeader = b+i+12;
                }
            }
            b += tmp.size();
        }
        if (!findHeader) {
            QMessageBox::information(NULL, "Error", "Was not a Blackberry Autoloader file.");
            die();
        }
        qint64 files;
        QList<qint64> offsets;
        QNXStream dataStream(signedFile);
        signedFile->seek(findHeader);
        dataStream >> files;
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
            if (size > 1024*1024*100) {
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
                    tmpFile.append(NULL);
            } else if (extracting) {
                if (size > 1024*1024*5)
                    processExtract(filename, size, offsets[i]);
                // Deal with RCFS (OS+Radio), QNX6(OS), Apps(Large OS 500+MB)
            }
        }
        if (splitting) {
            // Write them out
            for (int i = 0; i < files; i++) {
                if (tmpFile.at(i) == NULL)
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
                delete tmpFile.at(i);
            }
            tmpFile.clear();
        }
        signedFile->close();
        delete signedFile;
    }
    void appendFile(QString fileName, QFile* target) {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        while (!file.atEnd())
        {
            QByteArray tmp = file.read(BUFFER_LEN);
            if (tmp.size() < 0)
                break;
            target->write(tmp);
            updateProgress(tmp.size());
        }
        file.close();
    }

    void processCombine() {
        combining = true;
        QFileInfo fileInfo(selectedFiles.first());
        // Fix up names
        QString baseDir = fileInfo.absolutePath();
        QString baseName = fileInfo.fileName().split('.').first();
        if (baseName.contains('-'))
        {
            QStringList baseList = baseName.split('-');
            baseName = baseList.at(baseList.count() - 2);
        }
        else if (baseName.contains('_'))
            baseName = baseName.split('_').last();
        if (!baseName.contains("autoloader", Qt::CaseInsensitive))
            baseName.prepend("Autoloader-");
        // Find potential file
        int f = 0;
        for (f = 0; QFile::exists(baseDir + "/" + baseName + (f == 0 ? "" : QString("-%1").arg(f)) + ".exe"); f++);
        // Open the new cap and append to it

        QSettings ini(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
        QString capPath = QFileInfo(ini.fileName()).absolutePath()+"/Sachesi/cap.exe";
        QFile newAutoloader(baseDir + "/" + baseName + (f == 0 ? "" : QString("-%1").arg(f)) + ".exe");
        newAutoloader.open(QIODevice::WriteOnly);
        appendFile(capPath, &newAutoloader);
        QByteArray dataHeader = QByteArray(56, 0);
        QNXStream dataStream(&dataHeader, QIODevice::WriteOnly);
        // This code is used as a separator
        dataStream << QByteArray::fromHex("9CD5C5979CD5C5979CD5C597") << (quint64)selectedFiles.count();
        quint64 counter = newAutoloader.pos()+52;
        read = 100 * counter;
        foreach (QString fileInfo, selectedFiles)
        {
            dataStream << counter;
            QFileInfo tmpFileInfo(fileInfo);
            counter += tmpFileInfo.size();
        }
        maxSize = counter;
        newAutoloader.write(dataHeader.right(52));
        foreach (QString file, selectedFiles)
            appendFile(file, &newAutoloader);
        newAutoloader.close();
        emit finished();
    }
    qint64 findIndexFromSig(unsigned char* signature, int startFrom, int distanceFrom, int num = 4, int skip = 1) {
        if (startFrom != -1)
            signedFile->seek(startFrom);
        int readlen = 4096;
        if (skip == 0x1000)
            readlen = num;
        while (!signedFile->atEnd())
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
            if (skip != 1)
                signedFile->seek(signedFile->pos() + skip - tmp.size());
        }
        return 0;
    }

    qint64 findSector(qint64 sector, qint64 startPos) {
        return ((sector - sectorOffset) * sectorSize) + startPos;
    }
    qint64 findNode(int node, qint64 startPos) {
        return (startPos) + (0x80 * (node - 1));
    }

    rinode createRNode(int offset, qint64 startPos) {
        QNXStream stream(signedFile);
        signedFile->seek(4 + offset + startPos);
        rinode ind;
        stream >> ind.mode >> ind.nameoffset >> ind.offset >> ind.size >> ind.time;
        signedFile->seek(ind.nameoffset + startPos);
        ind.name = QString(signedFile->readLine(128));
        if (ind.name == "")
            ind.name = ".";
        return ind;
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
        stream >> ind.tiers;
        return ind;
    }
    void extractManifest(int nodenum, qint64 startPos);
    void extractDir(int nodenum, QString basedir, qint64 startPos, int tier);
    int  processQStart(qint64 startPos, QString startDir);
    void processRStart(qint64 startPos, QString startDir);
    void extractRCFSDir(int offset, int numNodes, QString basedir, qint64 startPos);

    void processExtractSigned();
    void processExtract(QString baseName, qint64 signedSize, qint64 signedPos);
    void processExtractRCFS();
    void processExtractQNX6();

    quint64 updateProgress(quint64 delta) {
        read += 100 * delta;
        emit progressChanged((int)(read / maxSize));
        return delta;
    }
signals:
    void finished();
    void progressChanged(int progress);
    void error(QString err);
private:
    quint64 read, maxSize;
    bool splitting;
    bool combining;
    bool extracting;
    bool kill;
    int option;
    QString selectedFile;
    QStringList selectedFiles;
    QList<QFile*> tmpFile;
    QIODevice* signedFile;
    quint16 sectorSize;   // For extracting
    quint16 sectorOffset; // For extracting
    QList<int> lfn;       // For extracting
    QuaZip* currentZip;   // For extracting apps
    QList<QString> manifestApps;
};
