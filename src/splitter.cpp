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

#include "splitter.h"


// This is our entry point for extraction that will determine which filetype we started with.
// In general we have a container (.exe, .bar, .zip) which may contain further containers.
// Underneath this we may have disk images with partition tables (.signed)
// And below this we can have filesystem images (.rcfs, .qnx6, .ifs)
// Although, the user could make us enter at any point!
void Splitter::processExtractWrapper() {
    extracting = true;
    progressInfo.clear();
    partitionInfo.clear();
    QFileInfo fileInfo(selectedFile);

    // We can probably use a better method, similar to processExtractType, for all files!
    if (fileInfo.suffix() == "exe")
        processExtractAutoloader();
    else if (fileInfo.suffix() == "signed")
        processExtractSigned();
    else if (fileInfo.suffix() == "bar" || fileInfo.suffix() == "zip")
        processExtractBar();
    else // Assume it is a rcfs/qnx6/ifs
        processExtractType();

    // Now we should be done parsing through the entire file and its components
    // So run through the partition Info we collected
    // First gather sizes so we can have an established goal in the UI
    for (int i = 0; i < partitionInfo.count(); i++) {
        if (partitionInfo[i].size <= 65536) // Too small to have anything of value
            partitionInfo.removeAt(i--);
        else
            maxSize += partitionInfo[i].size;
    }

    // All files will be extracted relative to the given container file
    QString baseDir = QFileInfo(selectedFile).absolutePath();
    foreach(PartitionInfo info, partitionInfo) {
        int unique = newProgressInfo(info.size);
        // If we are extracting FS images (only type supported for this method right now), then we want to create a filesystem type
        QFileSystem* fs = createTypedFileSystem(selectedFile, info.dev, info.type, info.offset, info.size, baseDir);
        if (fs == nullptr)
            continue;
        connect(fs, &QFileSystem::sizeChanged, [=] (qint64 delta) {
            updateCurProgress(unique, fs->curSize, delta);
        });
        // TODO: This should be cleaner
        if (info.type == FS_QNX6) {
            qobject_cast<FS::QNX6*>(fs)->extractApps = extractApps;
        }
        if (extractImage)
            fs->extractImage();
        else
            fs->extractContents();

        delete fs;
    }
    emit finished();

    // Cleanup all pointers we had.
    foreach(QIODevice* dev, devHandle) {
        // QIODevice's are automatically closed.
        if (dev != nullptr) {
            dev->deleteLater();
            dev = nullptr;
        }
    }
    devHandle.clear();
    partitionInfo.clear();
    progressInfo.clear();
}

