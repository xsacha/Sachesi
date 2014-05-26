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

#ifndef INSTALL_H
#define INSTALL_H

#include <QtNetwork>
#include <QDataStream>
#include <QDeclarativeContext>
#include <QDeclarativeListProperty>
#include <QCryptographicHash>
#include <QSettings>
#include <QFileDialog>
#include <QThread>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/pem.h>
#ifndef Q_WS_WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include "apps.h"
#include "backupinfo.h"

class SslNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    SslNetworkAccessManager();


protected:
    QNetworkReply* createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);
};

class InstallNet : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(bool wrongPass READ wrongPass WRITE setWrongPass NOTIFY wrongPassChanged)
    Q_PROPERTY(bool wrongPassBlock READ wrongPassBlock WRITE setWrongPassBlock NOTIFY wrongPassBlockChanged)
    Q_PROPERTY(QString ip       READ ip       WRITE setIp       NOTIFY ipChanged)
    Q_PROPERTY(QString newLine  READ newLine  WRITE setNewLine  NOTIFY newLineChanged)
    Q_PROPERTY(int     state    READ state    NOTIFY stateChanged)
    Q_PROPERTY(int     dgPos READ dgPos NOTIFY dgPosChanged)
    Q_PROPERTY(int     dgMaxPos READ dgMaxPos NOTIFY dgMaxPosChanged)
    Q_PROPERTY(int     dgProgress READ dgProgress WRITE setDGProgress NOTIFY dgProgressChanged)
    Q_PROPERTY(int     curDGProgress READ curDGProgress WRITE setCurDGProgress NOTIFY curDGProgressChanged)
    Q_PROPERTY(QString currentInstallName READ currentInstallName WRITE setCurrentInstallName NOTIFY currentInstallNameChanged)
    Q_PROPERTY(bool    completed READ completed NOTIFY completedChanged)
    Q_PROPERTY(bool    installing READ installing WRITE setInstalling NOTIFY installingChanged)
    Q_PROPERTY(bool    restoring READ restoring NOTIFY restoringChanged)
    Q_PROPERTY(bool    backing READ backing NOTIFY backingChanged)
    Q_PROPERTY(bool    firmwareUpdate READ firmwareUpdate WRITE setFirmwareUpdate NOTIFY firmwareUpdateChanged)
    Q_PROPERTY(QStringList firmwareNames READ firmwareNames NOTIFY firmwareNamesChanged)
    Q_PROPERTY(QStringList firmwarePaths READ firmwarePaths NOTIFY firmwarePathsChanged)
    Q_PROPERTY(QString knownOS READ knownOS WRITE setKnownOS NOTIFY knownOSChanged)
    Q_PROPERTY(int knownBattery READ knownBattery WRITE setKnownBattery NOTIFY knownBatteryChanged)
    Q_PROPERTY(QString knownName READ knownName WRITE setKnownName NOTIFY knownNameChanged)
    Q_PROPERTY(QString knownHW READ knownHW WRITE setKnownHW NOTIFY knownHWChanged)
    Q_PROPERTY(QString knownPIN READ knownPIN WRITE setKnownPIN NOTIFY knownPINChanged)
    Q_PROPERTY(QDeclarativeListProperty<Apps> appList READ appList NOTIFY appListChanged)
    Q_PROPERTY(int appCount READ appCount NOTIFY appListChanged)

    Q_PROPERTY(QString backStatus READ backStatus NOTIFY backStatusChanged)
    Q_PROPERTY(int     backProgress READ backProgress NOTIFY backCurProgressChanged)
    Q_PROPERTY(int     backCurProgress READ backCurProgress NOTIFY backCurProgressChanged)

    Q_PROPERTY(int     backMethods READ backMethods NOTIFY backMethodsChanged)
    Q_PROPERTY(QStringList backNames READ backNames NOTIFY backMethodsChanged)
    Q_PROPERTY(QStringList backSizes READ backSizes NOTIFY backMethodsChanged)
