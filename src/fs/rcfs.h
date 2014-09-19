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
    explicit RCFS(QString filename)
        : QFileSystem(filename) {}

    rinode createNode(int offset);
    QString generateName(QString imageExt = "");
    QByteArray extractFile(qint64 node_offset, int node_size, int node_mode);
    void extractDir(int offset, int numNodes, QString basedir, qint64 startPos);

    bool createContents();

};

}
