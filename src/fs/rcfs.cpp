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

#include "rcfs.h"

#ifdef _LZO2_SHARED
#include <lzo/lzo1x.h>
#else
#include "lzo.h"
#endif

namespace FS {

rinode RCFS::createNode(int offset) {
    QNXStream stream(_file);
    _file->seek(_offset + offset + 4);
    rinode ind;
    stream >> ind.mode >> ind.nameoffset >> ind.offset >> ind.size >> ind.time;
    _file->seek(_offset + ind.nameoffset);
    ind.name = QString(_file->readLine(QNX6_MAX_CHARS));
    if (ind.name == "")
        ind.name = ".";
    return ind;
}

QString RCFS::generateName(QString imageExt) {
    _file->seek(_offset + 8);
    QString board = "rcfs";
    QString variant = "unk";
    QString cpu = "unk";
    QString version = "unk";

    if (_file->readLine(4).startsWith("fs-")) {
        _file->seek(_offset + 0x1038);
        QNXStream stream(_file);
        READ_TMP(qint32, offset);
        rinode dotnode = createNode(offset);
        for (int i = 0; i < dotnode.size / 0x20; i++) {
            rinode slashdotnode = createNode(dotnode.offset + (i * 0x20));
            if (slashdotnode.name == "etc") {
                for (int i = 0; i < slashdotnode.size / 0x20; i++) {
                    rinode node = createNode(slashdotnode.offset + (i * 0x20));
                    if (node.name == "os.version" || node.name == "radio.version") {
                        QByteArray versionData = extractFile(_offset + node.offset, node.size, node.mode);
                        version = QString(versionData).simplified();
                    }
                }
            }
            if (slashdotnode.name.endsWith(".tdf")) {
                QByteArray boardData = extractFile(_offset + slashdotnode.offset, slashdotnode.size, slashdotnode.mode);
                foreach (QString config, QString(boardData).split('\n')) {
                    if (config.startsWith("CPU=")) {
                        cpu = config.split('=').last().remove('"');
                    } else if (config.startsWith("BOARD=")) {
                        board = config.split('=').last().remove('"');
                        if (board != "radio")
                            board.prepend("os.");
                    } else if (config.startsWith("BOARD_CONFIG=") ||
                               config.startsWith("RADIO_BOARD_CONFIG=")) {
                        variant = config.split('=').last().remove('"');
                    }
                }
            }
        }
    }

    QString name = QString("%1.%2.%3.%4")
            .arg(board)
            .arg(variant)
            .arg(version)
            .arg(cpu);
    if (imageExt.isEmpty())
        return uniqueDir(name);

    return uniqueFile(name + imageExt);
}


// Returning the array of data might be dangerous if it's huge. Consider taking a var instead
QByteArray RCFS::extractFile(qint64 node_offset, int node_size, int node_mode)
{
    QByteArray ret;
    _file->seek(node_offset);
    QNXStream stream(_file);
    if (node_mode & QCFM_IS_LZO_COMPRESSED) {
        READ_TMP(int, next);
        int chunks = (next - 4) / 4;
        QList<int> sizes, offsets;
        offsets.append(next);
        for (int s = 0; s < chunks; s++) {
            stream >> next;
            offsets.append(next);
            sizes.append(offsets[s+1] - offsets[s]);
        }
        char* buffer = new char[node_size];
        foreach(int size, sizes) {
            char* readData = new char[size];
            _file->read(readData, size);
            size_t write_len = 0x4000;
            lzo1x_decompress_safe(reinterpret_cast<const unsigned char*>(readData), size, reinterpret_cast<unsigned char*>(buffer), &write_len, nullptr);
            ret.append(buffer, write_len);
            delete [] readData;
        }
        delete [] buffer;
    }
    else {
        for (qint64 i = node_size; i > 0;) {
            QByteArray data = _file->read(qMin(BUFFER_LEN, i));
            i -= data.size();
            ret.append(data);
        }
    }
    return ret;
}

void RCFS::extractDir(int offset, int numNodes, QString basedir, qint64 _offset)
{
    QNXStream stream(_file);
    QDir mainDir(basedir);
    for (int i = 0; i < numNodes; i++) {
        rinode node = createNode(offset + (i * 0x20));
        node.path_to = basedir;
        QString absName = node.path_to + "/" + node.name;
        if (node.mode & QCFM_IS_DIRECTORY) {
            mainDir.mkpath(node.name);
            if (node.size > 0)
                extractDir(node.offset, node.size / 0x20, absName, _offset);
        }
        else {
            _file->seek(node.offset + _offset);
            if (node.mode & QCFM_IS_SYMLINK) {
#ifdef _WIN32
                QString lnkName = absName + ".lnk";
                QFile::link(node.path_to + "/" + _file->readLine(QNX6_MAX_CHARS), lnkName);
                fixFileTime(lnkName, node.time);
#else
                QFile::link(node.path_to + "/" + _file->readLine(QNX6_MAX_CHARS), absName);
#endif
                continue;
            }
            if (node.mode & QCFM_IS_LZO_COMPRESSED) {
                QFile newFile(absName);
                newFile.open(QFile::WriteOnly);
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
                    _file->read(readData, size);
                    size_t write_len = 0x4000;
                    lzo1x_decompress_safe(reinterpret_cast<const unsigned char*>(readData), size, reinterpret_cast<unsigned char*>(buffer), &write_len, nullptr);
                    newFile.write(buffer, (qint64)write_len);
                    increaseCurSize(size); // Uncompressed size
                    delete [] readData;
                }
                delete [] buffer;
                newFile.close();
            } else {
                writeFile(absName, node.size, true);
            }
#ifdef _WIN32
            fixFileTime(absName, node.time);
#endif
        }
    }
}

bool RCFS::createContents() {
    _file->seek(_offset + 0x1038);
    QNXStream stream(_file);
    READ_TMP(qint32, offset);
    extractDir(offset, 1, _path, _offset);

    // Display result
    QDesktopServices::openUrl(QUrl(_path));
    return true;
}
}
