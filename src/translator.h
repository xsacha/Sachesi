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

#include <QApplication>
#include <QTranslator>

class Translator : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool exists MEMBER _exists NOTIFY existsChanged)
    Q_PROPERTY(QString lang MEMBER _lang NOTIFY langChanged)
public:
    Translator()
        : QObject()
        , _exists(false)
    {
        // Install translator by locale language string
        // zh_HK is considered 'Chinese' language but the characters are entirely different.
        if ((QLocale().name() != "zh_HK") && _appTranslator.load(QString(":/translations/%1.qm")
                                                                .arg(QLocale::languageToString(QLocale().language())))
                ) {
            _exists = true;
        }
        emit existsChanged();
    }
    Q_INVOKABLE void load() {
        if (_exists) {
            qApp->installTranslator(&_appTranslator);
            emit langChanged();
        }
    }
    Q_INVOKABLE void remove() {
        qApp->removeTranslator(&_appTranslator);
        emit langChanged();
    }

signals:
    void existsChanged();
    void langChanged();

private:
    bool _exists;
    QString _lang;
    QTranslator _appTranslator;
};
