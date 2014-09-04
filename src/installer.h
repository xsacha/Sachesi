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
#include <QDataStream>
#include <QQmlListProperty>
#include <QCryptographicHash>
#include <QSettings>
#include <QFileDialog>
#include <QThread>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#ifndef _WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include "apps.h"
#include "backupinfo.h"

enum BarType {
    NotInstallableType = 0,
    ApplicationType,
    RadioType,
    OSType,
};

struct BarInfo {
    QString name;
    QString version;
    BarType type;
};

class SslNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    SslNetworkAccessManager() {}

protected:
    QNetworkReply* createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);
};

class InstallNet : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString password         MEMBER _password        WRITE setPassword         NOTIFY passwordChanged)
    Q_PROPERTY(bool wrongPass           MEMBER _wrongPass       WRITE setWrongPass        NOTIFY wrongPassChanged)
    Q_PROPERTY(bool wrongPassBlock      MEMBER _wrongPassBlock  WRITE setWrongPassBlock   NOTIFY wrongPassBlockChanged)
    Q_PROPERTY(int possibleDevices      MEMBER _possibleDevices WRITE setPossibleDevices  NOTIFY possibleDevicesChanged)
    Q_PROPERTY(QString ip               MEMBER _ip              WRITE setIp               NOTIFY ipChanged)
    Q_PROPERTY(QString newLine          MEMBER _newLine         WRITE setNewLine          NOTIFY newLineChanged)
    Q_PROPERTY(int     state            MEMBER _state           WRITE setState            NOTIFY stateChanged)
    Q_PROPERTY(int     dgPos            MEMBER _downgradePos                              NOTIFY dgPosChanged)
    Q_PROPERTY(int     dgMaxPos         READ dgMaxPos                                     NOTIFY dgMaxPosChanged)
    Q_PROPERTY(int     dgProgress       MEMBER _dgProgress      WRITE setDGProgress       NOTIFY dgProgressChanged)
    Q_PROPERTY(int     curDGProgress    MEMBER _curDGProgress   WRITE setCurDGProgress    NOTIFY curDGProgressChanged)
    Q_PROPERTY(QString curInstallName   MEMBER _curInstallName  WRITE setCurInstallName   NOTIFY curInstallNameChanged)
    Q_PROPERTY(bool    completed        MEMBER _completed                                 NOTIFY completedChanged)
    Q_PROPERTY(bool    installing       MEMBER _installing      WRITE setInstalling       NOTIFY installingChanged)
    Q_PROPERTY(bool    restoring        MEMBER _restoring       WRITE setRestoring        NOTIFY restoringChanged)
    Q_PROPERTY(bool    backing          MEMBER _backing         WRITE setBacking          NOTIFY backingChanged)
    Q_PROPERTY(bool    firmwareUpdate   MEMBER _firmwareUpdate  WRITE setFirmwareUpdate   NOTIFY firmwareUpdateChanged)
    Q_PROPERTY(QStringList firmwareNames MEMBER _firmwareNames                            NOTIFY firmwareNamesChanged)
    Q_PROPERTY(QStringList firmwarePaths MEMBER _firmwarePaths                            NOTIFY firmwarePathsChanged)
    Q_PROPERTY(QString knownOS          MEMBER _knownOS         WRITE setKnownOS          NOTIFY knownOSChanged)
    Q_PROPERTY(QString knownRadio       MEMBER _knownRadio      WRITE setKnownRadio       NOTIFY knownRadioChanged)
    Q_PROPERTY(int knownBattery         MEMBER _knownBattery    WRITE setKnownBattery     NOTIFY knownBatteryChanged)
    Q_PROPERTY(QString knownName        MEMBER _knownName       WRITE setKnownName        NOTIFY knownNameChanged)
    Q_PROPERTY(QString knownHW          MEMBER _knownHW         WRITE setKnownHW          NOTIFY knownHWChanged)
    Q_PROPERTY(QString knownPIN         MEMBER _knownPIN        WRITE setKnownPIN         NOTIFY knownPINChanged)
    Q_PROPERTY(QQmlListProperty<Apps> appList READ appList                                NOTIFY appListChanged)
    Q_PROPERTY(int appCount READ appCount NOTIFY appListChanged)

    Q_PROPERTY(QString backStatus READ backStatus NOTIFY backStatusChanged)
    Q_PROPERTY(int     backProgress READ backProgress NOTIFY backCurProgressChanged)
    Q_PROPERTY(int     backCurProgress READ backCurProgress NOTIFY backCurProgressChanged)

    Q_PROPERTY(int     backMethods READ backMethods NOTIFY backMethodsChanged)
    Q_PROPERTY(QStringList backNames READ backNames NOTIFY backMethodsChanged)
    Q_PROPERTY(QList<double> backSizes READ backSizes NOTIFY backMethodsChanged)
