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
        maxId = 0;
        size = 0;
        totalSize = 0;
        toVerify = 0;
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
        QNetworkReply* reply = _manager->head(QNetworkRequest(url));

        QObject::connect(reply, &QNetworkReply::finished, [=]() {
            if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt() != 200) {
                reset();
                QMessageBox::information(NULL, "Error", "The server did not have the " + type + " for the selected 'Download Device'.\n\nPlease try a different search result or a different download device.");
            } else {
                toVerify--;
                // Verified. Now lets complete
                if (toVerify == 0)
                    download();
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
        emit idChanged(); // For above, running
        QDir(baseDir).mkpath(".");
        downloadNextFile();
    }

    void downloadNextFile() {
        // Set to a temporary filename
        _updateFile.setFileName(baseDir + "/." + apps.at(id).name());
        // Obviously something is wrong if this file is bigger than what we want
        if (_updateFile.size() > apps.at(id).size())
            _updateFile.remove();
        // It is possible that it already exists and, in this case, we would want to resume
        _updateFile.open(QIODevice::WriteOnly | QIODevice::Append);
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

            // When we have size of OS/Radio working, should check the file was successful
            _updateFile.rename(baseDir + "/" + apps.at(id).name());

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
            apps.append(*newApp);
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
            return apps.at(i).friendlyName();
        else
            return "";
    }

    QString getUrl(int i = CURR_FILE) {
        if (i == CURR_FILE)
            i = id;
        if (i >= 0 && i < maxId)
            return apps.at(i).packageId();
        else
            return "";
    }

    int getSize(int i = CURR_FILE) {
        if (i == CURR_FILE)
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
    QList<Apps> apps;
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
