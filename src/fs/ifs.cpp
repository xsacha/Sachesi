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
    QString builder = QString(_file->readLine(16)); // ec_agent, developer
    if (builder == "ec_agent")
        builder = "prod";
    else if (builder == "developer")
        builder = "trunk";

    _file->seek(_offset + 0x50);
    QString build_date = QString(_file->readLine(16)); // Mmm dd yyyy

    QString name = QString("boot-%1-%2")
            .arg(builder)
            .arg(build_date.replace(' ', '.'));
    if (imageExt.isEmpty())
        return uniqueDir(name);

    return uniqueFile(name  + imageExt);
}

void IFS::extractDir(int offset, int numNodes, QString basedir, qint64 startPos)
{

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
    _file->seek(_offset + 0x1020);
    qint32 boot_size, startup_size;
    QNXStream stream(_file);
    // boot @ 0 with boot_size;
    stream >> boot_size;
    boot_size &= 0xffffff;
    unsigned char ifsSig[] = {0xEB, 0x7E, 0xFF, 0x00};

    _file->seek(_offset + boot_size);
    // Make sure there is a startup header
    if (_file->read(4) != QByteArray((char*)ifsSig, 4)) {
        return false; // Not a valid IFS image
    }
    _file->seek(_offset + boot_size + 0x20);
    // startup @ boot_size + 0x100 with startup_size - 0x100
    stream >> startup_size;
    // imagefs @ boot_size + startup_size
    //extractBootDir(0xC, 1, _path, _offset + boot_size + startup_size);


    // Temporarily dump the components until a full extraction is available
    QDir(_path).mkpath(".");
    // -- Dump boot.bin --
    QFileSystem::writeFile("boot.bin", _offset, boot_size);
    // -- Dump startup.bin
    QFileSystem::writeFile("startup.bin", _offset + boot_size + 0x100, startup_size - 0x100);
    // -- Dump imagefs.bin
    QFileSystem::writeFile("imagefs.bin", _offset + boot_size + startup_size, maxSize - boot_size - startup_size);

    // Display result
    QDesktopServices::openUrl(QUrl(_path));
    return true;
}
}