public:
    InstallNet(QObject* parent = 0);
    ~InstallNet();
    Q_INVOKABLE void keepAlive();
    Q_INVOKABLE void scanProps();
    Q_INVOKABLE void install(QList<QUrl> files);
    Q_INVOKABLE void uninstall(QStringList packageids);
    Q_INVOKABLE bool uninstallMarked();
    Q_INVOKABLE void restore(QUrl url, int options);
    Q_INVOKABLE void backup(QUrl url, int options);
    Q_INVOKABLE void wipe();
    Q_INVOKABLE void startRTAS();
    Q_INVOKABLE void newPin(QString pin);
    Q_INVOKABLE void resignNVRAM();
    Q_INVOKABLE void factorywipe();
    Q_INVOKABLE void reboot();
    Q_INVOKABLE void getPIN();
    Q_INVOKABLE void backupQuery();
    Q_INVOKABLE void exportInstalled();
    Q_INVOKABLE QString appDeltaMsg();
    void requestConfigure();
    void requestChallenge();
    void replyChallenge();
    void requestAuthenticate();
    void authorise();
    QByteArray HashPass(QByteArray challenge, QByteArray salt, int iterations);
    void logadd(QString logtxt);
    void AESEncryptSend(QByteArray &plain, int code);
    QString password() const;
    int dgMaxPos() const;
    int curDGProgress() const;
    QQmlListProperty<Apps> appList();
    int appCount() const { return _appList.count(); }
    BackupInfo* back();
    QString backStatus() const;
    int backProgress() const;
    int backCurProgress() const;
    int backMethods() const;
    QStringList backNames() const;
    QList<double> backSizes() const;

    void setIp(const QString &ip);
    void setPassword(const QString &password);
    void setWrongPass(const bool &wrong);
    void setWrongPassBlock(const bool &wrong);
    void setPossibleDevices(const int &devices);
    void setNewLine(const QString &newLine);
    void setState(const int &state);
    void setDGProgress(const int &progress);
    void setCurDGProgress(const int &progress);
    void setCurInstallName(const QString &name);
    void setCompleted(const bool &exists);
    void setInstalling(const bool &installing);
    void setRestoring(const bool &restoring);
    void setBacking(const bool &backing);
    void setFirmwareUpdate(const bool &firmwareUpdate);
    void setKnownOS(const QString &OS);
    void setKnownRadio(const QString &Radio);
    void setKnownBattery(const int &battery);
    void setKnownName(const QString &Name);
    void setKnownHW(const QString &HW);
    void setKnownPIN(const QString &PIN);
signals:
    void passwordChanged();
    void newPassword(QString newPass);
    void wrongPassChanged();
    void wrongPassBlockChanged();
    void possibleDevicesChanged();
    void ipChanged();
    void newLineChanged();
    void stateChanged();
    void completedChanged();
    void dgPosChanged();
    void dgMaxPosChanged();
    void dgProgressChanged();
    void curDGProgressChanged();
    void curInstallNameChanged();
    void installingChanged();
    void restoringChanged();
    void backingChanged();

    void backStatusChanged();
    void backCurProgressChanged();
    void backMethodsChanged();

    void firmwareUpdateChanged();
    void firmwareNamesChanged();
    void firmwarePathsChanged();
    void knownOSChanged();
    void knownRadioChanged();
    void knownBatteryChanged();
    void knownNameChanged();
    void knownHWChanged();
    void knownPINChanged();
    void appListChanged();
private slots:
    bool checkLogin();
    void login();
    void connected();
    void disconnected();
    //void endConnect()
    void restoreSendFile();
    void restoreReply();
    void discoveryReply();
    void resetVars();
    void restoreError(QNetworkReply::NetworkError error);
    void installProgress(qint64 pread, qint64);
    void backupProgress(qint64 pread, qint64 psize);
    void restoreProgress(qint64 pwrite, qint64 psize);
    void backupFileReady();
    void backupFileFinish();
    void dumpLogs();
    void setActionProperty(QString name, QString value);
private:
    QNetworkRequest setData(QString page, QString contentType);
    QNetworkReply* postQuery(QString page, QString contentType, const QUrlQuery& query);
    QNetworkReply* getQuery(QString page, QString contentType);
    BarInfo checkInstallableInfo(QString name);
    void install();
    void restore();
    void backup();
    QTcpSocket* sock;
    unsigned char* serverChallenge;
    RSA* privkey;
    QByteArray sessionKey;
    QByteArray hashedPassword;
    QTimer* connectTimer;
    QProcess proc;
#ifdef _MSC_VER
    WSADATA wsadata;
#endif
    QNetworkAccessManager* dlmanager;
    SslNetworkAccessManager* manager;
    QNetworkReply *reply;
    QNetworkCookieJar* cookieJar;
    QFile* compressedFile;
    QStringList _firmwareNames;
    QStringList _firmwarePaths;
    QString _knownOS;
    QString _knownRadio;
    int _knownBattery;
    QString _knownName;
    QString _knownHW;
    QString _knownPIN;
    int _knownProtocol;

    QStringList _firmwareInfo;
    QStringList _downgradeInfo;
    int _downgradePos;
    int _firmwarePos;

    QString _ip;
    QString _password;
    bool _wrongPass;
    bool _wrongPassBlock;
    int _possibleDevices;
    QString _newLine;
    QFile _firmware;
    int _state;
    quint64 _dlBytes;
    quint64 _dlTotal;
    int _dgProgress;
    int _curDGProgress;
    QString _curInstallName;
    BackupInfo _back;
    bool _completed;
    bool _firmwareUpdate;
    QList<BarInfo> _installInfo;
    QString _backupFileName;
    QStringList _currentApps;
    bool _installing;
    bool _restoring;
    bool _backing;
    bool _hadPassword;
    QuaZip* currentBackupZip;
    QuaZipFile* _zipFile;
    QList<Apps*> _appList;
    QList<Apps*> _appRemList;
};
