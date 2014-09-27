#pragma once
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QXmlStreamReader>

class CarrierInfo : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString country     MEMBER _country     NOTIFY resultChanged)
    Q_PROPERTY(QString carrier     MEMBER _carrier     NOTIFY resultChanged)
    Q_PROPERTY(QString mcc         MEMBER _mcc         NOTIFY resultChanged)
    Q_PROPERTY(QString mnc         MEMBER _mnc         NOTIFY resultChanged)
    Q_PROPERTY(int     image       MEMBER _image       NOTIFY resultChanged)

public:
    CarrierInfo(QObject* parent = 0)
        : QObject(parent)
    {
        _manager = new QNetworkAccessManager();
        _timer = new QTimer();
        _timer->setSingleShot(true);
        connect(_timer, SIGNAL(timeout()), this, SLOT(update()));
        _mcc = "510";
        _mnc = "010";
        //_country = "Indonesia";
        //_carrier = "TELKOMSEL";
        _image = 0;
        update();
    }

    Q_INVOKABLE void mccChange(QString mcc) {
        if (mcc != _mcc) {
            _mcc = mcc;
            update();
        }
    }

    Q_INVOKABLE void mncChange(QString mnc){
        if (mnc != _mnc) {
            _mnc = mnc;
            update();
        }
    }

public slots:
    void update() {
        // Make sure the search is still relevant when the results come in
        QString mcc = _mcc;
        QString mnc = _mnc;

        QNetworkRequest request(QString("http://appworld.blackberry.com/ClientAPI/checkcarrier?homemcc=%1&homemnc=%2&devicevendorid=-1&pin=0")
                                .arg(_mcc)
                                .arg(_mnc));
        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "AppWorld/4.4.0.0");
        QNetworkReply* reply = _manager->get(request);
        connect(reply, &QNetworkReply::finished, [=] {
            int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (reply->size() > 0 && (status == 200 || (status > 300 && status <= 308))) {
                if (mcc != _mcc || mnc != _mnc)
                    return;
                QXmlStreamReader xml(reply->readAll());
                while(!xml.atEnd() && !xml.hasError()) {
                    if(xml.tokenType() == QXmlStreamReader::StartElement) {
                        if (xml.name() == "country") {
                            _country = xml.attributes().value("name").toString();
                        } else if (xml.name() == "carrier") {
                            _carrier = xml.attributes().value("name").toString().split(" ").first();
                            _image = xml.attributes().value("icon").toInt();
                            if (_carrier == "default") {
                                _carrier = "";
                                _image = 0;
                            }
                        }
                    }
                    xml.readNext();
                }
            }
            emit resultChanged();
            reply->deleteLater();
        });
        connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), [=]() {
            _country = "";
            _carrier = "";
            _image = 0;
            reply->deleteLater();
        });
    }

signals:
    void resultChanged();
private:
    QString _country, _carrier;
    QString _mcc, _mnc;
    int _image;
    QNetworkAccessManager* _manager;
    QTimer* _timer;
};
