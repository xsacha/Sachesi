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
#include "lzo.h"
#ifdef _WIN32
#include "Windows.h"
// We need to update the time of the extracted file based on the unix filesystem it comes from.
void fixFileTime(QString filename, int time) {
    FILETIME pft;
    LONGLONG ll = Int32x32To64(time, 10000000) + 116444736000000000;
    pft.dwLowDateTime = (DWORD)ll;
    pft.dwHighDateTime = ll >> 32;
    HANDLE fd_handle = CreateFile(filename.toStdWString().c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    SetFileTime(fd_handle, &pft,(LPFILETIME) nullptr, &pft);
    CloseHandle(fd_handle);
}
#endif

void Splitter::extractManifest(int nodenum, qint64 startPos) {
    QNXStream stream(signedFile);
    int count;
    qinode ind = createNode(nodenum, startPos);
    foreach(int num, ind.sectors)
    {
        for (int i = 0; i < 0x1000; i += 0x20)
        {
            if (kill) return die();
            updateProgress(1400);
            signedFile->seek(findSector(num, startPos) + i);
            READ_TMP(int, inodenum);
            if (inodenum == 0)
                break;

            READ_TMP(unsigned char, c_byte);
            count = c_byte;
            if (count == 0xFF)
            {
                signedFile->seek(findSector(num, startPos) + i + 0x8);
                READ_TMP(int, item);
                if (item > lfn.count())
                    item = 1;
                signedFile->seek(findSector(lfn.at(item), startPos));
                READ_TMP(unsigned short, c_sint);
                count = c_sint;
            }
            QString dir = QString(signedFile->read(count));
            if (dir == "." || dir == "..")
                continue;
            qinode ind2 = createNode(inodenum, startPos);
            if (ind2.perms & QCFM_IS_DIRECTORY && dir == "META-INF") {
                extractDir(inodenum, dir, startPos, 3);
            }
        }
    }
}

void Splitter::extractDir(int nodenum, QString basedir, qint64 startPos, int tier)
{
    QDir mainDir;
    if (!extractApps)
        mainDir.mkdir(basedir);

    QNXStream stream(signedFile);
    int count;
    qinode ind = createNode(nodenum, startPos);
    foreach(int num, ind.sectors)
    {
        for (int i = 0; i < 0x80; i++)
        {
            if (kill)
                return die();

            signedFile->seek(findSector(num, startPos) + (i * 0x20));
            READ_TMP(int, inodenum);
            if (inodenum == 0)
                break;

            READ_TMP(unsigned char, c_byte);
            count = c_byte;
            if (count == 0xFF)
            {
                signedFile->seek(findSector(num, startPos) + (i * 0x20) + 0x8);
                READ_TMP(int, item);
                if (item > lfn.count())
                    item = 1;
                signedFile->seek(findSector(lfn.at(item), startPos));
                READ_TMP(unsigned short, c_sint);
                count = c_sint;
            }
            QString dir = QString(signedFile->read(count));
            if (dir == "." || dir == "..")
                continue;

            qinode ind2 = createNode(inodenum, startPos);

            if (ind2.perms & QCFM_IS_DIRECTORY)
            {
                if (extractApps) {
                    if (dir == "apps" && tier == 0) {
                        mainDir.mkdir(basedir);
                        extractDir(inodenum, basedir, startPos, 1);
                        continue;
                    }
                    else if (tier == 1) {
                        if (!dir.contains(".gY"))
                            continue;
                        dir = dir.split(".gY").first();
                        currentZip = new QuaZip(basedir + "/" + dir+".bar");
                        currentZip->open(QuaZip::mdCreate);
                        extractManifest(inodenum, startPos);
                        extractDir(inodenum, "", startPos, 2);
                        currentZip->close();
                        manifestApps.clear();
                        delete currentZip;
                        continue;
                    } else if (tier == 2 && dir != "META-INF") {
                        extractDir(inodenum, dir, startPos, 3);
                        continue;
                    }
                }
                extractDir(inodenum, basedir + "/" + dir, startPos, tier ? tier + 1 : 0);
                continue;
            }
            if (extractApps) {
                if (!tier)
                    continue;
                QString thisFile = (tier == 2) ? dir : (basedir + "/" + dir);
                if (!manifestApps.isEmpty() && basedir != "META-INF" && !manifestApps.contains(thisFile))
                    continue;
            }
            QList<int> sections;
            if (ind2.tiers == 0 && (ind2.sectors[0] > 0)) {
                foreach(int sector, ind2.sectors) {
                    if (sector != -1)
                        sections.append(sector);
                }
            } else if (ind2.tiers > 0 ) {
                foreach (int sector, ind2.sectors) {
                    if (sector == -1) break;
                    signedFile->seek(findSector(sector, startPos));
                    for (int j = 0; j < 0x400; j++)
                    {
                        stream >> sector;
                        if (sector > 0)
                            sections.append(sector);
                    }
                }
                if (ind2.tiers == 2) {
                    QList<int> nodes = sections;
                    sections.clear();
                    foreach (int fn, nodes) {
                        if (fn == -1) break;
                        signedFile->seek(findSector(fn, startPos));
                        for (int tmpx = 0; tmpx < 1024; tmpx++) {
                            stream >> fn;
                            if (fn > 0) sections.append(fn);
                        }
                    }
                }
            }

            QuaZipFile* zipFile = 0;
            QFile* newFile = 0;
            bool isManifest = (tier == 3) && (dir == "MANIFEST.MF");
            QByteArray manifestDump;
            if (extractApps)
            {
                zipFile = new QuaZipFile(currentZip);
                zipFile->open(QIODevice::WriteOnly, tier == 2 ? QuaZipNewInfo(dir) : QuaZipNewInfo(basedir + "/" + dir), nullptr, 0, 8);
            } else {
                newFile = new QFile(basedir + "/" + dir);
                newFile->open(QIODevice::WriteOnly);
            }
            if (ind2.size != 0) {
                foreach(int section, sections)
                {
                    signedFile->seek(findSector(section, startPos));
                    int len = sectorSize;
                    if (section == sections.last() && (ind2.size % sectorSize))
                        len = ind2.size % sectorSize;
                    updateProgress(len);
                    QByteArray tmp = signedFile->read(len);
                    if (extractApps)
                    {
                        zipFile->write(tmp);
                        if (isManifest)
                            manifestDump.append(tmp);
                    }
                    else
                        newFile->write(tmp);
                }
            }
            if (extractApps)
            {
                zipFile->close();
                delete zipFile;
                if (isManifest) {
                    foreach(QByteArray manifestString, manifestDump.split('\n')) {
                        QString tmp = QString(manifestString).simplified();
                        if (tmp.startsWith("Archive-Asset-Name:")) {
                            manifestApps.append(tmp.split(": ").last());
                        }
                    }
                }
            }
            else {
                newFile->close();
#ifdef _WIN32
                fixFileTime(newFile->fileName(), ind.time);
#endif
                delete newFile;
            }
        }
    }
}

void Splitter::extractRCFSDir(int offset, int numNodes, QString basedir, qint64 startPos)
{
    QNXStream stream(signedFile);
    QDir mainDir(basedir);
    for (int i = 0; i < numNodes; i++) {
        rinode node = createRNode(offset + (i * 0x20), startPos);
        node.path_to = basedir;
        if (node.mode & QCFM_IS_DIRECTORY) {
            mainDir.mkpath(node.name);
            if (node.size > 0)
                extractRCFSDir(node.offset, node.size / 0x20, node.path_to + "/" + node.name, startPos);
        }
        else {
            signedFile->seek(node.offset + startPos);
            if (node.mode & QCFM_IS_SYMLINK) {
#ifdef _WIN32
                QString lnkName = node.path_to + "/" + node.name + ".lnk";
                QFile::link(node.path_to + "/" + signedFile->readLine(128), lnkName);
                fixFileTime(lnkName, node.time);
#else
                QFile::link(node.path_to + "/" + signedFile->readLine(128), node.path_to + "/" + node.name);
#endif
                continue;
            }
            QFile newFile(node.path_to + "/" + node.name);
            newFile.open(QFile::WriteOnly);
            if (node.mode & QCFM_IS_COMPRESSED) {
                READ_TMP(int, next);
                int chunks = (next - 4) / 4;
                QList<int> sizes, offsets;
                offsets.append(next);
                for (int s = 0; s < chunks; s++) {
                    stream >> next;
                    offsets.append(next);
                    sizes.append(offsets[s+1] - offsets[s]);
                }
                char* buffer = new char[node.size];
                foreach(int size, sizes) {
                    char* readData = new char[size];
                    signedFile->read(readData, size);
                    size_t write_len = 0x4000;
                    lzo1x_decompress_safe(reinterpret_cast<const unsigned char*>(readData), size, reinterpret_cast<unsigned char*>(buffer), &write_len, nullptr);
                    newFile.write(buffer, (qint64)write_len);
                    updateProgress(size);
                    delete [] readData;
                }
                delete [] buffer;
            } else {
                for (qint64 i = node.size; i > 0;) {
                    qint64 read_len = qMin(BUFFER_LEN, i);
                    qint64 written_size = newFile.write(signedFile->read(read_len));
                    i -= updateProgress(written_size);
                }
            }
            newFile.close();
#ifdef _WIN32
            fixFileTime(newFile.fileName(), node.time);
#endif
        }
    }
}

void Splitter::processExtractRCFS() {
    extracting = true;
    signedFile = new QFile(selectedFile);
    signedFile->open(QIODevice::ReadOnly);
    read = 0;
    maxSize = signedFile->size() * 1.25;
    progressChanged(0);
    QString baseName = selectedFile;
    baseName.chop(5);
    processRStart(0, baseName);
    signedFile->close();
    delete signedFile;
    emit finished();
}

void Splitter::processExtractQNX6() {
    extracting = true;
    signedFile = new QFile(selectedFile);
    signedFile->open(QIODevice::ReadOnly);
    read = 0;
    progressChanged(0);
    QString baseName = selectedFile;
    baseName.chop(5);
    maxSize = signedFile->size();
    processQStart(0, baseName);
    signedFile->close();
    delete signedFile;
    emit finished();
}

int Splitter::processQStart(qint64 startPos, QString startDir) {
    QNXStream stream(signedFile);
    signedFile->seek(startPos+8);
    READ_TMP(unsigned char, typeQNX); // 0x10 = no offset; 0x08 = has offset
    unsigned char bootSig[] = {0xDD, 0xEE, 0xE6, 0x97};
    signedFile->seek(startPos + 0x2000);
    if ( (startPos = findIndexFromSig(bootSig, -1, 0)) == 0) { return 1; }
    signedFile->seek(startPos+48);
    stream >> sectorSize;
    if (sectorSize % 512) { return 1; }
    startPos += sectorSize;

    // Find sectorOffset
    qinode ind2 = createNode(1, startPos);
    // Try all offsets
    if (typeQNX == 0x10)
    {
        sectorOffset = 0;
        signedFile->seek((ind2.sectors[0] - sectorOffset)*sectorSize + startPos);
        READ_TMP(int, offsetCheck);
        if (offsetCheck != 1)
            typeQNX = 8;
    }
    if (typeQNX != 0x10) {
        for (sectorOffset = 0x6320; sectorOffset < 0x6400; sectorOffset += 0x10)
        {
            signedFile->seek((ind2.sectors[0] - sectorOffset)*sectorSize + startPos);
            READ_TMP(int, offsetCheck);
            if (offsetCheck == 1)
                break;
        }
    }

    for (qint64 s = startPos - 0xF10; true; s+=4)
    {
        signedFile->seek(s);
        READ_TMP(int, next);
        if (next == -1) break;
        signedFile->seek(findSector(next, startPos));
        for (int i = 0; i < 0x400; i++) {
            stream >> next;
            if (next > 0) lfn.append(next);
        }
    }
    extractDir(1, startDir, startPos, 0);
    return 0;
}

void Splitter::processRStart(qint64 startPos, QString startDir) {
    signedFile->seek(startPos + 0x1038);
    QNXStream stream(signedFile);
    READ_TMP(int, offset);
    extractRCFSDir(offset, 1, startDir, startPos);
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

    bool hasApps = (signedSize > 500*1024*1024);
    if (signedPos > 0)
        signedFile->seek(signedPos);
    if (signedFile->read(4) != QByteArray("mfcq", 4)) {
        QMessageBox::information(nullptr, "Error", "Was not a Blackberry .signed image.");
        return;
    }
    QNXStream stream(signedFile);
    QList<qint64> partitionOffsets, partitionSizes;
    signedFile->seek(signedPos+12);
    READ_TMP(int, numPartitions);
    READ_TMP(int, blockSize);
    if (numPartitions > 10) {
        QMessageBox::information(nullptr, "Error", "Bad partition table.");
        return;
    }
    int nextOffset;
    qint64 movingOffset = signedPos + blockSize;
    partitionOffsets.append(movingOffset);
    int scan_offset = 0;
    for (int i = 1; i < numPartitions + 1; i++) {
        int max_scan = 3;
        signedFile->seek(signedPos + 0xc - 0x20 + (0x40 * i) + scan_offset);
        stream >> max_scan;
        for (int j = 0; j < max_scan; j++) {
            signedFile->seek(signedPos + 0xc + (0x40 * i) + (8*j) + scan_offset);
            stream >> nextOffset;
            if (nextOffset > 0) {
                movingOffset += nextOffset * blockSize;
                signedFile->seek(movingOffset);
                QByteArray header = signedFile->read(4);
                if (header != QByteArray(4,(char)0)) {
                    partitionOffsets.append(movingOffset);
                    partitionSizes.append(partitionOffsets.last() - partitionOffsets[partitionOffsets.count() - 2]);
                }
            }
            else
                j = 0xd7ad;
        }
        if (max_scan > 3) {
            scan_offset += (max_scan - 3) * 8;
        }

    }
    partitionSizes.append(signedPos + signedSize - partitionOffsets.last());


    // Detect if RCFS exists in this file
    if (!extractApps && (extractTypes & 1)) {
        foreach(int partitionOffset, partitionOffsets) {
            signedFile->seek(partitionOffset);
            QByteArray header = signedFile->read(4);
            char unknownSig[] = {0x0,0x10,0x0,0x0};
            if (header == QByteArray("rimh",4) || header == QByteArray(unknownSig,4)) {
                startPos = partitionOffset;
                QString rcfsTypeString = "";
                if (header == QByteArray("rimh",4)) {
                    READ_TMP(int, rcfsType);
                    rcfsTypeString = QString(signedFile->readLine(5));
                    if (rcfsTypeString.startsWith("fs-r"))
                        rcfsTypeString = "Radio";
                    else if (rcfsTypeString.startsWith("fs-o"))
                        rcfsTypeString = "OS";
                    if (baseName.contains(rcfsTypeString))
                        rcfsTypeString = "";
                    else
                        rcfsTypeString.prepend("-");
                }
                signedFile->seek(startPos + 0x1030);
                READ_TMP(int, rcfsSize);
                maxSize += rcfsSize * 1.25;
                signedFile->seek(startPos);
                if (!extractImage) {
                    processRStart(startPos, baseName + rcfsTypeString);
                } else {
                    // Extract the file
                    QScopedPointer<QFile> rcfsFile(new QFile(baseName + rcfsTypeString + ".rcfs"));
                    if (!rcfsFile->open(QIODevice::WriteOnly))
                        return die();

                    for (qint64 s = rcfsSize + 0x1000; s > 0; s -= updateProgress(rcfsFile->write(signedFile->read(qMin(BUFFER_LEN, s)))));
                    rcfsFile->close();
                }
            }
        }
    }

    // Now extract the user partition
    if (hasApps && (extractTypes & 2)) {
        int qnxcounter = 0;
        char qnx6Sig[] = {(char)0xEB, 0x10, (char)0x90, 0x0};
        if (!extractImage) {
            maxSize += (signedFile->size() - partitionOffsets[0]) * 0.58; // Guesstimate. I think the real size is in the header
        }
        for (int i = 0; i < partitionOffsets.count(); i++) {
            signedFile->seek(partitionOffsets[i]);
            if (signedFile->read(4) == QByteArray(qnx6Sig,4)) {
                startPos = partitionOffsets[i];
                signedFile->seek(startPos);
                QString type = extractApps ? "Apps" : "OS";
                if (!extractApps && baseName.contains("OS"))
                    type == "";
                else
                    type.prepend("-");
                if (!extractImage) {
                    if (!processQStart(startPos, baseName + type))
                        break;
                } else {
                    maxSize += partitionSizes[i];
                    // Extract the file
                    QScopedPointer<QFile> qnx6File(new QFile(QString(baseName + ".%1.qnx6").arg(qnxcounter++)));
                    if (!qnx6File->open(QIODevice::WriteOnly))
                        return die();

                    for (qint64 s = partitionSizes[i]; s > 0; s -= updateProgress(qnx6File->write(signedFile->read(qMin(BUFFER_LEN, s)))));
                    qnx6File->close();
                }
            }
        }
    }
}
