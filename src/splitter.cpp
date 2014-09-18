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
    int unique = newProgressInfo();
    if (type == FS_QNX6) {
        FS::QNX6* qnx = new FS::QNX6(selectedFile);
        qnx->extractApps = extractApps;
        connect(qnx, &FS::QNX6::sizeChanged, [=] () {
            updateCurProgress(unique, qnx->curSize, qnx->maxSize);
        });
        qnx->extractContents();
        delete qnx;
    } else if (type == FS_RCFS) {
        FS::RCFS* rcfs = new FS::RCFS(selectedFile);
        connect(rcfs, &FS::RCFS::sizeChanged, [=] () {
            updateCurProgress(unique, rcfs->curSize, rcfs->maxSize);
        });
        rcfs->extractContents();
        delete rcfs;
    } else if (type == FS_IFS) {
        FS::IFS* ifs = new FS::IFS(selectedFile);
        connect(ifs, &FS::IFS::sizeChanged, [=] () {
            updateCurProgress(unique, ifs->curSize, ifs->maxSize);
        });
        ifs->extractContents();
        delete ifs;
    }

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

void Splitter::processExtract(QString baseName, qint64 signedSize, qint64 signedPos)
{
    qint64 startPos = 0;
    QString baseDir = QFileInfo(baseName).absolutePath();

    if (signedPos > 0)
        signedFile->seek(signedPos);
    if (signedFile->read(4) != QByteArray("mfcq", 4)) {
        QMessageBox::information(nullptr, "Error", "Was not a Blackberry .signed image.");
        return;
    }
    QList<PartitionInfo> partInfo;
    signedFile->seek(signedPos+12);

    // We are now at the partition table
    QByteArray partitionTable = signedFile->read(4000);
    QBuffer buffer(&partitionTable);
    buffer.open(QIODevice::ReadOnly);
    QNXStream tableStream(&buffer);
    int numPartitions, blockSize;
    tableStream >> numPartitions >> blockSize;

    if (numPartitions > 10) {
        QMessageBox::information(nullptr, "Error", "Bad partition table.");
        return;
    }

    partInfo.append(PartitionInfo(signedPos + blockSize));
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
        if (extractTypes & partInfo.last().type)
            maxSize += partInfo.last().size;
        partInfo.append(PartitionInfo(partInfo.last().size + partInfo.last().offset));
        // Check what sort of image we are dealing with
        signedFile->seek(partInfo.last().offset);
        QByteArray header = signedFile->read(4);
        if (header == QByteArray("rimh", 4))
            partInfo.last().type = FS_RCFS;
        else if (header == QByteArray::fromHex("EB109000"))
            partInfo.last().type = FS_QNX6;
        else if (header == QByteArray::fromHex("FE0300EA"))
            partInfo.last().type = FS_IFS;
        if (max_scan > 3) {
            scan_offset += (max_scan - 3) * 8;
        }

    }
    partInfo.last().size = signedPos + signedSize - partInfo.last().offset;
    if (extractTypes & partInfo.last().type)
        maxSize += partInfo.last().size;

    foreach(PartitionInfo info, partInfo) {
        if (info.type == FS_UNKNOWN)
            continue;
        if (extractTypes & info.type) {
            if (info.type == FS_RCFS) {
                int unique = newProgressInfo();
                FS::RCFS* rcfs = new FS::RCFS(selectedFile, signedFile, info.offset, info.size /*+ 0x1000*/, baseDir);
                connect(rcfs, &FS::RCFS::sizeChanged, [=] () {
                    updateCurProgress(unique, rcfs->curSize, rcfs->maxSize);
                });
                if (extractImage) {
                    rcfs->extractImage();
                } else {
                    rcfs->extractContents();
                }
                delete rcfs;
            } else if (info.type == FS_QNX6) {
                int unique = newProgressInfo();
                FS::QNX6* qnx = new FS::QNX6(selectedFile, signedFile, info.offset, info.size, baseDir);
                qnx->extractApps = extractApps;
                connect(qnx, &FS::QNX6::sizeChanged, [=] () {
                    updateCurProgress(unique, qnx->curSize, qnx->maxSize);
                });
                if (extractImage) {
                    qnx->extractImage();
                } else {
                    qnx->extractContents();
                }
                delete qnx;
            } else if (info.type == FS_IFS) {
                int unique = newProgressInfo();
                FS::IFS* ifs = new FS::IFS(selectedFile, signedFile, info.offset, info.size, baseDir);
                connect(ifs, &FS::IFS::sizeChanged, [=] () {
                    updateCurProgress(unique, ifs->curSize, ifs->maxSize);
                });
                if (extractImage) {
                    ifs->extractImage();
                } else {
                    ifs->extractContents();
                }
                delete ifs;
            }
        }
    }

}
