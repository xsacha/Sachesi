#pragma once
#include <QObject>
#include <QDir>
#include <QFile>
#include "apps.h"

class DownloadInfo : public QObject {
    Q_OBJECT
    Q_PROPERTY(int     id          MEMBER id          NOTIFY idChanged)
    Q_PROPERTY(int     maxId       MEMBER maxId       NOTIFY appsChanged)
    Q_PROPERTY(int     curProgress MEMBER curProgress NOTIFY sizeChanged)
    Q_PROPERTY(int     progress    MEMBER progress    NOTIFY sizeChanged)
    Q_PROPERTY(int     size        MEMBER size        NOTIFY sizeChanged)
    Q_PROPERTY(qint64  totalSize   MEMBER totalSize   NOTIFY appsChanged)
    Q_PROPERTY(QString curName     MEMBER curName     NOTIFY idChanged)
public:
    DownloadInfo(QObject* parent = 0)
        : QObject(parent)
        , baseDir("")
        , id(0), maxId(0)
        , progress(-1), curProgress(0)
        , size(0), totalSize(0)
        , starting(false)
    {
    }

    Q_INVOKABLE void reset() {
        maxId = 0;
        size = 0;
        totalSize = 0;
        progress = -1;
        apps.clear();
        curName.clear();
        emit sizeChanged();
    }

    Q_INVOKABLE void start() {
        starting = true;
    }
    bool isStarting() {
        bool isRequested = starting;
        starting = false;
        return isRequested;
    }

    void setApps(QList<Apps*> newApps, QString& version) {
        baseDir = QDir::currentPath() + "/" + version;

        // Check which apps user wanted
        for (Apps* newApp : newApps) {
            if (!newApp->isMarked())
                continue;
            apps.append(*newApp);
        }

        // Check which apps user doesn't already have
        for(int i = 0; i < apps.count(); i++) {
            QString fileName = baseDir + "/" + apps.at(i).name();
            // Either check signature or size to confirm?
            if (QFile::exists(fileName)) {
                apps.removeAt(i--);
            } else {
                // NOTE: Guide only! Maybe we should ask server for real filesizes beforehand
                totalSize += apps.at(i).size();
            }
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

        if (id >= 0 && id < maxId)
            curName = apps.at(id).friendlyName();

        emit idChanged();

        return (id < maxId);
    }

    QString baseDir;
    QList<Apps> apps;
    int id, maxId;
    int progress, curProgress;
    qint64 curSize, size, totalSize;
    QString curName;
    bool starting;
signals:
    void idChanged();
    void sizeChanged();
    void appsChanged();
};
