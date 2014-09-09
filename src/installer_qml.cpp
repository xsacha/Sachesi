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
        cookieJar->deleteLater();
        cookieJar = nullptr;
        manager->deleteLater();
        manager = nullptr;
    }
    _state = state;
    emit stateChanged();
}

int InstallNet::dgMaxPos() const {
    return _downgradeInfo.count();
}
WRITE_QML(int,  dgProgress, setDGProgress)
WRITE_QML(int,  curDGProgress, setCurDGProgress)
WRITE_QML(QString, curInstallName, setCurInstallName)
WRITE_QML(bool, installing, setInstalling)
WRITE_QML(bool, firmwareUpdate, setFirmwareUpdate)
WRITE_QML(QString, knownOS, setKnownOS)
WRITE_QML(QString, knownRadio, setKnownRadio)
WRITE_QML(int,  knownBattery, setKnownBattery)
WRITE_QML(QString, knownName, setKnownName)
WRITE_QML(QString, knownPIN, setKnownPIN)
WRITE_QML(QString, knownHW, setKnownHW)


void appendApps(QQmlListProperty<Apps> * property, Apps * app)
{
    Q_UNUSED(property);
    Q_UNUSED(app);
    //Do nothing. Can't add to Apps using this method
}
int appsSize(QQmlListProperty<Apps> * property)
{
    return static_cast< QList<Apps *> *>(property->data)->size();
}
Apps* appsAt(QQmlListProperty<Apps> * property, int index)
{
    return static_cast< QList<Apps *> *>(property->data)->at(index);
}
void clearApps(QQmlListProperty<Apps> *property)
{
    return static_cast< QList<Apps *> *>(property->data)->clear();
}

QQmlListProperty<Apps> InstallNet::appList()
{
    return QQmlListProperty<Apps>(this, &_appList, &appendApps, &appsSize, &appsAt, &clearApps);
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
    _newLine = ""; // Otherwise it thinks it didn't change...
    emit newLineChanged();
    _newLine = newLine + "<br>";
    emit newLineChanged();
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
        sizes.append(cat->bytesize.toLongLong() / (double)(1024.0 * 1024.0));
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
    /*if (_wrongPass)
        setWrongPass(false);*/
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