public:
    InstallNet(QObject* parent = 0);
    ~InstallNet();
    Q_INVOKABLE void keepAlive();
    Q_INVOKABLE void scanProps();
    Q_INVOKABLE void listApps();
    Q_INVOKABLE bool selectInstallFolder();
    Q_INVOKABLE bool selectInstall();
    Q_INVOKABLE void install();
    Q_INVOKABLE void install(QStringList files);
    Q_INVOKABLE void uninstall(QStringList packageids);
    Q_INVOKABLE bool uninstallMarked();
    Q_INVOKABLE void restore();
    Q_INVOKABLE void selectRestore(int options);
    Q_INVOKABLE void backup();
    Q_INVOKABLE void selectBackup(int options);
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
    bool wrongPass() const;
    bool wrongPassBlock() const;
    QString ip() const;
    QString newLine() const;
    int state() const;
    int dgPos() const;
    int dgMaxPos() const;
    int dgProgress() const;
    int curDGProgress() const;
    QString currentInstallName() const;
    bool completed() const;
    bool installing() const;
    bool restoring() const;
    bool backing() const;
    bool firmwareUpdate() const;
    QStringList firmwareNames() const;
    QStringList firmwarePaths() const;
    QString knownOS() const;
    int knownBattery() const;
    QString knownName() const;
    QString knownHW() const;
    QString knownPIN() const;
    QDeclarativeListProperty<Apps> appList();
    int appCount() const { return _appList.count(); }
    BackupInfo* back();
    QString backStatus() const;
    int backProgress() const;
    int backCurProgress() const;
    int backMethods() const;
    QStringList backNames() const;
    QStringList backSizes() const;

    void setIp(const QString &ip);
    void setPassword(const QString &password);
    void setWrongPass(const bool &wrong);
    void setWrongPassBlock(const bool &wrong);
    void setNewLine(const QString &newLine);
    void setState(const int &state);
    void setDGProgress(const int &progress);
    void setCurDGProgress(const int &progress);
    void setCurrentInstallName(const QString &name);
    void setCompleted(const bool &exists);
    void setInstalling(const bool &installing);
    void setRestoring(const bool &restoring);
    void setBacking(const bool &backing);
    void setFirmwareUpdate(const bool &firmwareUpdate);
    void setKnownOS(const QString &OS);
    void setKnownBattery(const int &battery);
    void setKnownName(const QString &Name);
    void setKnownHW(const QString &HW);
    void setKnownPIN(const QString &PIN);
signals:
    void passwordChanged();
    void newPassword(QString newPass);
    void wrongPassChanged();
    void wrongPassBlockChanged();
    void ipChanged();
    void newLineChanged();
    void stateChanged();
    void completedChanged();
    void dgPosChanged();
    void dgMaxPosChanged();
    void dgProgressChanged();
    void curDGProgressChanged();
    void currentInstallNameChanged();
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
    void knownBatteryChanged();
    void knownNameChanged();
    void knownHWChanged();
    void knownPINChanged();
    void appListChanged();
private slots:
    void requestLogin();
    void login();
    void connected();
    void disconnected();
    //void endConnect()
    void restoreSendFile();
    void restoreReply();
    void discoveryReply();
    void restoreError(QNetworkReply::NetworkError error);
    void installProgress(qint64 pread, qint64);
    void backupProgress(qint64 pread, qint64 psize);
    void restoreProgress(qint64 pwrite, qint64 psize);
    void backupFileReady();
    void backupFileFinish();
    void dumpLogs();
    void setActionProperty(QString name, QString value);
private:
    void setData(QString page, QString contentType);

    QTcpSocket* sock;
    unsigned char* serverChallenge;
    RSA* privkey;
    QByteArray sessionKey;
    QByteArray hashedPassword;
    QTimer* connectTimer;
    QProcess proc;
#ifdef Q_WS_WIN32
    WSADATA wsadata;
#endif
    QNetworkAccessManager* dlmanager;
    SslNetworkAccessManager* manager;
    QNetworkReply *reply, *replydl;
    QNetworkCookieJar* cookieJar;
    QNetworkRequest request;
    QFile* compressedFile;
    QStringList _firmwareNames;
    QStringList _firmwarePaths;
    QString _knownOS;
    int _knownBattery;
    QString _knownName;
    QString _knownHW;
    QString _knownPIN;

    QStringList _firmwareInfo;
    QStringList _downgradeInfo;
    int _downgradePos;
    int _firmwarePos;

    QString _ip;
    QString _password;
    bool _wrongPass;
    bool _wrongPassBlock;
    QString _newLine;
    QFile _firmware;
    int _state;
    quint64 _dlBytes;
    quint64 _dlTotal;
    int _dgProgress;
    int _curDGProgress;
    QString _currentInstallName;
    BackupInfo _back;
    bool _completed;
    bool _firmwareUpdate;
    QStringList _fileNames;
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

#endif // InstallNetNET_H
