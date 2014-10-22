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

#include <QPair>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include "fs.h"

namespace FS {

struct qinode {
    int size;
    QList<int> sectors;
    quint8 tiers;
    int time;
    quint16 perms;
};

class QNX6 : public QFileSystem
{
    Q_OBJECT

public:
    explicit QNX6(QString filename, QIODevice* file, qint64 offset, qint64 size, QString path)
        : QFileSystem(filename, file, offset, size, path, ".qnx6")
        , currentZip(nullptr) {}

    inline qint64 findSector(qint64 sector) {
        return _offset + ((sector - sectorOffset) * sectorSize);
    }
    inline qint64 findNode(int node) {
        return _offset + (0x80 * (node - 1));
    }
    qint64 findIndexFromSig(unsigned char* signature, int startFrom, int distanceFrom, unsigned int maxBlocks = -1, int num = 4);
    qinode createNode(int node);
    // TODO: Read ./.rootfs.os.version or ./var/pps/system/installer/coreos/0
    //QString generateName(QString imageExt = "");
    void extractManifest(int nodenum);
    void extractDir(int offset, QString basedir, int numNodes);

    bool createContents();

    // TODO: These need to have a better method of passing from Splitter
    bool extractApps;

signals:
    void currentNameChanged(QString name);

private:
    QPair<int, QString> nodeInfo(QNXStream* stream, qint64 offset);
    quint16 sectorSize;
    quint16 sectorOffset;
    QList<int> lfn;
    QuaZip* currentZip;
    QList<QString> manifestApps;

};

}
