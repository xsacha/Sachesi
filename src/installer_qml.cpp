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

// This file is just Qt hooks that don't need to be seen. It should really do this for you anyway.

#include "installer.h"
#include "ports.h"

#define WRITE_QML(type, name, caps) \
    void InstallNet::caps(const type &var) { \
    if (var != _ ## name) { \
    _ ## name = var; \
    emit name ## Changed(); \
    } \
    }

// _()

WRITE_QML(bool, loginBlock, setLoginBlock)
WRITE_QML(int, possibleDevices, setPossibleDevices)
void InstallNet::setState(const int &state) {
    if (!state) {
        qSafeFree(manager);
        qSafeFree(cookieJar);
    }
    _state = state;
    emit stateChanged();
}

int InstallNet::dgMaxPos() const {
    return _downgradeInfo.count();
}
WRITE_QML(QString, curInstallName, setCurInstallName)
WRITE_QML(bool, installing, setInstalling)
WRITE_QML(bool, firmwareUpdate, setFirmwareUpdate)
WRITE_QML(bool, allowDowngrades, setAllowDowngrades)

void InstallNet::setCurDGProgress(const int &curDGProgress) {
    _curDGProgress = curDGProgress;
    if (curDGProgress == -1) {
        _dgProgress = -1;
    } else {
        qint64 curBytes = ((qint64)_curDGProgress * _dlTotal);
        _dgProgress = (curBytes + _dlDoneBytes) / _dlOverallTotal;
    }
    emit curDGProgressChanged();
}

QQmlListProperty<Apps> InstallNet::appList()
{
    return QQmlListProperty<Apps>(this, _appList);
}

QQmlListProperty<Apps> InstallNet::backAppList()
{
    return QQmlListProperty<Apps>(this, _back.apps);
}

void InstallNet::setIp(const QString &ip)
{
    _ip = ip;
    emit ipChanged();
    QSettings settings("Qtness","Sachesi");
    if (settings.value("ip","169.254.0.1").toString() != ip)
        settings.setValue("ip",ip);
}

void InstallNet::setWrongPass(const bool &wrong)
{
    _wrongPass = wrong;
    if (wrong) {
        setState(0);
        setCompleted(false);
    }
    emit wrongPassChanged();
}

void InstallNet::setCompleted(const bool &exists)
{
    _completed = exists;
    if (_loginBlock && _completed)
        setLoginBlock(false);
    emit completedChanged();
}

void InstallNet::setNewLine(const QString &newLine)
{
    // Prefix with date
    QString newLog = QTime().currentTime().toString("[hh:mm:ss] ") + newLine;

    _newLine = ""; // Reset qproperty and notify
    emit newLineChanged();
    _newLine = newLog + "<br>";
    emit newLineChanged();
    newLog.append("\n");
    logFile->write(newLog.toLatin1());
    logFile->flush(); // Update file so it can be viewed in real-time
    emit hasLogChanged();
}

bool InstallNet::hasLog() {
    return logFile->fileName() != "";
}

void InstallNet::openLog() {
    if (logFile->fileName() != "")
      openFile(logFile->fileName());
}

QString InstallNet::backStatus() const {
    QString ret = _back.curMode();
    ret[0] = ret[0].toUpper();
    return ret;
}

int InstallNet::backProgress() const {
    return qMin((int)100, (int)((_back.size() + _back.curSize()) / _back.maxSize()));
}

int InstallNet::backCurProgress() const {
    return _back.progress();
}

QStringList InstallNet::backNames() const {
    QStringList names;
    foreach(BackupCategory* cat, _back.categories)
        names.append(cat->name);
    return names;
}

QList<double> InstallNet::backSizes() const {
    QList<double> sizes;
    foreach(BackupCategory* cat, _back.categories)
        sizes.append(cat->bytesize.toLongLong());
    return sizes;
}

int InstallNet::backMethods() const {
    return _back.numMethods();
}

void InstallNet::setBacking(const bool &backing) {
    _backing = backing;
    emit backingChanged();
    if (!backing)
    {
        _back.setMode(0);
        for (int i = 0; i < _back.numMethods(); i++)
            _back.setCurMaxSize(i, 1);
        _back.setSize(0);
        _back.setMaxSize(1);
    }
}

void InstallNet::setRestoring(const bool &restoring) {
    _restoring = restoring;
    emit restoringChanged();
    if (!restoring) {
        _back.setMode(0);
        for (int i = 0; i < _back.numMethods(); i++)
            _back.setCurMaxSize(i, 1);
        _back.setSize(0);
        _back.setMaxSize(1);
    }
}

// Note: not overly secure if user is specifically targeted.
void InstallNet::setPassword(const QString &password) {
    _password = password;
    emit newPassword(password);
    QByteArray tmp = password.toLatin1();
    for (int i = 0; i < password.length(); i++) {
        tmp[i] = tmp[i] ^ ((0x40 + 5 * i - password.length()) % 127);
    }
    QByteArray tmp2(1, tmp.length());
    QByteArray hashedPass = tmp.toBase64();
    hashedPass.prepend(tmp2.toBase64());
    QSettings settings("Qtness","Sachesi");
    settings.setValue("pass", hashedPass);
    emit passwordChanged();
}
