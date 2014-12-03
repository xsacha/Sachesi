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

#include <QXmlStreamReader>
#include "scanner.h"
#include "../ports.h"

void Scanner::clearHistory() {
    foreach(DiscoveredRelease* rel, _history) {
        rel->deleteLater();
    }
    _history.clear();
    emit historyChanged();
}

void Scanner::exportHistory() {
    QString historyText;
    foreach(DiscoveredRelease* rel, _history) {
        historyText.append(tr("SR: ") + " " + rel->srVersion() + " | " + tr("OS: ") + rel->osVersion() + " [");
        if (rel->activeServers() & 1)
            historyText.append(tr("Production") + ", ");
        if (rel->activeServers() & 2)
            historyText.append(tr("Beta") + ", ");
        if (rel->activeServers() & 4)
            historyText.append(tr("Alpha") + ", ");
        historyText.chop(2);
        historyText.append("]\n");
    }

    writeDisplayFile(tr("History"), historyText);
}

void Scanner::reverseLookup(QString OSver) {
    // If we only want real links, we're only going to want the server we can download from
    _isActive = true; emit isActiveChanged();
    _curRelease = new DiscoveredRelease();
    _curRelease->setOsVersion(OSver);
    emit curReleaseChanged();
    QString query = QString("<srVersionLookupRequest version=\"2.0.0\" authEchoTS=\"%1\">"
                            "<clientProperties>"
                            "<hardware><pin>0x2FFFFFB3</pin><bsn>1140011878</bsn><id>0x85002c0a</id></hardware>"
                            "<software><osVersion>%2</osVersion></software>"
                            "</clientProperties>"
                            "</srVersionLookupRequest>")
            .arg(QDateTime::currentMSecsSinceEpoch())
            .arg(OSver);
    QNetworkRequest request;
    request.setRawHeader("Content-Type", "text/xml;charset=UTF-8");
    QStringList serverList = QStringList("cs.sl");
    if (_findExisting != 1) {
        serverList << "beta2.sl.eval" << "alpha2.sl.eval";
    }
    _scansActive = serverList.count();
    foreach(QString server, serverList) {
        request.setUrl(QUrl(QString("https://%1.blackberry.com/%2cse/srVersionLookup/2.0/").arg(server).arg(server == "cs.sl" ? "" : "sls")));
        QNetworkReply* reply = _manager->post(request, query.toUtf8());
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(serverError(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(finished()), this, SLOT(newSRVersion()));
    }
}

void Scanner::newSRVersion() {
    QNetworkReply* reply = (QNetworkReply*)sender();
    QString swRelease;
    QByteArray data = reply->readAll();
    //for (int i = 0; i < data.size(); i += 3000) qDebug() << data.mid(i, 3000);
    QXmlStreamReader xml(data);
    while(!xml.atEnd() && !xml.hasError()) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "softwareReleaseVersion") {
                swRelease = xml.readElementText();
            }
        }
        xml.readNext();
    }

    // Software release has a version
    if (swRelease.startsWith('1')) {
        QString replyHost = reply->url().host();
        if (replyHost.startsWith("cs")) {
            _curRelease->setActiveServers(1);
        } else if (replyHost.startsWith("beta")) {
            _curRelease->setActiveServers(2);
        } else if (replyHost.startsWith("alpha")) {
            _curRelease->setActiveServers(4);
        }
        // Software release is new so we should check if it has a release
        if (swRelease != _curRelease->srVersion()) {
            _curRelease->setSrVersion(swRelease);
            // Don't care about Links?
            if (_findExisting == 2) {
                completeScan();
            } else {
                QCryptographicHash hash(QCryptographicHash::Sha1);
                hash.addData(swRelease.toLatin1());
                QString url = "http://cdn.fs.sl.blackberry.com/fs/qnx/production/" + QString(hash.result().toHex());
                QNetworkRequest request;
                request.setRawHeader("Content-Type", "text/xml;charset=UTF-8");
                request.setUrl(QUrl(url));
                QNetworkReply* replyTmp = _manager->head(request);
                connect(replyTmp, SIGNAL(finished()), this, SLOT(validateDownload()));
            }
            reply->deleteLater();
            return;
        }
    }
    completeScan();
    reply->deleteLater();
}

