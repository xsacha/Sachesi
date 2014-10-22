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
#include <QObject>
#include <QList>
#include <QFile>
#include <QFileInfo>

class AutoloaderWriter: public QFile {
    Q_OBJECT
public:
    AutoloaderWriter(QList<QFileInfo> selectedInfo)
        : _infos(selectedInfo)
    {
        qSort(selectedInfo.begin(), selectedInfo.end(), compareSizes);
    }
    static bool compareSizes(QFileInfo i, QFileInfo j)
    {
        return i.size() > j.size();
    }
    void create(QString name) {
        // Find potential file
        QString append = ".exe";
        for (int f = 2; QFile::exists(name + append); f++) {
            append = QString("-%1.exe").arg(QString::number(f));
        }
        // Start the autoloader as a cap file
        QFile::copy(capPath(), name + append);
        setFileName(name + append);
        open(QIODevice::WriteOnly | QIODevice::Append);
        // This code is used as a separator
        write(QByteArray::fromBase64("at9dFE5LT0dJSE5JTk1TDRAMBRceERhTLUY8T0crSzk5OVNOT1FNT09RTU9RSEhwnNXFl5zVxZec1cWX").constData(), 60);
        // This is a placeholder for a password
        write(QByteArray(80, 0), 80);

        QByteArray dataHeader;
        QNXStream dataStream(&dataHeader, QIODevice::WriteOnly);
        dataStream << (quint64)_infos.count();
        quint64 counter = pos() + 64;
        foreach (QFileInfo info, _infos)
        {
            dataStream << counter;
            counter += info.size();
        }
        for (int i = _infos.count() - 1; i < 6; i++)
            dataStream << (qint64)0;
        write(dataHeader);
        _read = 100 * pos();
        _maxSize = counter;
        foreach (QFileInfo file, _infos)
            appendFile(file.filePath());
        close();
    }

    void appendFile(QString fileName) {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        while (!file.atEnd())
        {
            QByteArray tmp = file.read(BUFFER_LEN);
            if (tmp.size() < 0)
                break;
            _read += 100 * write(tmp);
            emit newProgress((int)(_read / _maxSize));
        }
        file.close();
    }
signals:
    void newProgress(int percent);
private:
    qint64 _read, _maxSize;
    QList<QFileInfo> _infos;
};
