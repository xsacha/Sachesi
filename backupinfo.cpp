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

#include "backupinfo.h"

BackupInfo::BackupInfo() :
    _size(0), _maxSize(1), _progress(0),
    _mode(0), _curMode(0), _numMethods(0)
{
}

void BackupInfo::clearModes() {
    _curMode = 0;
    categories.clear();
    _curSize.clear();
    _curMaxSize.clear();
    emit curModeChanged();
    emit numMethodsChanged();
}

void BackupInfo::addMode(QXmlStreamAttributes cat) {
    categories.append(new BackupCategory(cat));
    _numMethods++;
    emit numMethodsChanged();
    _curSize.append(0);
    _curMaxSize.append(1);
}

QString BackupInfo::modeString()
{
    QString ret = "";
    for (int i = 0; i < _numMethods; i++)
    {
        if (_mode & (1 << i)) {
            if (ret != "") ret += "_";
            ret += categories.at(i)->id;
        }
    }
    return ret;
}

QString BackupInfo::stringFromMode(int mode) {
    if (mode >= 0 && mode < _numMethods)
        return categories.at(mode)->id;
    if (mode == _numMethods)
        return "complete";
    return "";
}

int BackupInfo::progress() const {
    return _progress;
}

void BackupInfo::setProgress(const int &progress) {
    _progress = progress;
    emit progressChanged();
}

int BackupInfo::mode() const {
    return _mode;
}

void BackupInfo::setMode(const int &mode) {
    _mode = mode;
    emit modeChanged();
}

QString BackupInfo::curMode() const {
    if (_curMode == _numMethods)
        return "complete";
    return categories.at(_curMode)->id;
}

void BackupInfo::setCurMode(int increment) {
    if (increment) {
        if (_numMethods > _curMode && _curSize[_curMode])
            _size += _curSize[_curMode];
        _curMode++;
    } else {
        _curMode = 0;
    }
    while (!(_mode & (1 << _curMode)) && _curMode < _numMethods)
        _curMode++;
    if (_curMode < _numMethods)
        _curSize[_curMode] = 0;
    emit curModeChanged();
    emit curSizeChanged();
    emit sizeChanged();
}

qint64 BackupInfo::size() const {
    return _size;
}

void BackupInfo::setSize(const qint64 &val) {
    _size = val;
    emit sizeChanged();
}

qint64 BackupInfo::maxSize() const {
    return _maxSize;
}

void BackupInfo::setMaxSize(const qint64 &val) {
    _maxSize = val;
    emit maxSizeChanged();
}

qint64 BackupInfo::curSize() const {
    if (_curMode == _numMethods)
        return 1;
    if (_numMethods > _curMode)
        return _curSize[_curMode];
    else
        return 0;
}

void BackupInfo::setCurSize(const qint64 &val) {
    if (_numMethods > _curMode)
        _curSize[_curMode] = val;
    emit curSizeChanged();
}

qint64 BackupInfo::curMaxSize() const {
    if (_curMode == _numMethods)
        return 1;

    if (_numMethods > _curMode)
        return _curMaxSize[_curMode];
    else
        return 1;
}

void BackupInfo::setCurMaxSize(const qint64 &val) {
    if (_numMethods > _curMode)
        _curMaxSize[_curMode] = val;
    emit curMaxSizeChanged();
}

void BackupInfo::setCurMaxSize(int index, const qint64 &val) {
    _curMaxSize[index] = val;
    emit curMaxSizeChanged();
}

int BackupInfo::numMethods() const {
    return categories.size();
}
