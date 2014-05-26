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

#ifndef MAINNET_H
#define MAINNET_H
#include <QtNetwork>
#include <QObject>
#include "splitter.h"

class MainNet : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString softwareRelease READ softwareRelease NOTIFY softwareReleaseChanged) // from reverse lookup
    Q_PROPERTY(QString versionRelease READ versionRelease NOTIFY versionChanged)
    Q_PROPERTY(QString versionOS READ versionOS NOTIFY versionChanged)
    Q_PROPERTY(QString versionRadio READ versionRadio NOTIFY versionChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(QString url READ url NOTIFY urlChanged)
    Q_PROPERTY(QString applications READ applications NOTIFY applicationsChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(bool    advanced READ advanced WRITE setAdvanced NOTIFY advancedChanged)
    Q_PROPERTY(bool    scanning READ scanning WRITE setScanning NOTIFY scanningChanged)
    Q_PROPERTY(bool    downloading READ downloading WRITE setDownloading NOTIFY downloadingChanged)
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
    QString softwareRelease() const;
    QString versionRelease() const;
    QString versionOS() const;
    QString versionRadio() const;
    QString description() const;
    QString url() const;
    QString applications() const;
    QString error() const;
    bool    advanced() const;
    bool    scanning() const;
    void    setAdvanced(const bool &advanced);
    void    setScanning(const bool &scanning);
    bool    downloading() const;
    void    setDownloading(const bool &downloading);
    int     dlProgress() const;
    void    setDLProgress(const int &progress);
    int     currentId() const;
    int     maxId() const;
    int     splitting() const;
    int     splitProgress() const;
    QString currentFile() const;
public slots:
    void    setSplitProgress(const int &progress);
    void    capNetworkReply(QNetworkReply* reply);
signals:
    void softwareReleaseChanged();
    void versionChanged();
    void descriptionChanged();
    void urlChanged();
    void applicationsChanged();
    void errorChanged();
    void advancedChanged();
    void scanningChanged();
    void downloadingChanged();
    void dlProgressChanged();
    void currentIdChanged();
    void maxIdChanged();
    void splittingChanged();
    void splitProgressChanged();
    void currentFileChanged();
    void killSplit();
    void populateFirmwareData(QByteArray data);
private slots:
    void reverseLookupReply();
    void serverReply();
    void serverError(QNetworkReply::NetworkError error);
    void downloadFinish();
    void cancelSplit();
// Blackberry
	void splitAutoloaderSlot(const QStringList& fileNames);
	void extractImageSlot(const QStringList& selectedFiles);

private:
    // Utils:
    QString NPCFromLocale(int country, int carrier);
    QString HWIDFromVariant(int variant);


    QThread* splitThread;
    QNetworkReply *reply, *replydl;
    QNetworkAccessManager *manager;
    QString _softwareRelease;
    QString _versionRelease, _versionOS, _versionRadio;
    QString _description;
    QString _url;
    QString _applications;
    QString _links;
    QString _error;
    bool _advanced, _scanning, _downloading;
    QFile _currentFile;
    QList<int> _sizes;
    int _currentId, _maxId, _dlBytes, _dlTotal;
    int _splitting, _splitProgress;
    int _dlProgress;
	int _options;
	int _type;
};

#endif // MAINNET_H
