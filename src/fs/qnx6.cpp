#include "qnx6.h"

namespace FS {

qint64 QNX6::findIndexFromSig(unsigned char* signature, int startFrom, int distanceFrom, unsigned int maxBlocks, int num) {
    if (startFrom != -1)
        _file->seek(startFrom);
    int readlen = BUFFER_LEN;
    while (!_file->atEnd() && (maxBlocks-- != 0))
    {
        QByteArray tmp = _file->read(readlen);
        if (tmp.size() < 0)
            break;
        for (int i = 0; i < tmp.size(); i++)
        {
            bool found = true;
            for (int j = 0; j < num; j++)
                if ((unsigned char)tmp[i+j] != signature[j]) {
                    found = false;
                    break;
                }
            if (found)
                return (qint64)(_file->pos() - tmp.size() + i + distanceFrom);
        }
    }
    return 0;
}

qinode QNX6::createNode(int node) {
    QNXStream stream(_file);
    qinode ind;
    qint64 base = findNode(node);
    _file->seek(base);
    stream >> ind.size;
    _file->seek(base + 0x10);
    stream >> ind.time;
    _file->seek(base + 0x20);
    stream >> ind.perms;
    stream.skipRawData(2);
    int sector;
    for (int i = 0; i < 16; i++)
    {
        stream >> sector;
        if (sector != -1)
            ind.sectors.append(sector);
    }
    // No sectors appears to be caused by empty files. These need to be extracted
    if (ind.sectors.count() == 0) {
        ind.sectors.append(0);
    }

    stream >> ind.tiers;
    return ind;
}

void QNX6::extractManifest(int nodenum) {
    QNXStream stream(_file);
    int count;
    qinode ind = createNode(nodenum);
    foreach(int num, ind.sectors)
    {
        for (int i = 0; i < 0x1000; i += 0x20)
        {
            // TODO: A good place ot check if we should exit

            // What a hack! This is really bad!
            //increaseCurSize(14);
            _file->seek(findSector(num) + i);
            READ_TMP(int, inodenum);
            if (inodenum == 0)
                break;

            READ_TMP(unsigned char, c_byte);
            count = c_byte;
            if (count == 0xFF)
            {
                _file->seek(findSector(num) + i + 0x8);
                READ_TMP(int, item);
                if (item > lfn.count())
                    item = 1;
                _file->seek(findSector(lfn.at(item)));
                READ_TMP(unsigned short, c_sint);
                count = c_sint;
            }
            QString dir = QString(_file->read(count));
            if (dir == "." || dir == "..")
                continue;
            qinode ind2 = createNode(inodenum);
            if (ind2.perms & QCFM_IS_DIRECTORY && dir == "META-INF") {
                extractDir(inodenum, dir, 3);
            }
        }
    }
}

void QNX6::extractDir(int nodenum, QString basedir, int tier)
{
    QDir mainDir;
    if (!extractApps)
        mainDir.mkdir(basedir);

    QNXStream stream(_file);
    int count;
    qinode ind = createNode(nodenum);
    foreach(int num, ind.sectors)
    {
        for (int i = 0; i < 0x80; i++)
        {
            // TODO: Nice place to check if we want to quit
            _file->seek(findSector(num) + (i * 0x20));
            READ_TMP(int, inodenum);
            if (inodenum == 0)
                break;

            READ_TMP(unsigned char, c_byte);
            count = c_byte;
            if (count == 0xFF)
            {
                _file->seek(findSector(num) + (i * 0x20) + 0x8);
                READ_TMP(int, item);
                if (item > lfn.count())
                    item = 1;
                _file->seek(findSector(lfn.at(item)));
                READ_TMP(unsigned short, c_sint);
                count = c_sint;
            }
            QString dir = QString(_file->read(count));
            if (dir == "." || dir == "..")
                continue;

            qinode ind2 = createNode(inodenum);

            if (ind2.perms & QCFM_IS_DIRECTORY)
            {
                if (extractApps) {
                    if (dir == "apps" && tier == 0) {
                        mainDir.mkdir(basedir);
                        extractDir(inodenum, basedir, 1);
                        continue;
                    }
                    else if (tier == 1) {
                        if (!dir.contains(".gY"))
                            continue;
                        dir = dir.split(".gY").first();
                        currentZip = new QuaZip(basedir + "/" + dir+".bar");
                        currentZip->open(QuaZip::mdCreate);
                        extractManifest(inodenum);
                        extractDir(inodenum, "", 2);
                        currentZip->close();
                        manifestApps.clear();
                        delete currentZip;
                        continue;
                    } else if (tier == 2 && dir != "META-INF") {
                        extractDir(inodenum, dir, 3);
                        continue;
                    }
                }
                extractDir(inodenum, basedir + "/" + dir, tier ? tier + 1 : 0);
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
                    _file->seek(findSector(sector));
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
                        _file->seek(findSector(fn));
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
                    _file->seek(findSector(section));
                    int len = sectorSize;
                    if (section == sections.last() && (ind2.size % sectorSize))
                        len = ind2.size % sectorSize;
                    QByteArray tmp = _file->read(len);
                    increaseCurSize(tmp.size());
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

bool QNX6::extractContents() {
    curSize = 0;
    maxSize = _size;
    _path += "/" + generateName();

    QNXStream stream(_file);
    _file->seek(_offset+8);
    READ_TMP(unsigned char, typeQNX); // 0x10 = no offset; 0x08 = has offset
    unsigned char qnx6Sig[] = {0x22, 0x11, 0x19, 0x68};
    unsigned char fsSig[] = {0xDD, 0xEE, 0xE6, 0x97};
    if ( (findIndexFromSig(qnx6Sig, -1, 0, 1)) == 0) { return false; }
    if ( (_offset = findIndexFromSig(fsSig, -1, 0)) == 0) { return false; }
    _file->seek(_offset+48);
    stream >> sectorSize;
    if (sectorSize % 512) { return false; }
    _offset += sectorSize;

    // Find sectorOffset
    qinode ind2 = createNode(1);
    // Try all offsets
    if (typeQNX == 0x10)
    {
        sectorOffset = 0;
        _file->seek((ind2.sectors[0] - sectorOffset)*sectorSize + _offset);
        READ_TMP(int, offsetCheck);
        if (offsetCheck != 1)
            typeQNX = 8;
    }
    if (typeQNX != 0x10) {
        for (sectorOffset = 0x6320; sectorOffset < 0x6400; sectorOffset += 0x10)
        {
            _file->seek((ind2.sectors[0] - sectorOffset)*sectorSize + _offset);
            READ_TMP(int, offsetCheck);
            if (offsetCheck == 1)
                break;
        }
    }

    for (qint64 s = _offset - 0xF10; true; s+=4)
    {
        _file->seek(s);
        READ_TMP(int, next);
        if (next == -1) break;
        _file->seek(findSector(next));
        for (int i = 0; i < 0x400; i++) {
            stream >> next;
            if (next > 0)
                lfn.append(next);
        }
    }
    extractDir(1, _path, 0);
    QDesktopServices::openUrl(QUrl(_path));
    return true;
}

}

