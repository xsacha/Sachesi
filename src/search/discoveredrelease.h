#pragma once
#include <QObject>
#include <QString>

class DiscoveredRelease : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString osVersion READ osVersion WRITE setOsVersion NOTIFY osVersionChanged)
    Q_PROPERTY(QString srVersion READ srVersion WRITE setSrVersion NOTIFY srVersionChanged)
    Q_PROPERTY(int  activeServers      READ activeServers      NOTIFY activeServersChanged)
    Q_PROPERTY(QString baseUrl READ baseUrl WRITE setBaseUrl NOTIFY baseUrlChanged)
public:
    DiscoveredRelease() : QObject(), _activeServers(0) { }
    DiscoveredRelease(const DiscoveredRelease& release)
    : QObject()
    , _osVersion(release.osVersion())
    , _srVersion(release.srVersion())
    , _activeServers(release.activeServers()) {}

    QString osVersion() const { return _osVersion; }
    QString srVersion() const { return _srVersion; }
    int activeServers() const { return _activeServers; }
    QString baseUrl() const { return _baseUrl; }
    void setOsVersion(QString osVersion) { _osVersion = osVersion; emit osVersionChanged(); }
    void setSrVersion(QString srVersion) { _srVersion = srVersion; emit srVersionChanged(); }
    void setActiveServers(int activeServers) { _activeServers |= activeServers; emit activeServersChanged(); }
    void setBaseUrl(QString baseUrl) { _baseUrl = baseUrl; emit baseUrlChanged(); }

signals:
    void osVersionChanged();
    void srVersionChanged();
    void activeServersChanged();
    void baseUrlChanged();

private:
    QString _osVersion;
    QString _srVersion;
    int _activeServers;
    QString _baseUrl;
};
Q_DECLARE_METATYPE(DiscoveredRelease* );
