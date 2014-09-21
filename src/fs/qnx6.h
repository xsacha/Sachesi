#pragma once

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

private:
    quint16 sectorSize;
    quint16 sectorOffset;
    QList<int> lfn;
    QuaZip* currentZip;
    QList<QString> manifestApps;

};

}
