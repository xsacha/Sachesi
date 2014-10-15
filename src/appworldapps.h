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

// Should probably just derive from Apps
class AppWorldApps : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString packageId READ packageId WRITE setPackageId NOTIFY packageIdChanged)
    Q_PROPERTY(QString friendlyName READ friendlyName WRITE setFriendlyName NOTIFY friendlyNameChanged)
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString fileId READ fileId WRITE setFileId NOTIFY fileIdChanged)
    Q_PROPERTY(int size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(QString image READ image WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY(bool isMarked READ isMarked WRITE setIsMarked NOTIFY isMarkedChanged)
    Q_PROPERTY(bool isAvailable READ isAvailable WRITE setIsAvailable NOTIFY isAvailableChanged)
    Q_PROPERTY(bool isInstalled READ isInstalled WRITE setIsInstalled NOTIFY isInstalledChanged)
    Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY versionChanged)
    Q_PROPERTY(QString versionId READ versionId WRITE setVersionId NOTIFY versionIdChanged)
    Q_PROPERTY(QString checksum READ checksum WRITE setChecksum NOTIFY checksumChanged)
    Q_PROPERTY(QString vendor READ vendor WRITE setVendor NOTIFY vendorChanged)
    Q_PROPERTY(QString vendorId READ vendorId WRITE setVendorId NOTIFY vendorIdChanged)
    Q_PROPERTY(QString price READ price WRITE setPrice NOTIFY priceChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QStringList screenshots READ screenshots NOTIFY screenshotsChanged)

public:
    AppWorldApps(QObject *parent = 0);
    AppWorldApps(const AppWorldApps* app, QObject *parent = 0);

    QString name() const;
    QString packageId() const;
    QString friendlyName() const;
    QString id() const;
    QString fileId() const;
    int size() const;
    QString image() const;
    bool isMarked() const;
    bool isAvailable() const;
    bool isInstalled() const;
    QString type() const;
    QString version() const;
    QString versionId() const;
    QString checksum() const;
    QString vendor() const;
    QString vendorId() const;
    QString price() const;
    QString description() const;
    QStringList screenshots() const {
        return _screenshots;
    }
    void setName(const QString &str);
    void setPackageId(const QString &str);
    void setFriendlyName(const QString &str);
    void setId(const QString &id);
    void setFileId(const QString &fileId);
    void setSize(const int &num);
    void setImage(const QString &image);
    void setIsMarked(const bool &marked);
    void setIsAvailable(const bool &available);
    void setIsInstalled(const bool &installed);
    void setType(const QString &str);
    void setVersion(const QString &str);
    void setVersionId(const QString &str);
    void setChecksum(const QString &str);
    void setVendor(const QString &str);
    void setVendorId(const QString &str);
    void setPrice(const QString &str);
    void setDescription(const QString &str);
    QStringList _screenshots;

signals:
    void nameChanged();
    void packageIdChanged();
    void friendlyNameChanged();
    void idChanged();
    void fileIdChanged();
    void sizeChanged();
    void imageChanged();
    void isMarkedChanged();
    void isAvailableChanged();
    void isInstalledChanged();
    void typeChanged();
    void versionChanged();
    void versionIdChanged();
    void checksumChanged();
    void vendorChanged();
    void vendorIdChanged();
    void priceChanged();
    void descriptionChanged();
    void screenshotsChanged();

private:
    QString _name;
    QString _friendlyName;
    QString _packageId;
    QString _id;
    QString _fileId;
    int _size;
    QString _image;
    bool _isMarked;
    bool _isAvailable;
    bool _isInstalled;
    QString _type;
    QString _version;
    QString _versionId;
    QString _checksum;
    QString _vendor;
    QString _vendorId;
    QString _price;
    QString _description;
};

void appendAppWorldApps(QQmlListProperty<AppWorldApps> * property, AppWorldApps * app);
int appWorldAppsSize(QQmlListProperty<AppWorldApps> * property);
int appWorldAppsSize(QQmlListProperty<AppWorldApps> * property);
AppWorldApps* appWorldAppsAt(QQmlListProperty<AppWorldApps> * property, int index);
void clearAppWorldApps(QQmlListProperty<AppWorldApps> *property);
