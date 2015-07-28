// The MIT License (MIT)
//
// Copyright (c) 2015 Alexander Nassian <picaschaf@me.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include "ifttt.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>


namespace {
const QString IFTTTRequestUrl("https://maker.ifttt.com/trigger/%1/with/key/%2");
}


class IFTTTPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(IFTTT)
    Q_DISABLE_COPY(IFTTTPrivate)

public:
    QNetworkAccessManager* qnam;
    QString secretKey;

    explicit IFTTTPrivate(IFTTT* parent = nullptr) :
        QObject(parent),
        qnam(nullptr),
        q_ptr(parent)
    {
        bool isConnected = false;
        Q_UNUSED(isConnected);

        qnam = new(std::nothrow) QNetworkAccessManager(this);
        Q_CHECK_PTR(qnam);

        isConnected = QObject::connect(
                    qnam,
                    SIGNAL(finished(QNetworkReply*)),
                    this,
                    SLOT(handleIFTTTResponse(QNetworkReply*)));
        Q_ASSERT(isConnected == true);

        isConnected = QObject::connect(
                    qnam,
                    SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
                    this,
                    SLOT(handleIFTTTSslErrors(QNetworkReply*,
                                              QList<QSslError>)));
        Q_ASSERT(isConnected == true);
    }

    virtual ~IFTTTPrivate()
    {
    }

public slots:
    void handleIFTTTResponse(QNetworkReply* reply)
    {
        reply->deleteLater();
    }

    void handleIFTTTSslErrors(QNetworkReply* reply,
                              const QList<QSslError>& errors)
    {
        //! \todo This should be done more nicely in future.
        reply->ignoreSslErrors(errors);
    }

private:
    IFTTT* q_ptr;
};

#include "ifttt.moc"


IFTTT::IFTTT(QObject* parent) :
    QObject(parent),
    d_ptr(new IFTTTPrivate(this))
{
}

IFTTT::~IFTTT()
{
}

void IFTTT::setSecretKey(const QString& key)
{
    Q_D(IFTTT);
    d->secretKey = key;
}

QString IFTTT::secretKey() const
{
    Q_D(const IFTTT);
    return d->secretKey;
}

void IFTTT::trigger(const QString& eventName,
                    const QString& value1,
                    const QString& value2,
                    const QString& value3)
{
    Q_D(IFTTT);

    bool doPostRequest = (!value1.isEmpty()
                          || !value2.isEmpty()
                          || !value3.isEmpty());

    QNetworkRequest request;
    request.setUrl(QUrl(::IFTTTRequestUrl.arg(eventName).arg(d->secretKey)));

    QByteArray postData;
    if (doPostRequest == true) {
        request.setRawHeader("Content-Type", "application/json");

        QVariantMap values;
        if (value1.isEmpty() == false)
            values["value1"] = value1;
        if (value2.isEmpty() == false)
            values["value2"] = value2;
        if (value3.isEmpty() == false)
            values["value3"] = value3;

        postData = QJsonDocument(
                    QJsonObject::fromVariantMap(values)).toJson();
    }

    d->qnam->post(request, postData);
}
