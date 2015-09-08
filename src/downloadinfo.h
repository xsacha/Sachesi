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
#include <QDir>
#include <QFile>
#include <QDesktopServices>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QMessageBox>
#include "apps.h"
#include "ports.h"

// Need to get creative with CURR_FILE when threading comes
enum {
    CURR_FILE = -1,
};

class DownloadInfo : public QObject {
    Q_OBJECT
    Q_PROPERTY(int     id          MEMBER id          NOTIFY idChanged)
    Q_PROPERTY(int     maxId       MEMBER maxId       NOTIFY appsChanged)
    Q_PROPERTY(int     curProgress MEMBER curProgress NOTIFY sizeChanged)
    Q_PROPERTY(int     progress    MEMBER progress    NOTIFY sizeChanged)
    Q_PROPERTY(int     size        MEMBER size        NOTIFY sizeChanged)
    Q_PROPERTY(qint64  totalSize   MEMBER totalSize   NOTIFY appsChanged)
    Q_PROPERTY(QString curName     READ   getName     NOTIFY idChanged)
    Q_PROPERTY(bool    verifying   READ   verifying   NOTIFY verifyingChanged)
    Q_PROPERTY(bool    running     MEMBER running     NOTIFY idChanged)
public:
    DownloadInfo(QObject* parent = 0)
        : QObject(parent)
        , baseDir("")
        , id(0), maxId(0)
        , progress(0), curProgress(0)
        , size(0), totalSize(0)
        , starting(false)
        , toVerify(0)
        , running(false)
    {
        _manager = new QNetworkAccessManager();
    }

    Q_INVOKABLE void reset() {
        if (_updateFile.isOpen())
            _updateFile.close();

        starting = false;
        running = false;
        id = 0;
        maxId = 0;
        size = 0;
        totalSize = 0;
        toVerify = 0;
        foreach(Apps* app, apps) {
            if (app != nullptr) {
                delete app;
                app = nullptr;
            }
        }
        apps.clear();
        emit sizeChanged();
        emit idChanged();
        emit appsChanged();
    }

    Q_INVOKABLE void start() {
        starting = true;
        if (_updateFile.isOpen()) {
            _updateFile.close();
            _updateFile.remove();
        }
    }

    bool isStarting() {
        bool isRequested = starting;
        starting = false;
        return isRequested;
    }

    bool verifying() const {
        return toVerify > 0;
    }