// Process an Autoloader with the aim of extracting files
void Splitter::processExtractAutoloader() {
    // We hardcode this only to speed it up. It isn't required and may cause issues later on.
#define START_CAP_SEARCH 0x400000
#define END_CAP_SEARCH 0x1000000
    QFile* autoloaderFile = new QFile(selectedFile);
    devHandle.append(autoloaderFile);
    autoloaderFile->open(QIODevice::ReadOnly);
    read = 0;
    maxSize = 1;
    int findHeader = 0;
    autoloaderFile->seek(START_CAP_SEARCH);
    for (int b = START_CAP_SEARCH; b < END_CAP_SEARCH; )
    {
        QByteArray tmp = autoloaderFile->read(BUFFER_LEN);
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
    QNXStream dataStream(autoloaderFile);
    autoloaderFile->seek(findHeader);

    // Search for offset table
    for (int attempts = 0; attempts < 32; attempts++) {
        qint64 tmp;
        dataStream >> tmp;
        if ((tmp - autoloaderFile->pos()) < 500 && (tmp - autoloaderFile->pos()) > -500) {
            autoloaderFile->seek(autoloaderFile->pos() - 16);
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
    offsets.append(autoloaderFile->size()); // End of file

    // Create sizes and files
    QString baseName = selectedFile;
    baseName.chop(4);
    // TODO: Use detection based on header. OS, Radio and PINList notify their header (but not RFOS?).
    // Not a priority as the size indication is 100% accurate as long as it is actually an Autoloader
    for (int i = 0; i < files; i++) {
        QString filename = baseName;
        qint64 size = offsets[i+1] - offsets[i];

        // Detection on type
        int type = 0;
        for (int index = 0; index < 1000 - 32; index += 4) {
            autoloaderFile->seek(offsets[i] + index);
            if (autoloaderFile->read(4) == QByteArray("pfcq",4)) {
                autoloaderFile->seek(offsets[i] + index + 12);
                qint8 typeChar;
                dataStream >> typeChar;
                if (typeChar == 5) {
                    type |= PACKED_FILE_USER;
                } else if (typeChar == 6) {
                    type |= PACKED_FILE_OS;
                } else if (typeChar == 10 || typeChar == 12) {
                    type |= PACKED_FILE_RADIO;
                } else if (typeChar == 8) {
                    type |= PACKED_FILE_IFS;
                    break;
                }

            }
        }
        // Detect by size alone
        if (type == 0) {
            if (size > 1024*1024*70)
                type = PACKED_FILE_OS;
            else if (size > 1024*1024*5)
                type = PACKED_FILE_RADIO;
            else
                type = PACKED_FILE_PINLIST;
        }

        // Append strings
        if (type & PACKED_FILE_USER)
            filename += "%User";
        if (type & PACKED_FILE_OS)
            filename += "%OS";
        if (type & PACKED_FILE_RADIO)
            filename += "%Radio";
        if (type & PACKED_FILE_IFS)
            filename += "%IFS";
        if (type & PACKED_FILE_PINLIST)
            filename += "%PINList";

        if (splitting) {
            if (option & type)
            {
                maxSize += size;
                QString unique = "";
                for (int i = 0; QFile::exists(filename + unique + ".signed"); i++)
                    unique = QString(".%1").arg(i);
                tmpFile.append(new QFile(filename+".signed"));
            }
            else
                tmpFile.append(nullptr);
        } else if (extracting) {
            if (type != PACKED_FILE_PINLIST)
                processExtract(autoloaderFile, size, offsets[i]);
        }
    }
    if (splitting) {
        // Write them out
        for (int i = 0; i < files; i++) {
            if (tmpFile.at(i) == nullptr)
                continue;
            autoloaderFile->seek(offsets[i]);
            tmpFile.at(i)->open(QIODevice::WriteOnly);
            tmpFile.at(i)->resize(offsets[i+1] - offsets[i]);
            QByteArray tmp;
            for (qint64 b = offsets[i]; b < offsets[i+1]; ) {
                qint64 read_len = qMin(BUFFER_LEN, offsets[i+1] - b);
                tmp = autoloaderFile->read(read_len);
                tmpFile.at(i)->write(tmp);
                b += updateProgress(tmp.size());
            }
            tmpFile.at(i)->close();
        }
        foreach (QFile* file, tmpFile)
            file->deleteLater();
        tmpFile.clear();
        autoloaderFile->close();
        delete autoloaderFile;
        autoloaderFile = nullptr;
    }
}

// Process a Disk Image with the aim of extracting files
void Splitter::processExtractSigned()
{
    QFile* file = new QFile(selectedFile); // Gets cleaned up later
    devHandle.append(file);                // By this
    if (!file->open(QIODevice::ReadOnly)) {
        return die("Could not open " + selectedFile);
    }
    processExtract(file, file->size(), 0);
}

// Process a zip container with the aim of extracting files
void Splitter::processExtractBar() {
    QuaZip barFile(selectedFile);
    barFile.open(QuaZip::mdUnzip);
    foreach (QString signedName, barFile.getFileNameList()) {
        if (QFileInfo(signedName).suffix() == "signed") {
            // Create a new internal QuaZip instance so we can successfully close the search instance but leave devHandle open
            QuaZipFile* signedFile = new QuaZipFile(selectedFile, signedName);
            devHandle.append(signedFile);
            barFile.setCurrentFile(signedName);
            signedFile->open(QIODevice::ReadOnly);
            if (splitting) {
                qint64 size = signedFile->size();
                int type = 0;
                if (size > 1024 * 1024 * 120)
                    type = PACKED_FILE_OS;
                else if (size > 1024 * 1024 * 5)
                    type = PACKED_FILE_RADIO;
                else
                    type = PACKED_FILE_PINLIST;
                if (option & type) {
                    read = 0;
                    maxSize = size;
                    progressChanged(0);
                    QFile outputSigned(QFileInfo(selectedFile).canonicalPath() + "/" + signedName);
                    outputSigned.open(QIODevice::WriteOnly);
                    outputSigned.resize(size);
                    QByteArray tmp;
                    for (qint64 b = size; b > 0; ) {
                        qint64 read_len = qMin(BUFFER_LEN, b);
                        tmp = signedFile->read(read_len);
                        outputSigned.write(tmp);
                        b -= updateProgress(tmp.size());
                    }
                    outputSigned.close();
                }
            } else if (extracting) {
                progressChanged(0);
                if (signedFile->size() > 1024*1024*5)
                    processExtract(signedFile, signedFile->size(), 0);
            }
        }
    }
    barFile.close();
}

// Process a Filesystem Image with the aim of extracting files
void Splitter::processExtractType() {
    QFile* imageFile = new QFile(selectedFile); // Gets cleaned up later
    devHandle.append(imageFile);                // By this
    if (!imageFile->open(QIODevice::ReadOnly)) {
        return die("Could not open " + selectedFile);
    }

    partitionInfo.append(PartitionInfo(imageFile, 0, imageFile->size()));
}

QFileSystem* Splitter::createTypedFileSystem(QString name, QIODevice* dev, QFileSystemType type, qint64 offset, qint64 size, QString baseDir) {
    if (type == FS_RCFS)
        return new FS::RCFS(name, dev, offset, size, baseDir);
    else if (type == FS_QNX6)
        return new FS::QNX6(name, dev, offset, size, baseDir);
    else if (type == FS_IFS)
        return new FS::IFS(name, dev, offset, size, baseDir);

    return nullptr;
}

void Splitter::processExtract(QIODevice* dev, qint64 signedSize, qint64 signedPos)
{
    if (signedPos > 0)
        dev->seek(signedPos);
    if (dev->read(4) != QByteArray("mfcq", 4)) {
        QMessageBox::information(nullptr, "Error", "Was not a Blackberry .signed image.");
        return;
    }
    QList<PartitionInfo> partInfo;
    dev->seek(signedPos+12);

    // We are now at the partition table
    QByteArray partitionTable = dev->read(4000);
    QBuffer buffer(&partitionTable);
    buffer.open(QIODevice::ReadOnly);
    QNXStream tableStream(&buffer);
    int numPartitions, firstOffset, unknown, startSearchOffset;
    tableStream >> numPartitions >> firstOffset;
    tableStream >> unknown >> startSearchOffset;

    if (numPartitions > 15) {
        QMessageBox::information(nullptr, "Error", "Bad partition table.");
        return;
    }
    // Possible issue with other Signed? So far this is always zero
    /*qint64 signedOffset;
    for (int i = startSearchOffset; i < 1000 - 32; i+=4) {
        dev->seek(signedPos+i);
        signedOffset = -1;
        if (dev->read(4) == QByteArray("pfcq",4) && partitionTable.at(i) == 5) { // PFCQ User FS
            int nextVal;
            buffer.seek(i+8);
            tableStream >> nextVal;
            qDebug() << nextVal;
            if (nextVal == 3) {
                qDebug() << "Got a WEIRD offset";
                int blockOffset, endOffset;
                buffer.seek(i+44);
                tableStream >> blockOffset;
                buffer.seek(i+56);
                tableStream >> endOffset;
                signedOffset = endOffset - blockOffset*16;
                break;
            } else if (nextVal == 1) {
                signedOffset = 0;
                break;
            }
        }
    }*/

    partInfo.append(PartitionInfo(dev, signedPos + firstOffset));
    for (int i = startSearchOffset; i < 1000 - 32; i += 4) {
        dev->seek(signedPos+i);
        if (dev->read(4) == QByteArray("pfcq",4)) {
            qint64 blocks = 0;
            int count = partitionTable.at(i+8);
            for (int j = 0; j < count; j++) {
                buffer.seek(i + j*16 + 44);
                int blockCount;
                tableStream >> blockCount;
                blocks += blockCount;
            }
            partInfo.last().size = blocks * (qint64)65536;
            // Type is at partitionTable.at(i). 5 is UserFS, 8 is Sig
            partInfo.append(PartitionInfo(dev, partInfo.last().offset + partInfo.last().size));
        }
    }
    partInfo.last().size = signedPos + signedSize - partInfo.last().offset;

    // Add to the main partition list
    foreach(PartitionInfo info, partInfo) {
        if (info.type == FS_UNKNOWN)
            continue;
        if (extractTypes & info.type) {
            partitionInfo.append(info);
        }
    }
}
