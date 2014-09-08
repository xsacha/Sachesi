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

#include <QtNetwork>
#include <QObject>
#include "splitter.h"

class MainNet : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString softwareRelease MEMBER _softwareRelease NOTIFY softwareReleaseChanged) // from reverse lookup
    Q_PROPERTY(QString versionRelease MEMBER _versionRelease NOTIFY versionChanged)
    Q_PROPERTY(QString versionOS MEMBER _versionOS NOTIFY versionChanged)
    Q_PROPERTY(QString versionRadio MEMBER _versionRadio NOTIFY versionChanged)
    Q_PROPERTY(QString variant MEMBER _variant NOTIFY variantChanged)
    Q_PROPERTY(QString description MEMBER _description NOTIFY descriptionChanged)
    Q_PROPERTY(QString updateUrl MEMBER _updateUrl NOTIFY updateUrlChanged)
    Q_PROPERTY(QString applications MEMBER _applications NOTIFY applicationsChanged)
    Q_PROPERTY(QString error MEMBER _error NOTIFY errorChanged)
    Q_PROPERTY(QString multiscanVersion MEMBER _multiscanVersion NOTIFY versionChanged)
    Q_PROPERTY(bool    advanced MEMBER _advanced WRITE setAdvanced NOTIFY advancedChanged)
    Q_PROPERTY(bool    downloading MEMBER _downloading WRITE setDownloading NOTIFY downloadingChanged)
    Q_PROPERTY(bool    hasPotentialLinks MEMBER _hasPotentialLinks NOTIFY hasPotentialLinksChanged)
    Q_PROPERTY(bool    hasBootAccess READ hasBootAccess CONSTANT)
    Q_PROPERTY(bool    multiscan MEMBER _multiscan WRITE setMultiscan NOTIFY multiscanChanged)
    Q_PROPERTY(int     scanning MEMBER _scanning WRITE setScanning NOTIFY scanningChanged)
    Q_PROPERTY(int     dlProgress MEMBER _dlProgress WRITE setDLProgress NOTIFY dlProgressChanged)
    Q_PROPERTY(int     currentId MEMBER _currentId NOTIFY currentIdChanged)
    Q_PROPERTY(int     maxId MEMBER _maxId NOTIFY maxIdChanged)
    Q_PROPERTY(int     splitting MEMBER _splitting NOTIFY splittingChanged)
    Q_PROPERTY(int     splitProgress MEMBER _splitProgress WRITE setSplitProgress NOTIFY splitProgressChanged)
    Q_PROPERTY(QString currentFile READ currentFile NOTIFY currentFileChanged)

public:
    MainNet(QObject* parent = 0);
    ~MainNet();
    Q_INVOKABLE void updateDetailRequest(QString delta, QString carrier, QString country, int device, int variant, int mode, int server/*, int version*/);
    Q_INVOKABLE void downloadLinks();
    Q_INVOKABLE void splitAutoloader(QUrl, int options);
    Q_INVOKABLE void combineAutoloader(QList<QUrl> selectedFiles);
    Q_INVOKABLE void extractImage(int type, int options);
    Q_INVOKABLE void grabLinks();
    Q_INVOKABLE void grabPotentialLinks(QString softwareRelease, QString osVersion);
    Q_INVOKABLE void abortDL(QNetworkReply::NetworkError error = (QNetworkReply::NetworkError)0);
    Q_INVOKABLE void abortSplit();
    Q_INVOKABLE void reverseLookup(QString carrier, QString country, int device, int variant, int server, QString OSver, bool skip);

    Q_INVOKABLE QString nameFromVariant(unsigned int device, unsigned int variant);
    Q_INVOKABLE QString hwidFromVariant(unsigned int device, unsigned int variant);
    Q_INVOKABLE unsigned int variantCount(unsigned int device);
    bool    hasBootAccess()  const { return
#ifdef BOOTLOADER_ACCESS
                true;
#else
                false;
#endif
                                   }
    void    setMultiscan(const bool &multiscan);
    void    setScanning(const int &scanning);
    void    setDLProgress(const int &progress);
    void    setAdvanced(const bool &advanced);
    void    setDownloading(const bool &downloading);
    QString currentFile() const;
public slots:
    void    setSplitProgress(const int &progress);
    void    capNetworkReply(QNetworkReply* reply);
    void    confirmNewSRSkip();
    void    confirmNewSR();
signals:
    void softwareReleaseChanged();
    void versionChanged();
    void variantChanged();
    void descriptionChanged();
    void updateUrlChanged();
    void applicationsChanged();
    void errorChanged();
    void advancedChanged();
    void multiscanChanged();
    void scanningChanged();
    void downloadingChanged();
    void hasPotentialLinksChanged();
    void hasBootAccessChanged();
    void dlProgressChanged();
    void currentIdChanged();
    void maxIdChanged();
    void splittingChanged();
    void splitProgressChanged();
    void currentFileChanged();
    void killSplit();
private slots:
    void reverseLookupReply();
    void serverReply();
    void showFirmwareData(QByteArray data, QString variant);
    void serverError(QNetworkReply::NetworkError error);
    void downloadFinish();
    void cancelSplit();
// Blackberry
	void extractImageSlot(const QStringList& selectedFiles);

private:
    // Utils:
    QString NPCFromLocale(int country, int carrier);

    QThread* splitThread;
    QNetworkReply *replydl;
    QNetworkAccessManager *manager;
    QString _softwareRelease;
    QString _versionRelease, _versionOS, _versionRadio;
    QString _variant;
    QString _description;
    QString _updateUrl;
    QString _applications;
    QString _links;
    QString _error;
    QString _multiscanVersion;
    bool _advanced, _downloading, _multiscan;
    bool _hasPotentialLinks;
    int _scanning;
    QFile _currentFile;
    QList<int> _sizes;
    int _currentId, _maxId, _dlBytes, _dlTotal;
    int _splitting, _splitProgress;
    int _dlProgress;
	int _options;
	int _type;
};