    void verifyDelta(int i) {
        toVerify++;
        emit verifyingChanged();
        QString url = apps.at(i)->url();
        QString oldVersion = apps.at(i)->installedVersion();
        oldVersion.replace('.','_');
        url.chop(4); // Remove extension
        url.append(QString("+patch+%1.bar").arg(oldVersion));
        QNetworkReply* reply = _manager->head(QNetworkRequest(url));

        QObject::connect(reply, &QNetworkReply::finished, [=]() {
            uint status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt();
            if (status == 200 || (status > 300 && status <= 308)) {
                uint realSize = reply->header(QNetworkRequest::ContentLengthHeader).toUInt();
                // Adjust the expected size
                totalSize += realSize - apps.at(i)->size();
                apps.at(i)->setSize(realSize);
                apps.at(i)->setUrl(url);
                emit sizeChanged();
                emit appsChanged();
            }
            toVerify--;
            emit verifyingChanged();
            // Verified. Now to complete
            if (toVerify == 0)
                startDownload();
            reply->deleteLater();
        });
        QObject::connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), [=]() {
            toVerify--;
            emit verifyingChanged();
            // Verified. Now to complete
            if (toVerify == 0)
                startDownload();
            reply->deleteLater();
        });
    }

    void verifyLink(QString url, QString type, bool delta) {
        toVerify++;
        emit verifyingChanged();
        QNetworkReply* reply = _manager->head(QNetworkRequest(url));

        QObject::connect(reply, &QNetworkReply::finished, [=]() {
            uint status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt();
            if (status == 200 || (status > 300 && status <= 308)) {
                uint realSize = reply->header(QNetworkRequest::ContentLengthHeader).toUInt();
                // Adjust the expected size
                foreach(Apps* app, apps) {
                    if (app->type() == type.toLower()) {
                        totalSize += realSize - app->size();
                        app->setSize(realSize);
                        emit sizeChanged();
                        emit appsChanged();
                    }
                }

                toVerify--;
                emit verifyingChanged();
                // Verified. Now to complete
                if (toVerify == 0)
                    download(delta);
            } else {
                reset();
                QMessageBox::information(NULL, "Error", "The server did not have the " + type + " for the selected 'Download Device'.\n\nPlease try a different search result or a different download device.");
            }
            reply->deleteLater();
        });
        QObject::connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), [=]() {
            if (toVerify > 0) {
                QMessageBox::information(NULL, "Error", "Encountered an error when attempting to verify the " + type +".\n Aborting download.");
                reset();
            }
            reply->deleteLater();
        });
    }

    void download(bool delta) {
        // Check for deltas
        if (delta) { // Checking for connected
            for (int i = 0; i < apps.count(); i++) {
                if (!apps.at(i)->isMarked() || apps.at(i)->installedVersion().isEmpty())
                    continue;
                if (isVersionNewer(apps.at(i)->version(), apps.at(i)->installedVersion(), false))
                    verifyDelta(i);
            }
        }
        if (toVerify == 0)
            startDownload();
    }

    void startDownload() {
        running = true;
        // OS and Radio files were just verified, so lets check if we already have them
        for (int i = 0; i < apps.count(); i++) {
            if (apps[i]->type() == "os" || apps[i]->type() == "radio") {
                QFileInfo fileInfo(baseDir + "/" + apps[i]->name());
                if (fileInfo.exists() && fileInfo.size() == apps[i]->size()) {
                    totalSize -= apps[i]->size();
                    delete apps[i];
                    apps.removeAt(i--);
                    maxId--;
                    emit appsChanged();
                    emit sizeChanged();
                }
            }
        }
        if (apps.count() == 0) {
            running = false;
            return;
        }
        emit idChanged(); // For above, running=true and if any apps changed
        QDir(baseDir).mkpath(".");
        downloadNextFile();
    }

    void downloadNextFile() {
        curSize = 0;
        QNetworkRequest request(getUrl());
        request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
        // Set to a temporary filename
        _updateFile.setFileName(baseDir + "/." + apps.at(id)->name());
        // Obviously something is wrong if this file is bigger than what we want
        if (_updateFile.size() > apps.at(id)->size()) {
            if (QMessageBox::warning(nullptr, "Issue",
                                     QString("Expected filesize of %1 did not match (Expected %2, Received %3). Ignore the warning or discard the file to try again?").arg(apps.at(id)->name()).arg(apps.at(id)->size()).arg(_updateFile.size()),
                                     QMessageBox::Discard, QMessageBox::Ignore) == QMessageBox::Discard) {
                _updateFile.remove();
            }
        }
        // In the unlikely event that we missed that we had already downloaded it
        // Maybe the user copied the relevant files in to a new folder during download?
        if (_updateFile.size() == apps.at(id)->size()) {
            size += _updateFile.size();
            _updateFile.rename(baseDir + "/" + apps.at(id)->name());
            completedFile();
            return;
        }
        // It is possible that it already exists and, in this case, we would want to resume
        if (_updateFile.size() > 0) {
            size += _updateFile.size();
            curSize += _updateFile.size();
            request.setRawHeader("Range", QString("bytes=%1-").arg(_updateFile.size()).toLatin1());
        }
        _updateFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QNetworkReply* replydl = _manager->get(request);

        connect(replydl, &QNetworkReply::readyRead, [=]() {
            if (!running) {
                replydl->abort();
                replydl->deleteLater();
                return;
            }

            QByteArray data = replydl->readAll();
            // Is this our first receive?
            if (size == 0)
            {
                if (data.startsWith("<?xml")) {
                    QMessageBox::critical(nullptr, "Error", "You are restricted from downloading this file.");
                    replydl->abort();
                    replydl->deleteLater();
                    reset();
                    return;
                }
            }
            progressSize(data.size());
            _updateFile.write(data);
        });
        connect(replydl, &QNetworkReply::finished, [=]() {
            replydl->deleteLater();

            // Then why are we here?
            if (!running)
                return;

            // This should always match otherwise I'm pretty sure something bad happened
            if (_updateFile.size() == apps.at(id)->size())
                _updateFile.rename(baseDir + "/" + apps.at(id)->name());
            else {
                if (QMessageBox::warning(nullptr, "Issue",
                                         QString("Expected filesize of %1 did not match (Expected %2, Received %3). Ignore the warning or discard the file to try again?").arg(apps.at(id)->name()).arg(apps.at(id)->size()).arg(_updateFile.size()),
                                         QMessageBox::Discard, QMessageBox::Ignore) == QMessageBox::Discard)
                {
                    // Discard and try again
                    size -= _updateFile.size();
                    _updateFile.close();
                    _updateFile.remove();
                    downloadNextFile();
                    return;
                }
                // Pretend like that didn't happen
            }

            completedFile();
        });
        QObject::connect(replydl, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), [=]() {
            replydl->deleteLater();
            reset();
            if (replydl->error() == 5)
                return; // User cancelled

            qDebug() << "DL Error: "  << replydl->error() << replydl->errorString();
        });
    }

    void completedFile() {
        if (nextFile())
        {
            downloadNextFile();
        } else {
            /*if (size != totalSize)
                QMessageBox::information(NULL, "Warning", QString("Your update completed successfully.\n"
                                         "However, the update size does not match the download size. This is probably just be a bug that you can ignore.\n"
                                         "Downloaded: %1\n"
                                         "Expected: %2")
                                         .arg(size)
                                         .arg(totalSize));*/

            QDesktopServices::openUrl(QUrl(QFileInfo(_updateFile).absolutePath()));
            reset();
        }
    }

    void setApps(QList<Apps*> newApps, QString& version) {
        baseDir = getSaveDir() + "/" + version;

        // Check which apps user wanted
        foreach (Apps* newApp, newApps) {
            if (!newApp->isMarked())
                continue;
            // We have to verify OS/Radio first
            if (newApp->type() == "application") {
                // Check which apps user has already downloaded
                QFileInfo fileInfo(baseDir + "/" + newApp->name());
                if (fileInfo.exists() && fileInfo.size() == newApp->size())
                    continue;
            }
            apps.append(new Apps(newApp, this));
            totalSize += newApp->size();
        }

        maxId = apps.count();
        nextFile(0);
        emit appsChanged();
    }

    QString getName(int i = CURR_FILE) {
        if (i == CURR_FILE)
            i = id;
        if (i >= 0 && i < maxId)
            return apps[i]->friendlyName();
        else
            return "";
    }

    QString getUrl(int i = CURR_FILE) {
        if (i == CURR_FILE)
            i = id;
        if (i >= 0 && i < maxId)
            return apps[i]->url();
        else
            return "";
    }

    int getSize(int i = CURR_FILE) {
        if (i == CURR_FILE)
            i = id;
        if (i >= 0 && i < maxId)
            return apps[i]->size();
        else
            return 0;
    }
    void progressSize(qint64 bytes) {
        size += bytes;
        curSize += bytes;
        curProgress = 100*curSize / apps.at(id)->size();
        if (totalSize == 0)
            progress = 0;
        else
            progress = 100*size / totalSize;
        emit sizeChanged();
    }

    bool nextFile(int i = CURR_FILE) {
        curSize = 0;
        if (i == CURR_FILE)
            id++;
        else
            id = i;

        emit idChanged();

        return (id < maxId);
    }

    QString baseDir;
    QList<Apps*> apps;
    int id, maxId;
    int progress, curProgress;
    qint64 curSize, size, totalSize;
    bool starting;
    qint16 toVerify;
    bool running;
signals:
    void idChanged();
    void sizeChanged();
    void appsChanged();
    void verifyingChanged();
private:
    QFile _updateFile;
    QNetworkAccessManager* _manager;
};
