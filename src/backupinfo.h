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

#include <QString>
#include <QStringList>
#include <QObject>
#include <QXmlStreamAttributes>

#include "apps.h"

class BackupCategory {

public:
    BackupCategory(QXmlStreamAttributes cat) {
        foreach(QXmlStreamAttribute attr, cat)
        {
            QString att = attr.name().toString();
            QString val = attr.value().toString();
            if (att == "id")
                id = val;
            else if (att == "name")
                name = val;
            else if (att == "count")
                count = val;
            else if (att == "bytesize")
                bytesize = val;
            else if (att == "perimetertype")
                perimetertype = (val != "personal") ? "1" : "0";
        }
    }
    BackupCategory(QString idGiven, QString nameGiven) {
        id = idGiven;
        name = nameGiven;
        count = "0";
        bytesize = "-1";
        perimetertype = "0";
    }

    ~BackupCategory() {}

    QString id;
    QString name;
    QString count;
    QString bytesize;
    QString perimetertype;
};

class BackupInfo : public QObject {
    Q_OBJECT

public:
    BackupInfo();
    ~BackupInfo() {}
    QString modeString();
    QString stringFromMode(int mode);
    int mode() const;
    int numMethods() const;
    void addMode(QXmlStreamAttributes cat);
    void addApp(QXmlStreamAttributes cat);
    void sortApps();
    void clearModes();
    void setMode(const int &val);
    QString curMode() const;
    void setCurMode(int increment);

    qint64 size() const;
    void setSize(const qint64 &val);
    qint64 maxSize() const;
    void setMaxSize(const qint64 &val);

    qint64 curSize() const;
    void setCurSize(const qint64 &val);
    qint64 curMaxSize() const;
    void setCurMaxSize(const qint64 &val);
    void setCurMaxSize(int index, const qint64 &val);
    int progress() const;
    void setProgress(const int &progress);

    QList<BackupCategory*> categories;
    QList<Apps*> apps;

    int rev() const;
signals:
    void modeChanged();
    void progressChanged();
    void curModeChanged();
    void sizeChanged();
    void maxSizeChanged();
    void curSizeChanged();
    void curMaxSizeChanged();
    void numMethodsChanged();
private:
    int _progress;
    qint64 _size, _maxSize;
    QList<qint64> _curSize, _curMaxSize;
    int _mode;
    int _curMode;
    int _numMethods;
    int _rev;
};
