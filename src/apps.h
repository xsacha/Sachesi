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
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
#include <QQmlListProperty>
#else
#include <QDeclarativeListProperty>
#define QQmlListProperty QDeclarativeListProperty
#endif

class Apps : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString packageId READ packageId WRITE setPackageId NOTIFY packageIdChanged)
    Q_PROPERTY(QString friendlyName READ friendlyName WRITE setFriendlyName NOTIFY friendlyNameChanged)
    Q_PROPERTY(int code READ code WRITE setCode NOTIFY codeChanged)
    Q_PROPERTY(int size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(bool isMarked READ isMarked WRITE setIsMarked NOTIFY isMarkedChanged)
    Q_PROPERTY(bool isAvailable READ isAvailable WRITE setIsAvailable NOTIFY isAvailableChanged)
    Q_PROPERTY(bool isInstalled READ isInstalled WRITE setIsInstalled NOTIFY isInstalledChanged)
    Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QString installedVersion READ installedVersion WRITE setInstalledVersion NOTIFY installedVersionChanged)
    Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY versionChanged)
    Q_PROPERTY(QString versionId READ versionId WRITE setVersionId NOTIFY versionIdChanged)
    Q_PROPERTY(QString checksum READ checksum WRITE setChecksum NOTIFY checksumChanged)

public:
    Apps(QObject *parent = 0);
    Apps(const Apps* app, QObject *parent = 0);

    QString name() const;
    QString url() const;
    QString packageId() const;
    QString friendlyName() const;
    int code() const;
    int size() const;
    bool isMarked() const;
    bool isAvailable() const;
    bool isInstalled() const;
    QString type() const;
    QString installedVersion() const;
    QString version() const;
    QString versionId() const;
    QString checksum() const;
    void setName(const QString &str);
    void setUrl(const QString &str);
    void setPackageId(const QString &str);
    void setFriendlyName(const QString &str);
    void setCode(const int &num);
    void setSize(const int &num);
    void setIsMarked(const bool &marked);
    void setIsAvailable(const bool &available);
    void setIsInstalled(const bool &installed);
    void setType(const QString &str);
    void setInstalledVersion(const QString &str);
    void setVersion(const QString &str);
    void setVersionId(const QString &str);
    void setChecksum(const QString &str);

signals:
    void nameChanged();
    void urlChanged();
    void packageIdChanged();
    void friendlyNameChanged();
    void codeChanged();
    void sizeChanged();
    void isMarkedChanged();
    void isAvailableChanged();
    void isInstalledChanged();
    void typeChanged();
    void installedVersionChanged();
    void versionChanged();
    void versionIdChanged();
    void checksumChanged();

private:
    QString _name;
    QString _url;
    QString _friendlyName;
    QString _packageId;
    int _code;
    int _size;
    bool _isMarked;
    bool _isAvailable;
    bool _isInstalled;
    QString _type;
    QString _installedVersion;
    QString _version;
    QString _versionId;
    QString _checksum;
};
