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

QPair<int, QString> QNX6::nodeInfo(QNXStream* stream, qint64 offset) {
    _file->seek(offset);
    QPair<int, QString> ret = {0, ""};
    ret.first = stream->grabInt();
    if (ret.first == 0)
        return ret;
    int count = stream->grabUChar();
    if (count == 0xFF)
    {
        _file->seek(offset + 0x8);
        int item = stream->grabInt();
        if (item > lfn.count())
            item = 1;
        _file->seek(findSector(lfn.at(item)));
        count = stream->grabUShort();
    }
    ret.second = _file->read(count);
    return ret;
}

// Specialised function which takes a directory and finds META-INF/MANIFEST.MF and grabs its data
void QNX6::extractManifest(int nodenum) {
    QNXStream stream(_file);
    foreach(int num, createNode(nodenum).sectors)
    {
        for (int i = 0; i < 0x1000; i += 0x20)
        {
            QPair<int, QString> info = nodeInfo(&stream, findSector(num) + i);
            if (info.second == "META-INF") {
                foreach(int num, createNode(info.first).sectors)
                {
                    for (int i = 0; i < 0x1000; i += 0x20) {
                        info = nodeInfo(&stream, findSector(num) + i);
                        if (info.second == "MANIFEST.MF") {
                            qinode ind = createNode(info.first);
                            QList<int> sections;
                            if (ind.tiers == 0 && (ind.sectors[0] > 0)) {
                                foreach(int sector, ind.sectors) {
                                    if (sector != -1)
                                        sections.append(sector);
                                }
                            } else if (ind.tiers > 0 ) {
                                foreach (int sector, ind.sectors) {
                                    if (sector == -1) break;
                                    _file->seek(findSector(sector));
                                    for (int j = 0; j < 0x400; j++)
                                    {
                                        stream >> sector;
                                        if (sector > 0)
                                            sections.append(sector);
                                    }
                                }
                                if (ind.tiers == 2) {
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
                            QByteArray manifestDump;
                            if (ind.size != 0) {
                                foreach(int section, sections)
                                {
                                    _file->seek(findSector(section));
                                    int len = sectorSize;
                                    if (section == sections.last() && (ind.size % sectorSize))
                                        len = ind.size % sectorSize;
                                    manifestDump.append(_file->read(len));
                                }
                            }

                            QString name = "";
                            QString version = "";
                            QString arch = "";
                            QString strSigned = "";
                            foreach(QByteArray manifestString, manifestDump.split('\n')) {
                                QString tmp = QString(manifestString).simplified();
                                if (tmp.startsWith("Package-Name:")) {
                                    name = tmp.split(": ").last().split('.').last();
                                }
                                else if (tmp.startsWith("Package-Version:")) {
                                    version = tmp.split(": ").last();
                                }
                                else if (tmp.startsWith("Package-Architecture:")) {
                                    arch = tmp.split(": ").last();
                                }
                                else if (tmp.startsWith("Package-Author-Certificate-Hash:")) {
                                    strSigned = "+signed";
                                }
                                else if (tmp.startsWith("Archive-Asset-Name:")) {
                                    manifestApps.append(tmp.split(": ").last());
                                }
                            }
                            if (currentZip == nullptr && name != "") {
                                currentZip = new QuaZip(QString("%1/%2-%3-nto+%4%5.bar").arg(_path).arg(name).arg(version).arg(arch).arg(strSigned));
                                emit currentNameChanged(QString("%1-%2").arg(name).arg(version));
                                currentZip->open(QuaZip::mdCreate);
                            }
                            // Leave now
                            return;
                        }
                    }
                }
                return;
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
    qinode ind = createNode(nodenum);
    foreach(int num, ind.sectors)
    {
        for (int i = 0; i < 0x80; i++)
        {
            // TODO: Nice place to check if we want to quit
            QPair<int, QString> info = nodeInfo(&stream, findSector(num) + (i * 0x20));
            if (info.second == "." || info.second == "..")
                continue;

            qinode ind2 = createNode(info.first);

            if (ind2.perms & QCFM_IS_DIRECTORY)
            {
                if (extractApps) {
                    if (info.second == "apps" && tier == 0) {
                        mainDir.mkdir(basedir);
                        extractDir(info.first, basedir, 1);
                        continue;
                    }
                    else if (tier == 1) {
                        currentZip = nullptr;
                        extractManifest(info.first);
                        if (currentZip != nullptr) {
                            extractDir(info.first, "", 2);
                            currentZip->close();
                            manifestApps.clear();
                            delete currentZip;
                        }
                        continue;
                    } else if (tier == 2) {
                        extractDir(info.first, info.second, 3);
                        continue;
                    }
                }
                extractDir(info.first, basedir + "/" + info.second, tier ? tier + 1 : 0);
                continue;
            }
            if (extractApps) {
                if (tier <= 1)
                    continue;
                QString thisFile = (tier == 2) ? info.second : (basedir + "/" + info.second);
                if (!manifestApps.isEmpty() && !(tier == 3 && basedir == "META-INF") && !manifestApps.contains(thisFile))
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
                            if (fn > 0)
                                sections.append(fn);
                        }
                    }
                }
            }

            QuaZipFile* zipFile = 0;
            QFile* newFile = 0;
            if (extractApps)
            {
                Q_ASSERT(currentZip != nullptr);
                zipFile = new QuaZipFile(currentZip);
                QuaZipNewInfo newInfo((tier == 2) ? info.second : (basedir + "/" + info.second));
                newInfo.setPermissions(QFileDevice::Permission(0x7774));
                newInfo.dateTime.setTime_t(ind.time);
                zipFile->open(QIODevice::WriteOnly, newInfo);
            } else {
                newFile = new QFile(basedir + "/" + info.second);
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
                        zipFile->write(tmp);
                    else
                        newFile->write(tmp);
                }
            }
            if (extractApps) {
                Q_ASSERT(zipFile->isOpen());
                zipFile->close();
                delete zipFile;
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

bool QNX6::createContents() {
    _file->seek(_offset+8);
    QNXStream stream(_file);
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
    emit currentNameChanged("");
    QDesktopServices::openUrl(QUrl(_path));
    return true;
}

}

