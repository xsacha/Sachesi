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

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQmlListProperty>
#include <QXmlStreamReader>
#include "appworldapps.h"

class AppWorld : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool    listing         MEMBER _listing         NOTIFY listingChanged)
    Q_PROPERTY(int     server          MEMBER _server          NOTIFY serverChanged)
    Q_PROPERTY(QQmlListProperty<AppWorldApps> appList READ appList NOTIFY appListChanged)
    Q_PROPERTY(AppWorldApps* contentItem MEMBER _contentItem NOTIFY contentItemChanged)
    Q_PROPERTY(int appCount READ appCount NOTIFY appListChanged)
public:
    explicit AppWorld(QObject *parent = 0)
        : QObject(parent)
        , _server(0)
    {
        _manager = new QNetworkAccessManager();
        _contentItem = new AppWorldApps();
        showFeatured();
    }

    QQmlListProperty<AppWorldApps> appList() {
        return QQmlListProperty<AppWorldApps>(this, &_appList, &appendAppWorldApps, &appWorldAppsSize, &appWorldAppsAt, &clearAppWorldApps);
    }

    int appCount() const { return _appList.count(); }
    QString currentServer() {
        switch (_server) {
        case 1:
            return "http://eval.appworld.blackberry.com";
        case 0:
        default:
            return "http://appworld.blackberry.com";
        }
    }

    Q_INVOKABLE void showFeatured() {
        QString server = currentServer();
        QNetworkRequest request(QString("%1/ClientAPI/featured?model=0x85002c0a&os=10.3.0")
                                .arg(server));
        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "AppWorld/5.1.0.60");
        QNetworkReply* reply = _manager->get(request);
        connect(reply, &QNetworkReply::finished, [=] {
            int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (reply->size() > 0 && (status == 200 || (status > 300 && status <= 308))) {
                QXmlStreamReader xml(reply->readAll());
                foreach(AppWorldApps* app, _appList)
                    app->deleteLater();
                _appList.clear();
                emit appListChanged();
                QString imageDepot = server + "/ClientAPI/image";
                while(!xml.atEnd() && !xml.hasError()) {
                    if(xml.tokenType() == QXmlStreamReader::StartElement) {
                        if (xml.name() == "featured") {
                            imageDepot = xml.attributes().value("imagedepot").toString();
                        }
                        else if (xml.name() == "content") {
                            AppWorldApps* app = new AppWorldApps();
                            app->setId(xml.attributes().value("id").toString());
                            app->setImage(imageDepot + "/" + xml.attributes().value("cover").toString());
                            while(!xml.atEnd() && !xml.hasError()) {
                                xml.readNext();
                                if (xml.tokenType() == QXmlStreamReader::StartElement) {
                                    if (xml.name() == "product") {
                                        app->setType(xml.attributes().value("name").toString());
                                        //app->setTypeImage(imageDepot + "/" + xml.attributes().value("icon").toString());
                                    }
                                    else if (xml.name() == "name")
                                        app->setFriendlyName(xml.readElementText());
                                }
                                if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "content")
                                    break;
                            }
                            _appList.append(app);
                        }
                    }
                    xml.readNext();
                }
            }
            emit appListChanged();
            _listing = true; emit listingChanged();
            reply->deleteLater();
        });
        connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), [=]() {
            reply->deleteLater();
        });
    }

    Q_INVOKABLE void search(QString query) {
        QString server = currentServer();

        QNetworkRequest request(QString("%1/ClientAPI/searchpage?s=%2&model=0x85002c0a&os=10.9.0")
                                .arg(server)
                                .arg(query));
        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "AppWorld/5.1.0.60");
        QNetworkReply* reply = _manager->get(request);
        connect(reply, &QNetworkReply::finished, [=] {
            int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (reply->size() > 0 && (status == 200 || (status > 300 && status <= 308))) {
                QXmlStreamReader xml(reply->readAll());
                QString curType;
                foreach(AppWorldApps* app, _appList)
                    app->deleteLater();
                _appList.clear();
                while(!xml.atEnd() && !xml.hasError()) {
                    if(xml.tokenType() == QXmlStreamReader::StartElement) {
                        if (xml.name() == "panel") {
                            curType = xml.attributes().value("displayname").toString(); // Eg. Apps, Games
                        } else if (xml.name() == "link") {
                            AppWorldApps* app = new AppWorldApps();
                            app->setType(curType);
                            app->setName(xml.attributes().value("name").toString());
                            app->setFriendlyName(xml.attributes().value("displayname").toString());
                            app->setId(xml.attributes().value("id").toString());
                            while(!xml.atEnd() && !xml.hasError()) {
                                xml.readNext();
                                if (xml.tokenType() == QXmlStreamReader::StartElement) {
                                    if (xml.name() == "image" && xml.attributes().value("imagetype").toInt() == 1)
                                        app->setImage(xml.attributes().value("src").toString());
                                }
                                if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "link")
                                    break;
                            }
                            _appList.append(app);
                        }
                    }
                    xml.readNext();
                }
            }
            emit appListChanged();
            _listing = true; emit listingChanged();
            reply->deleteLater();
        });
        connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), [=]() {
            reply->deleteLater();
        });
    }

    Q_INVOKABLE void showContentItem(QString item) {
        QString server = currentServer();

        QNetworkRequest request(QString("%1/ClientAPI/content/%2?model=0x85002c0a&os=10.9.0")
                                .arg(server)
                                .arg(item));
        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "AppWorld/5.1.0.60");
        QNetworkReply* reply = _manager->get(request);
        connect(reply, &QNetworkReply::finished, [=] {
            int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (reply->size() > 0 && (status == 200 || (status > 300 && status <= 308))) {
                QXmlStreamReader xml(reply->readAll());
                QString imageDepot = server + "/ClientAPI/image";
                _contentItem->deleteLater();
                _contentItem = new AppWorldApps();
                while(!xml.atEnd() && !xml.hasError()) {
                    if(xml.tokenType() == QXmlStreamReader::StartElement) {
                        if (xml.name() == "content") {
                            imageDepot = xml.attributes().value("imagedepot").toString();
                            _contentItem->setImage(imageDepot + "/" + xml.attributes().value("icon").toString());
                            _contentItem->setId(xml.attributes().value("id").toString());
                        } else if (xml.name() == "name" && _contentItem->friendlyName() == "") {
                            _contentItem->setFriendlyName(xml.readElementText());
                        } else if (xml.name() == "product") {
                            _contentItem->setType(xml.attributes().value("name").toString());
                        } else if (xml.name() == "packageid") {
                            _contentItem->setPackageId(xml.readElementText());
                        } else if (xml.name() == "packagename") {
                            _contentItem->setName(xml.readElementText());
                        } else if (xml.name() == "description") {
                            _contentItem->setDescription(xml.readElementText());
                        } else if (xml.name() == "screenshot") {
                            _contentItem->_screenshots.append(imageDepot + "/" + xml.attributes().value("id").toString());
                            emit _contentItem->screenshotsChanged();
                        } else if (xml.name() == "vendor") {
                            _contentItem->setVendor(xml.readElementText());
                        } else if (xml.name() == "filebundle") {
                            _contentItem->setVersion(xml.attributes().value("version").toString());
                            _contentItem->setSize(xml.attributes().value("size").toString().toInt());
                            _contentItem->setFileId(xml.attributes().value("id").toString());
                        }
                    }
                    xml.readNext();
                }
            }
            emit contentItemChanged();
            _listing = false; emit listingChanged();
            reply->deleteLater();
        });
        connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), [=]() {
            reply->deleteLater();
        });
    }

signals:
    void listingChanged();
    void serverChanged();
    void appListChanged();
    void contentItemChanged();

public slots:

private:
    bool _listing;
    int _server;
    QList<AppWorldApps*> _appList;
    AppWorldApps* _contentItem;
    QNetworkAccessManager* _manager;

};
