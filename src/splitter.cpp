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

void Splitter::processExtractType(FileSystemType type) {
    extracting = true;

    // TODO: Move 'detection' somewhere else and detect by header
    if (type == FS_UNKNOWN) {
        if (selectedFile.endsWith(".qnx6"))
            type = FS_QNX6;
        else if (selectedFile.endsWith(".rcfs"))
            type = FS_RCFS;
        else if (selectedFile.endsWith(".ifs"))
            type = FS_IFS;
        else
            return;
    }
    int unique = newProgressInfo(QFileInfo(selectedFile).size());
    QFileSystem* fs = createTypedFileSystem(type);
    connect(fs, &QFileSystem::sizeChanged, [=] (qint64 delta) {
        updateCurProgress(unique, fs->curSize, delta);
    });
    if (extractApps) {
        // TODO: This should be cleaner
        qobject_cast<FS::QNX6*>(fs)->extractApps = extractApps;
    }
    fs->extractContents();
    delete fs;

    emit finished();
}

void Splitter::processExtractSigned()
{
    extracting = true;
    signedFile = new QFile(selectedFile);
    if (!signedFile->open(QIODevice::ReadOnly)) {
        return die("Could not open " + selectedFile);
    }
    QString baseName = selectedFile;
    baseName.chop(7);
    processExtract(baseName, signedFile->size(), 0);
    signedFile->close();
    delete signedFile;
    emit finished();
}

QFileSystem* Splitter::createTypedFileSystem(FileSystemType type, qint64 offset, qint64 size, QString baseDir) {
    if (type == FS_RCFS)
        return new FS::RCFS(selectedFile, signedFile, offset, size, baseDir);
    else if (type == FS_QNX6)
        return new FS::QNX6(selectedFile, signedFile, offset, size, baseDir);
    else if (type == FS_IFS)
        return new FS::IFS(selectedFile, signedFile, offset, size, baseDir);

    return nullptr;
}

void Splitter::processExtract(QString baseName, qint64 signedSize, qint64 signedPos)
{
    QString baseDir = QFileInfo(baseName).absolutePath();

    if (signedPos > 0)
        signedFile->seek(signedPos);
    if (signedFile->read(4) != QByteArray("mfcq", 4)) {
        QMessageBox::information(nullptr, "Error", "Was not a Blackberry .signed image.");
        return;
    }
    QList<PartitionInfo> partInfo;
    progressInfo.clear();
    signedFile->seek(signedPos+12);

    // We are now at the partition table
    QByteArray partitionTable = signedFile->read(4000);
    QBuffer buffer(&partitionTable);
    buffer.open(QIODevice::ReadOnly);
    QNXStream tableStream(&buffer);
    int numPartitions, blockSize;
    tableStream >> numPartitions >> blockSize;

    if (numPartitions > 15) {
        QMessageBox::information(nullptr, "Error", "Bad partition table.");
        return;
    }

    partInfo.append(PartitionInfo(signedFile, signedPos + blockSize));
    int scan_offset = 0;
    for (int i = 0; i < numPartitions; i++) {
        int max_scan = 3;
        buffer.seek(0x20 + 0x40 * i + scan_offset);
        tableStream >> max_scan;
        qint64 blocks = 0;
        for (int j = 0; j < max_scan; j++) {
            buffer.seek(0x40 + 0x40 * i + 8*j + scan_offset);
            int nextOffset;
            tableStream >> nextOffset;
            if (nextOffset > 0) {
                blocks += nextOffset;
            }
            else
                break;
        }
        partInfo.last().size = blocks * (qint64)blockSize;
        partInfo.append(PartitionInfo(signedFile, partInfo.last().size + partInfo.last().offset));
        if (max_scan > 3) {
            scan_offset += (max_scan - 3) * 8;
        }

    }
    partInfo.last().size = signedPos + signedSize - partInfo.last().offset;

    // Would be great to get this at a container level (eg. for .exe or .zip) where multiple .signed partition tables are possible!
    // To do this, we either need to calculate size beforehand or extract afterwards. Latter is most likely
    // So, we would want a wrapper function for Image/Container and then go through each partition table, accumulating these info results.
    // Finally, we would perform the code below.
    foreach(PartitionInfo info, partInfo) {
        if (extractTypes & info.type) {
            maxSize += info.size;
        }
    }

    foreach(PartitionInfo info, partInfo) {
        if (info.type == FS_UNKNOWN)
            continue;
        if (extractTypes & info.type) {
            int unique = newProgressInfo(info.size);
            QFileSystem* fs = createTypedFileSystem(info.type, info.offset, info.size, baseDir);
            connect(fs, &QFileSystem::sizeChanged, [=] (qint64 delta) {
                updateCurProgress(unique, fs->curSize, delta);
            });
            if (extractApps) {
                // TODO: This should be cleaner
                qobject_cast<FS::QNX6*>(fs)->extractApps = extractApps;
            }
            if (extractImage)
                fs->extractImage();
            else
                fs->extractContents();

            delete fs;
        }
    }

}
