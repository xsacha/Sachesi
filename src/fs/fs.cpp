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

#include "fs.h"

#ifdef _WIN32
void fixFileTime(QString filename, int time) {
    FILETIME pft;
    LONGLONG ll = Int32x32To64(time, 10000000) + 116444736000000000;
    pft.dwLowDateTime = (DWORD)ll;
    pft.dwHighDateTime = ll >> 32;
    HANDLE fd_handle = CreateFile(filename.toStdWString().c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    SetFileTime(fd_handle, &pft,(LPFILETIME) nullptr, &pft);
    CloseHandle(fd_handle);
}
#endif

QFileSystem::QFileSystem(QString filename, QIODevice* file, qint64 offset, qint64 size, QString path, QString imageExt)
    : QObject(nullptr)
    , _file(file)
    , _offset(offset)
    , _size(size)
    , _path(path)
    , _filename(filename)
    , _imageExt(imageExt)
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
QString QFileSystem::generateName(QString imageExt) {
    QString name = QFileInfo(_filename).completeBaseName();
    if (imageExt.isEmpty())
        return uniqueDir(name);

    return uniqueFile(name + imageExt);
}

// Entry for requesting an extration of the filesystem image
bool QFileSystem::extractImage() {
    curSize = 0;
    maxSize = _size;
    return this->createImage(this->generateName(_imageExt));
}

// A generic method for extracting an entire image of maxSize
bool QFileSystem::createImage(QString name) {
    return writeFile(name, _offset, maxSize);
}

// Entry for requesting an extration of the filesystem contents
bool QFileSystem::extractContents() {
    curSize = 0;
    maxSize = _size;
    _path += "/" + this->generateName();
    return this->createContents();
}

// A method to write writeSize bytes from a QIODevice to a new file, named filename
bool QFileSystem::writeFile(QString fileName, qint64 offset, qint64 writeSize, bool absolute) {
    _file->seek(offset);
    QFile newFile;
    if (absolute)
        newFile.setFileName(fileName);
    else
        newFile.setFileName(_path + "/" + fileName);
    if (!newFile.open(QIODevice::WriteOnly))
        return false;
    qint64 endSize = curSize + writeSize;
    while (endSize > curSize) {
        int diff = newFile.write(_file->read(qMin(BUFFER_LEN, endSize - curSize)));
        if (diff <= 0)
            return false;
        increaseCurSize(diff);
    }
    newFile.close();

    return true;
}
