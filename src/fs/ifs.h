#pragma once

#include "fs.h"

namespace FS {

struct binode {
    int mode;
    QString name;
    int offset;
    // TODO: Sizes greater than 16-bit?
    qint16 size;
    int time;
    QString path_to;
    int chunks;
};

class IFS : public QFileSystem
{
    Q_OBJECT

public:
    explicit IFS(QString filename, QIODevice* file, qint64 offset, qint64 size, QString path)
        : QFileSystem(filename, file, offset, size, path) {}

    explicit IFS(QString filename)
        : QFileSystem(filename) {}

    binode createBNode(int offset, qint64 startPos);
    QString generateName(QString imageExt = "");
    void extractDir(int offset, int numNodes, QString basedir, qint64 startPos);
    bool extractImage() {
        return QFileSystem::extractImage(".ifs");
    }
    bool extractContents();

};
}
