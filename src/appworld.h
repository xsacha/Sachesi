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

// Notes:
// Alternative solution is to use JSON-frontend
// Example: https://appworld.blackberry.com/cas/producttype/all/listtype/search_listing/category/0/search/%1
//          https://appworld.blackberry.com/cas/content/%1
// Advantages: No user agent required, doesn't discriminate OS/model
// Disadvantages: We need to use the ClientAPI for everything else anyway

#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include "appworldapps.h"

class AppWorld : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool    listing         MEMBER _listing         NOTIFY listingChanged)
    Q_PROPERTY(QStringList more        READ more               NOTIFY moreChanged)
    Q_PROPERTY(int     server          MEMBER _server          NOTIFY serverChanged)
    Q_PROPERTY(int     osVer           MEMBER _osVer           NOTIFY osVerChanged)
    Q_PROPERTY(QQmlListProperty<AppWorldApps> appList READ appList NOTIFY appListChanged)
    Q_PROPERTY(AppWorldApps* contentItem MEMBER _contentItem NOTIFY contentItemChanged)
    Q_PROPERTY(int appCount READ appCount NOTIFY appListChanged)
public:
    explicit AppWorld(QObject *parent = 0)
        : QObject(parent)
        , _listing(false)
        , _more()
        , _server(0)
        , _osVer(0)
    {
        _manager = new QNetworkAccessManager();
        _contentItem = new AppWorldApps();
        showHome();
    }

    QQmlListProperty<AppWorldApps> appList() {
        return QQmlListProperty<AppWorldApps>(this, &_appList, &appendAppWorldApps, &appWorldAppsSize, &appWorldAppsAt, &clearAppWorldApps);
    }

    int appCount() const { return _appList.count(); }

    QStringList more() const { return _more; }

    QString currentServer() {
        switch (_server) {
        case 2:
            return "http://eval.appworld.blackberry.com";
        case 1:
            return "http://enterprise.appworld.blackberry.com";
        case 0:
        default:
            return "http://appworld.blackberry.com";
        }
    }
    QString currentOS() {
        switch (_osVer) {
        case 1:
            return "";
        case 0:
        default:
            return "&os=10.9.0";
        }
    }

    Q_INVOKABLE void searchLocker(QString type) {
        QString server = currentServer();
        QNetworkRequest request(QString("%1/ClientAPI/%2?model=0x85002c0a" + currentOS())
                                .arg(server)
                                .arg(type));
        request.setRawHeader("User-Agent", "AppWorld/5.1.0.60");
        QNetworkReply* reply = _manager->get(request);
        connect(reply, &QNetworkReply::finished, [=] {
            int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (reply->size() > 0 && (status == 200 || (status > 300 && status <= 308))) {
                QXmlStreamReader xml(reply->readAll());
                foreach(AppWorldApps* app, _appList)
                    app->deleteLater();
                _appList.clear();
                _more.clear(); emit moreChanged();
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
                                    else if (xml.name() == "image" && xml.attributes().value("imagetype").toInt() == 1)
                                        app->setImage(xml.attributes().value("src").toString());
                                    else if (xml.name() == "name")
                                        app->setFriendlyName(xml.readElementText());
                                    else if (xml.name() == "vendor")
                                        app->setVendor(xml.readElementText());
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
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), reply, SLOT(deleteLater()));
    }

    Q_INVOKABLE void search(QString query) {
        QString server = currentServer();
        searchRequest(QString("%1/ClientAPI/searchlist?s=%2&model=0x85002c0a" + currentOS())
                      .arg(server)
                      .arg(query));
    }

    Q_INVOKABLE void showVendor(QString id) {
        QString server = currentServer();
        searchRequest(QString("%1/ClientAPI/vendorlistforappsgames/%2?model=0x85002c0a" + currentOS())
                      .arg(server)
                      .arg(id));
    }

    Q_INVOKABLE void showCars() {
        QString server = currentServer();
        searchRequest(QString("%1/ClientAPI/autostorepages/?model=0x85002c0a&os=10.9.0") // Requires an OS version, so use latest
                      .arg(server));
    }

    Q_INVOKABLE void showHome() {
        QString server = currentServer();
        searchRequest(QString("%1/ClientAPI/usfpage/?model=0x85002c0a&os=10.9.0") // Requires an OS version, so use latest
                      .arg(server));
    }

    Q_INVOKABLE void searchMore(QString url, bool os_required = false) {
        if (os_required)
            searchRequest(url + "model=0x85002c0a&os=10.9.0");
        else
            searchRequest(url + "model=0x85002c0a" + currentOS());
    }

    void searchRequest(QString requestString) {
        QNetworkRequest request(requestString);
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
                _more.clear(); emit moreChanged();
                while(!xml.atEnd() && !xml.hasError()) {
                    if(xml.tokenType() == QXmlStreamReader::StartElement) {
                        if (xml.name() == "panel") {
                            curType = xml.attributes().value("displayname").toString(); // Eg. Apps, Games
                        } else if (xml.name() == "link") {
                            int linkType = xml.attributes().value("linktype").toInt();
                            if (linkType == 3
                                    || (linkType == 1 && xml.attributes().value("onactionbar").toString() == "false")
                                    || (linkType == 2 && xml.attributes().value("showicon").toString() == "true")) {
                                AppWorldApps* app = new AppWorldApps();
                                if (linkType == 1 || linkType == 2) {
                                    app->setType("category");
                                    app->setId(xml.attributes().value("url").toString());
                                }
                                else {
                                    app->setType(curType);
                                    app->setName(xml.attributes().value("name").toString());
                                    app->setId(xml.attributes().value("id").toString());
                                }
                                app->setFriendlyName(xml.attributes().value("displayname").toString());
                                while(!xml.atEnd() && !xml.hasError()) {
                                    xml.readNext();
                                    if (xml.tokenType() == QXmlStreamReader::StartElement) {
                                        if (((xml.name() == "image") && (xml.attributes().value("imagetype").toInt() == 1))||(xml.attributes().value("imagetype").toInt() == 7))
                                        {
                                            app->setImage(xml.attributes().value("src").toString());
                                        }
                                        else if (xml.name() == "vendor") {
                                            app->setVendorId(xml.attributes().value("id").toString());
                                            app->setVendor(xml.readElementText());
                                        } else if (xml.name() == "autocheck") {
                                            app->setFriendlyName(xml.attributes().value("make").toString());
                                        }
                                    }
                                    if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "link")
                                        break;
                                }
                                _appList.append(app);
                            } else if (linkType == 2 || linkType == 1) {
                                _more.append(xml.attributes().value("displayname").toString() + "," + xml.attributes().value("url").toString()); emit moreChanged();
                            } else if (linkType == 13) {
                                // This is either used for next page (no name) or a specific task like Search
                                if (!xml.attributes().hasAttribute("displayname"))
                                    _more.append("Next Page," + xml.attributes().value("url").toString()); emit moreChanged();
                            }
                        }
                    }
                    xml.readNext();
                }
            }
            emit appListChanged();
            _listing = true; emit listingChanged();
            reply->deleteLater();
        });
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), reply, SLOT(deleteLater()));
    }

    Q_INVOKABLE void showContentItem(QString item) {
        QString server = currentServer();

        QNetworkRequest request(QString("%1/ClientAPI/content/%2?model=0x85002c0a" + currentOS())
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
                            _contentItem->setDescription(QString("This application was created %1 and last updated %2.\n\n")
                                                         .arg(QDateTime::fromMSecsSinceEpoch(xml.attributes().value("createddate").toString().toLongLong()).toString())
                                                         .arg(QDateTime::fromMSecsSinceEpoch(xml.attributes().value("forsaledate").toString().toLongLong()).toString()));
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
                            _contentItem->setDescription(_contentItem->description() + xml.readElementText());
                        } else if (xml.name() == "screenshot") {
                            _contentItem->_screenshots.append(imageDepot + "/" + xml.attributes().value("id").toString());
                            emit _contentItem->screenshotsChanged();
                        } else if (xml.name() == "vendor") {
                            _contentItem->setVendorId(xml.attributes().value("id").toString());
                            _contentItem->setVendor(xml.readElementText());
                        } else if (xml.name() == "filebundle") {
                            _contentItem->setVersion(xml.attributes().value("version").toString());
                            _contentItem->setSize(xml.attributes().value("size").toString().toInt());
                            _contentItem->setFileId(xml.attributes().value("id").toString());
                        } else if (xml.name() == "price" && _contentItem->price() == "") {
                            _contentItem->setPrice(xml.readElementText());
                        } else if (xml.name() == "releasenotes") {
                            _contentItem->setDescription(_contentItem->description() + "\n\nRelease Notes (" + _contentItem->version() + ")\n" + xml.readElementText());
                        }
                    }
                    xml.readNext();
                }
            }
            emit contentItemChanged();
            _listing = false; emit listingChanged();
            reply->deleteLater();
        });
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), reply, SLOT(deleteLater()));
    }

signals:
    void listingChanged();
    void moreChanged();
    void serverChanged();
    void osVerChanged();
    void appListChanged();
    void contentItemChanged();

private:
    bool _listing;
    QStringList _more;
    int _server;
    int _osVer;
    QList<AppWorldApps*> _appList;
    AppWorldApps* _contentItem;
    QNetworkAccessManager* _manager;

};
