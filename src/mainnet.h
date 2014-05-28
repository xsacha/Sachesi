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
    Q_PROPERTY(QString softwareRelease READ softwareRelease NOTIFY softwareReleaseChanged) // from reverse lookup
    Q_PROPERTY(QString versionRelease READ versionRelease NOTIFY versionChanged)
    Q_PROPERTY(QString versionOS READ versionOS NOTIFY versionChanged)
    Q_PROPERTY(QString versionRadio READ versionRadio NOTIFY versionChanged)
    Q_PROPERTY(QString variant READ variant NOTIFY variantChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(QString url READ url NOTIFY urlChanged)
    Q_PROPERTY(QString applications READ applications NOTIFY applicationsChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString multiscanVersion READ multiscanVersion NOTIFY versionChanged)
    Q_PROPERTY(bool    advanced READ advanced WRITE setAdvanced NOTIFY advancedChanged)
    Q_PROPERTY(bool    downloading READ downloading WRITE setDownloading NOTIFY downloadingChanged)
    Q_PROPERTY(bool    hasBootAccess READ hasBootAccess NOTIFY hasBootAccessChanged)
    Q_PROPERTY(bool    multiscan READ multiscan WRITE setMultiscan NOTIFY multiscanChanged)
    Q_PROPERTY(int     scanning READ scanning WRITE setScanning NOTIFY scanningChanged)
    Q_PROPERTY(int     dlProgress READ dlProgress WRITE setDLProgress NOTIFY dlProgressChanged)
    Q_PROPERTY(int     currentId READ currentId NOTIFY currentIdChanged)
    Q_PROPERTY(int     maxId READ maxId NOTIFY maxIdChanged)
    Q_PROPERTY(int     splitting READ splitting NOTIFY splittingChanged)
    Q_PROPERTY(int     splitProgress READ splitProgress WRITE setSplitProgress NOTIFY splitProgressChanged)
    Q_PROPERTY(QString currentFile READ currentFile NOTIFY currentFileChanged)

public:
    MainNet(QObject* parent = 0);
    ~MainNet();
    Q_INVOKABLE void updateDetailRequest(QString delta, QString carrier, QString country, int device, int variant, int mode, int server/*, int version*/);
    Q_INVOKABLE void downloadLinks();
    Q_INVOKABLE void splitAutoloader(int options);
    Q_INVOKABLE void combineFolder();
    Q_INVOKABLE void combineFiles();
    Q_INVOKABLE void combineAutoloader(QStringList selectedFiles);
    Q_INVOKABLE void extractImage(int type, int options);
    Q_INVOKABLE void grabLinks();
    Q_INVOKABLE void grabPotentialLinks(QString softwareRelease, QString osVersion);
    Q_INVOKABLE void abortDL(QNetworkReply::NetworkError error = (QNetworkReply::NetworkError)0);
    Q_INVOKABLE void abortSplit();
    Q_INVOKABLE void reverseLookup(QString carrier, QString country, int device, int variant, int server, QString OSver);
    Q_INVOKABLE void downloadPotentialLink(QString softwareRelease, QString osVersion);
    Q_INVOKABLE QString nameFromVariant(unsigned int device, unsigned int variant);
    Q_INVOKABLE QString hwidFromVariant(unsigned int device, unsigned int variant);
    Q_INVOKABLE unsigned int variantCount(unsigned int device);
    QString softwareRelease()const { return _softwareRelease; }
    QString versionRelease() const { return _versionRelease; }
    QString versionOS()      const { return _versionOS; }
    QString versionRadio()   const { return _versionRadio; }
    QString variant()        const { return _variant; }
    QString description()    const { return _description; }
    QString url()            const { return _url; }
    QString applications()   const { return _applications; }
    QString error()          const { return _error; }
    QString multiscanVersion() const { return _multiscanVersion; }
    bool    advanced()       const { return _advanced; }
    bool    downloading()    const { return _downloading; }
    bool    hasBootAccess()  const { return
#ifdef BOOTLOADER_ACCESS
                true;
#else
                false;
#endif
                                   }
    bool    multiscan()      const { return _multiscan; }
    int     scanning()       const { return _scanning; }
    int     dlProgress()     const { return _dlProgress; }
    int     currentId()      const { return _currentId; }
    int     maxId()          const { return _maxId; }
    int     splitting()      const { return _splitting; }
    int     splitProgress()  const { return _splitProgress; }
    void    setMultiscan(const bool &multiscan);
    void    setScanning(const int &scanning);
    void    setDLProgress(const int &progress);
    void    setAdvanced(const bool &advanced);
    void    setDownloading(const bool &downloading);
    QString currentFile() const;
public slots:
    void    setSplitProgress(const int &progress);
    void    capNetworkReply(QNetworkReply* reply);
signals:
    void softwareReleaseChanged();
    void versionChanged();
    void variantChanged();
    void descriptionChanged();
    void urlChanged();
    void applicationsChanged();
    void errorChanged();
    void advancedChanged();
    void multiscanChanged();
    void scanningChanged();
    void downloadingChanged();
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
	void splitAutoloaderSlot(const QStringList& fileNames);
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
    QString _url;
    QString _applications;
    QString _links;
    QString _error;
    QString _multiscanVersion;
    bool _advanced, _downloading, _multiscan;
    int _scanning;
    QFile _currentFile;
    QList<int> _sizes;
    int _currentId, _maxId, _dlBytes, _dlTotal;
    int _splitting, _splitProgress;
    int _dlProgress;
	int _options;
	int _type;
};