void Scanner::validateDownload()
{
    QNetworkReply* reply = (QNetworkReply*)sender();
    uint status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt();
    // Seems to give 301 redirect if it's real
    if (status == 200 || (status > 300 && status <= 308)) {
        _curRelease->setBaseUrl(reply->url().toString());
    }
    emit softwareReleaseChanged();
    completeScan();
    reply->deleteLater();
}

void appendNewHeader(QString *potentialText, QString name, QString devices) {
    potentialText->append("\n" + name + ": " + devices + " (Debrick + Core OS)\n");
}

void Scanner::appendNewLink(QString *potentialText, QString linkType, QString hwType, QString version) {
    bool os = (linkType == "Core" || linkType == "Debrick");
    if (!os) {
        potentialText->append(linkType + " Radio\n");
    }
    potentialText->append(_curRelease->baseUrl() + "/" + hwType + "-" + version + "-nto+armle-v7+signed.bar\n");
}


void Scanner::generatePotentialLinks() {
    QStringList parts = _curRelease->osVersion().split('.');
    // Just a guess that the Radio is +1. In some builds this isn't true.
    int build = parts.last().toInt() + 1;
    QString radioVersion = "";
    for (int i = 0; i < 3; i++)
        radioVersion += parts.at(i) + ".";
    radioVersion += QString::number(build);

    QString potentialText = QString("Potential OS and Radio links for SR" + _curRelease->srVersion() + " (OS:" + _curRelease->osVersion() + " + Radio:" + radioVersion + ")\n\n"
                                    "* Operating Systems *\n");
    appendNewHeader(&potentialText, "QC8974", "Blackberry Passport");
    appendNewLink(&potentialText, "Debrick", "qc8960.factory_sfi_hybrid_qc8974.desktop", _curRelease->osVersion());
    appendNewLink(&potentialText, "Core",    "qc8960.factory_sfi_hybrid_qc8974", _curRelease->osVersion());

    appendNewHeader(&potentialText, "QC8960", "Blackberry Z3/Z10/Z30/Q5/Q10");
    appendNewLink(&potentialText, "Debrick", "qc8960.factory_sfi.desktop", _curRelease->osVersion());
    appendNewLink(&potentialText, "Core",    "qc8960.factory_sfi", _curRelease->osVersion());

    appendNewHeader(&potentialText, "OMAP", "Blackberry Z10 STL 100-1");
    appendNewLink(&potentialText, "Debrick", "winchester.factory_sfi.desktop", _curRelease->osVersion());
    appendNewLink(&potentialText, "Core",    "winchester.factory_sfi", _curRelease->osVersion());

    potentialText.append("\n\n* Radios *\n");
    // Touch
    appendNewLink(&potentialText, "Z30 + Classic", "qc8960.wtr5", radioVersion);
    appendNewLink(&potentialText, "Z10 (STL 100-1)", "m5730", radioVersion);
    appendNewLink(&potentialText, "Z10 (STL 100-2/3/4) and Porsche P9982", "qc8960", radioVersion);
    appendNewLink(&potentialText, "Z3 (Jakarta) + Cafe", "qc8930.wtr5", radioVersion);
    // QWERTY
    appendNewLink(&potentialText, "Passport + Ontario", "qc8974.wtr2", radioVersion);
    appendNewLink(&potentialText, "Q5 + Q10 + Khan", "qc8960.wtr", radioVersion);
    writeDisplayFile(tr("VersionLookup"), potentialText);
}

void Scanner::serverError(QNetworkReply::NetworkError err) {
    Q_UNUSED(err);
    completeScan();
    sender()->deleteLater();
}

