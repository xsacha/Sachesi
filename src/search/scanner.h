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
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQmlListProperty>
#include "discoveredrelease.h"

class Scanner : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool finishedScan READ finishedScan NOTIFY signalFinished)
    Q_PROPERTY(bool isAuto READ isAuto WRITE setIsAuto NOTIFY isAutoChanged)
    Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged)
    Q_PROPERTY(int findExisting READ findExisting WRITE setFindExisting NOTIFY findExistingChanged)
    Q_PROPERTY(DiscoveredRelease* curRelease READ curRelease NOTIFY curReleaseChanged)

    Q_PROPERTY(QQmlListProperty<DiscoveredRelease> history READ history NOTIFY historyChanged)
public:
    Scanner()
    : QObject()
    , _finishedScan(false)
    , _isAuto(false)
    , _isActive(false)
    , _findExisting(0)
    , _scansActive(0)
    , _curRelease(NULL)
    {
        _manager = new QNetworkAccessManager();
    }
    virtual ~Scanner() {}
    bool isAuto() const { return _isAuto; }
    bool isActive() const { return _isActive; }
    int findExisting() const { return _findExisting; }
    QString softwareRelease() const { return _softwareRelease; }
    DiscoveredRelease* curRelease() { return _curRelease; }
    QQmlListProperty<DiscoveredRelease> history() {
        return QQmlListProperty<DiscoveredRelease>(this, _history);
    }
    void completeScan() {
        _scansActive--;
        if (_scansActive < 0)
            _scansActive = 0;
        if (_scansActive == 0) {
            // Did we get a version that either exists or we didn't care?
            if (_curRelease->srVersion().startsWith('1')) {
                if (_findExisting != 2 && (_findExisting != 1 || _curRelease->baseUrl() != "")) {
                    setIsAuto(false);
                }
                _history.prepend(_curRelease);
                emit historyChanged();
            } else {
                _curRelease->setSrVersion(tr("No Release"));
            }
            setIsActive(false);
            finishedFunc();
        }
        emit curReleaseChanged();
    }
    // Because Cascades is stupid
    void finishedFunc() {
        emit signalFinished();
        _finishedScan = true; emit signalFinished();
        _finishedScan = false;
    }
    void setIsAuto(bool isAuto) { _isAuto = isAuto; emit isAutoChanged(); }
    void setIsActive(bool isActive) { _isActive = isActive; emit isActiveChanged(); }
    void setFindExisting(int findExisting) { _findExisting = findExisting; emit findExistingChanged(); }

    Q_INVOKABLE void clearHistory();
    Q_INVOKABLE void reverseLookup(QString OSver);
    Q_INVOKABLE void generatePotentialLinks();

    bool finishedScan() const { return _finishedScan; }

private slots:
    void newSRVersion();
    void validateDownload();
    void serverError(QNetworkReply::NetworkError err);

Q_SIGNALS:
    void signalFinished();
    void isAutoChanged();
    void isActiveChanged();
    void findExistingChanged();
    void softwareReleaseChanged();
    void curReleaseChanged();
    void historyChanged();

private:
    bool _finishedScan;
    bool _isAuto, _isActive;
    int _findExisting;
    int _scansActive;
    QString _softwareRelease;
    DiscoveredRelease* _curRelease;
    QList<DiscoveredRelease*> _history;
    QNetworkAccessManager* _manager;

    void appendNewLink(QString *potentialText, QString linkType, QString hwType, QString version);
};
