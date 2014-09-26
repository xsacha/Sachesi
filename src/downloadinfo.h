#pragma once
#include <QDir>
#include <QFile>
#include <QDesktopServices>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QMessageBox>
#include "apps.h"

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

    void verifyLink(QString url, QString type) {
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
                    }
                }

                toVerify--;
                emit verifyingChanged();
                // Verified. Now lets complete
                if (toVerify == 0)
                    download();
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
        });
    }

    void download() {
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
                }
            }
        }
        emit idChanged(); // For above, running=true and if any apps changed
        QDir(baseDir).mkpath(".");
        downloadNextFile();
    }

    void downloadNextFile() {
        QNetworkRequest request(getUrl());
        request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
        // Set to a temporary filename
        _updateFile.setFileName(baseDir + "/." + apps.at(id)->name());
        // Obviously something is wrong if this file is bigger than what we want
        if (_updateFile.size() > apps.at(id)->size())
            _updateFile.remove();
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
                // Pretend like that didn't happen and try again
                size -= _updateFile.size();
                _updateFile.close();
                _updateFile.remove();
                curSize = 0;
                downloadNextFile();
                return;
            }

            if (nextFile())
            {
                downloadNextFile();
            } else {
                if (size != totalSize)
                    QMessageBox::information(NULL, "Warning", "Your update completed successfully. However, the update size does not match the download size. This could just be a bug.");

                QDesktopServices::openUrl(QUrl(QFileInfo(_updateFile).absolutePath()));
                reset();
            }
        });
        QObject::connect(replydl, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), [=]() {
            replydl->deleteLater();
            reset();
            if (replydl->error() == 5)
                return; // User cancelled

            qDebug() << "DL Error: "  << replydl->error() << replydl->errorString();
        });
    }

    void setApps(QList<Apps*> newApps, QString& version) {
        baseDir = QDir::currentPath() + "/" + version;

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
            return apps[i]->packageId();
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
