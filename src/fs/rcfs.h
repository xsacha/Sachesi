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

#include "fs.h"

namespace FS {

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

class RCFS : public QFileSystem
{
    Q_OBJECT

public:
    explicit RCFS(QString filename, QIODevice* file, qint64 offset, qint64 size, QString path)
        : QFileSystem(filename, file, offset, size, path, ".rcfs") {}

    rinode createNode(int offset);
    QString generateName(QString imageExt = "");
    QByteArray extractFile(qint64 node_offset, int node_size, int node_mode);
    void extractDir(int offset, int numNodes, QString basedir, qint64 startPos);

    bool createContents();

};

}
