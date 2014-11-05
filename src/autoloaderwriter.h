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
#include <QApplication>
#include <QList>
#include <QFile>
#include "fs/fs.h" // QNXStream
#include "ports.h"

class AutoloaderWriter: public QFile {
    Q_OBJECT
public:
    AutoloaderWriter(QList<QIODevice*> devices)
        : _devHandle(devices)
        , _finished(false)
    {

    }

    void kill() {
        _finished = true;
        if (isOpen())
            close();
        remove();
    }

    void create(QString name) {
        _maxSize = 0x10000000; // A very large number :)
        // Find potential file
        QString append = ".exe";
        for (int f = 2; QFile::exists(name + append); f++) {
            append = QString("-%1.exe").arg(QString::number(f));
        }
        // Start the autoloader as a cap file
        setFileName(name + append);
        open(QIODevice::WriteOnly);
        QFile cap(capPath());
        cap.open(QIODevice::ReadOnly);
        appendFile(&cap);

        // This code is used as a separator
        write(QByteArray::fromBase64("at9dFE5LT0dJSE5JTk1TDRAMBRceERhTLUY8T0crSzk5OVNOT1FNT09RTU9RSEhwnNXFl5zVxZec1cWX").constData(), 60);
        // This is a placeholder for a password
        write(QByteArray(80, 0), 80);

        QByteArray dataHeader;
        QNXStream dataStream(&dataHeader, QIODevice::WriteOnly);
        dataStream << (quint64)_devHandle.count();
        quint64 counter = pos() + 64;
        foreach (QIODevice* file, _devHandle)
        {
            dataStream << counter;
            counter += file->size();
        }
        for (int i = _devHandle.count() - 1; i < 6; i++)
            dataStream << (qint64)0;
        write(dataHeader);
        _read = 100 * pos();
        _maxSize = counter;
        resize(_maxSize);
        foreach (QIODevice* file, _devHandle)
            if (!_finished) appendFile(file);
        if (isOpen())
            close();
    }

    void appendFile(QIODevice* file) {
        while (!file->atEnd())
        {
            // Check if we are being told to leave
            qApp->processEvents();
            if (_finished)
                return;
            int writeSize = write(file->read(FAST_BUFFER_LEN));
            if (writeSize < 0) {
                kill();
                return;
            }
            _read += 100 * writeSize;
            emit newProgress((int)(_read / _maxSize));
        }
        file->close();
    }
signals:
    void newProgress(int percent);
private:
    qint64 _read, _maxSize;
    QList<QIODevice*> _devHandle;
    bool _finished;
};
