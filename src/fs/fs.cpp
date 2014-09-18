#include "fs.h"

QFileSystem::QFileSystem(QString filename, QIODevice* file, qint64 offset, qint64 size, QString path)
    : QObject(nullptr)
    , _file(file)
    , _offset(offset)
    , _size(size)
    , _path(path)
    , _filename(filename)
{
    if (_file == nullptr) {
        _file = new QFile(filename);
        _file->open(QIODevice::ReadOnly);
        size = _file->size();
    }
}

// Free anything we made here
QFileSystem::~QFileSystem() {
    // TODO: Better tracking of whether we created this?
    // Assume offset of 0 means we decided to let QFileSystem make it
    if (_offset == 0) {
        if (_file->isOpen())
            _file->close();
        delete _file;
    }
}

// A method to append numbers to a filename/folder until it is unique
QString QFileSystem::uniqueDir(QString name) {
    int counter = 0;
    while (QDir(name + (counter++ > 0 ? QString::number(counter + 1) : "")).exists());

    return name + (counter > 1 ? QString::number(counter) : "");
}
QString QFileSystem::uniqueFile(QString name) {
    int counter = 0;
    while (QFile::exists(name + (counter++ > 0 ? QString::number(counter + 1) : "")));

    return name + (counter > 1 ? QString::number(counter) : "");
}

// A generic method of working out new name based on old name
QString QFileSystem::generateName(QString imageExt = "") {
    QString name = QFileInfo(_filename).completeBaseName();
    if (imageExt.isEmpty())
        return uniqueDir(name);

    return uniqueFile(name + imageExt);
}

// A generic method for extracting an entire image of maxSize
bool QFileSystem::extractImage(QString imageExt) {
    curSize = 0;
    maxSize = _size;
    _file->seek(_offset);
    return writeFile(generateName(imageExt), maxSize);
}

// A method to write writeSize bytes from a QIODevice to a new file, named filename
bool QFileSystem::writeFile(QString fileName, qint64 writeSize, bool absolute) {
    QFile newFile;
    if (absolute)
        newFile.setFileName(fileName);
    else
        newFile.setFileName(_path + "/" + fileName);
    if (!newFile.open(QIODevice::WriteOnly))
        return false;
    qint64 endSize = curSize + writeSize;
    while (endSize > curSize)
        increaseCurSize(newFile.write(_file->read(qMin(BUFFER_LEN, endSize - curSize))));
    newFile.close();

    return true;
}
