#pragma once
#include <QDir>
#include <QFile>
#include <QDesktopServices>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QMessageBox>
#include "apps.h"

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
        , verifyLink(0)
        , running(false)
    {
        _manager = new QNetworkAccessManager();
    }

    Q_INVOKABLE void reset() {
        if (_updateFile.isOpen())
            _updateFile.close();

        starting = false;
        running = false;
        maxId = 0;
        size = 0;
        totalSize = 0;
        verifyLink = 0;
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
    void download() {
        running = true;
        emit idChanged(); // For above, running
        QDir(baseDir).mkpath(".");
        downloadNextFile();
    }
    void downloadNextFile() {
        _updateFile.setFileName(getFilename());
        _updateFile.open(QIODevice::WriteOnly);
        QNetworkReply* replydl = _manager->get(QNetworkRequest(getUrl()));

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

            if (nextFile())
            {
                downloadNextFile();
            } else {
                // May not be 100% equal to totalSize because we switched out some files actually
                QDesktopServices::openUrl(QUrl(QFileInfo(_updateFile).absolutePath()));
                reset();
            }
        });
        QObject::connect(replydl, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), [=]() {
            qDebug() << "DL Error: " << replydl->errorString() << replydl->error();
            replydl->deleteLater();
            _updateFile.remove();
            reset();
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
            apps.append(*newApp);
            totalSize += newApp->size();
        }

        maxId = apps.count();
        nextFile(0);
        emit appsChanged();
    }

    QString getFilename(int i = -1) {
        if (i == -1)
            i = id;
        if (i >= 0 && i < maxId)
            return baseDir + "/" + apps.at(i).name();
        else
            return "";
    }

    QString getName(int i = -1) {
        if (i == -1)
            i = id;
        if (i >= 0 && i < maxId)
            return apps.at(i).friendlyName();
        else
            return "";
    }

    QString getUrl(int i = -1) {
        if (i == -1)
            i = id;
        if (i >= 0 && i < maxId)
            return apps.at(i).packageId();
        else
            return "";
    }
    int getSize(int i = -1) {
        if (i == -1)
            i = id;
        if (i >= 0 && i < maxId)
            return apps.at(i).size();
        else
            return 0;
    }
    void progressSize(qint64 bytes) {
        size += bytes;
        curSize += bytes;
        curProgress = 100*curSize / apps.at(id).size();
        if (totalSize == 0)
            progress = 0;
        else
            progress = 100*size / totalSize;
        emit sizeChanged();
    }

    bool nextFile(int i = -1) {
        curSize = 0;
        if (i == -1)
            id++;
        else
            id = i;

        emit idChanged();

        return (id < maxId);
    }

    bool verifying() const {
        return verifyLink > 0;
    }

    QString baseDir;
    QList<Apps> apps;
    int id, maxId;
    int progress, curProgress;
    qint64 curSize, size, totalSize;
    bool starting;
    qint16 verifyLink;
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
