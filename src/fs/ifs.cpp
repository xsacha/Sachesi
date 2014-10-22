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

#include "ifs.h"

namespace FS {

binode IFS::createBNode(int offset, qint64 startPos) {
    QNXStream stream(_file);
    _file->seek(startPos + offset);
    binode ind;
    stream >> ind.mode >> ind.size >> ind.time;

    ind.name = QString(_file->readLine(QNX6_MAX_CHARS));
    if (ind.name == "")
        ind.name = ".";
    ind.offset = _file->pos() - startPos;

    return ind;
}

QString IFS::generateName(QString imageExt) {
    _file->seek(_offset + 0x40);
    QString builder = QString(_file->readLine(16)); // ec_agent, developer or username
    if (builder == "ec_agent")
        builder = "prod";
    else if (builder == "developer")
        builder = "trunk";

    _file->seek(_offset + 0x50);
    QStringList buildDateList = QString(_file->readLine(16)).split(' '); // Mmm dd yyyy
    buildDateList.swap(0,1); // Swap day and month
    QString buildDate = buildDateList.join("");

    _file->seek(_offset + 0xAC);
    QNXStream stream(_file);
    READ_TMP(short, build);
    READ_TMP(quint8, majorminor);
    READ_TMP(quint8, os);
    QString ifs_ver = QString("%1.%2.%3.%4")
            .arg(os)
            .arg(majorminor >> 3)
            .arg(majorminor & 0x7)
            .arg(build);

    QString name = QString("boot-%1-%2-%3")
            .arg(ifs_ver)
            .arg(builder)
            .arg(buildDate);
    if (imageExt.isEmpty())
        return uniqueDir(name);

    return uniqueFile(name  + imageExt);
}

void IFS::extractDir(int offset, int numNodes, QString basedir, qint64 startPos)
{

    Q_UNUSED(offset);
    Q_UNUSED(numNodes);
    Q_UNUSED(basedir);
    Q_UNUSED(startPos);
    // TODO: Nodes seem to fall apart. This must be whole-image compressed?
    /*
    QNXStream stream(_file);
    QDir mainDir(basedir);
    for (int i = 0; i < numNodes; i++) {
        binode node = createBNode(offset + (i * 0x20), startPos);
        qDebug() << QString::number(node.mode,16) << node.name << QString::number(node.offset,16);
        if (node.mode & QCFM_IS_DIRECTORY) {
            qDebug() << "Is directory";
            //extractRCFSDir(node.offset, node.size / 0x20, node.path_to + "/" + node.name, startPos);
        } else {
            if (node.mode & QCFM_IS_SYMLINK) {
                _file->seek(startPos + node.offset);
                qDebug() << "Symlink: " << node.name << " -> " << _file->readLine(QNX6_MAX_CHARS);
            } else if (node.mode & QCFM_IS_COMPRESSED) {
                qDebug() << "Is compressed file";
            } else {
                qDebug() << "Is regular file";
            }
        }
    }*/
}

bool IFS::createContents() {
    QNXStream stream(_file);
    _file->seek(_offset + 1);
    qint8 type;
    qint32 boot_size, startup_size;
    stream >> type;
    if (type == 3) { // Qualcomm
        _file->seek(_offset + 0x1020);
        // boot @ 0 with boot_size;
        stream >> boot_size;
        boot_size &= 0xfffff;
    }
    else { // 1 // OMAP
        // No boot.bin
        boot_size = 0x808;
    }

    _file->seek(_offset + boot_size);
    // Make sure there is a startup header
    if (_file->read(4) != QByteArray::fromHex("EB7EFF00")) {
        // It may be offset by 0x1000
        boot_size += 0x1000;
        _file->seek(_offset + boot_size);
        if (_file->read(4) != QByteArray::fromHex("EB7EFF00")) {
            return false; // Not a valid IFS image
        }
    }
    _file->seek(_offset + boot_size + 0x20);
    // startup @ boot_size + 0x100 with startup_size - 0x100
    stream >> startup_size;
    // imagefs @ boot_size + startup_size
    //extractBootDir(0xC, 1, _path, _offset + boot_size + startup_size);


    // Temporarily dump the components until a full extraction is available
    QDir(_path).mkpath(".");
    // -- Dump boot.bin --
    if (boot_size > 0x1100) // Does it have a boot.bin? Some speciality images don't and start at 0x1008
        QFileSystem::writeFile("boot.bin", _offset + 0x1100, boot_size - 0x1100);
    // -- Dump startup.bin
    QFileSystem::writeFile("startup.bin", _offset + boot_size + 0x100, startup_size - 0x100);
    // -- Dump imagefs.bin
    QFileSystem::writeFile("imagefs.bin", _offset + boot_size + startup_size, maxSize - boot_size - startup_size);

    // Display result
    QDesktopServices::openUrl(QUrl(_path));
    return true;
}
}
