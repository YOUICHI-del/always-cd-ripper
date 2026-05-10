#include "CoverArt.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDebug>

CoverArt::CoverArt(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{}

void CoverArt::fetch(const QString &artist, const QString &album)
{
    QPixmap pix = fetchSync(artist, album);
    if (!pix.isNull())
        emit imageReady(pix);
    else
        emit error("Cover art not found");
}

static QByteArray syncGet(QNetworkAccessManager *nam, const QUrl &url)
{
    QNetworkRequest req;
    req.setUrl(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = nam->get(req);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished,
                     &loop,  &QEventLoop::quit);
    loop.exec();

    QByteArray data;
    if (reply->error() == QNetworkReply::NoError)
        data = reply->readAll();
    else
        qWarning() << "HTTP error:" << reply->errorString();

    reply->deleteLater();
    return data;
}

QString CoverArt::searchITunes(const QString &artist, const QString &album)
{
    QString term = QString(QUrl::toPercentEncoding(artist + " " + album));
    QUrl url(QString("https://itunes.apple.com/search"
                     "?term=%1&entity=album&limit=10&country=JP").arg(term));

    QByteArray data = syncGet(m_nam, url);
    if (data.isEmpty()) return {};

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray results = doc.object().value("results").toArray();

    for (const QJsonValue &rv : results) {
        QJsonObject obj = rv.toObject();
        QString colName = obj.value("collectionName").toString();
        QString artUrl  = obj.value("artworkUrl100").toString();
        if (!artUrl.isEmpty() &&
            (colName.contains(album, Qt::CaseInsensitive) ||
             album.contains(colName, Qt::CaseInsensitive))) {
            artUrl.replace("100x100bb", "600x600bb");
            return artUrl;
        }
    }

    if (!results.isEmpty()) {
        QString artUrl = results[0].toObject()
                         .value("artworkUrl100").toString();
        if (!artUrl.isEmpty()) {
            artUrl.replace("100x100bb", "600x600bb");
            return artUrl;
        }
    }

    // アーティスト名だけで再試行
    QString term2 = QString(QUrl::toPercentEncoding(artist));
    QUrl url2(QString("https://itunes.apple.com/search"
                      "?term=%1&entity=album&limit=5&country=JP").arg(term2));

    data = syncGet(m_nam, url2);
    if (data.isEmpty()) return {};

    doc     = QJsonDocument::fromJson(data);
    results = doc.object().value("results").toArray();
    if (!results.isEmpty()) {
        QString artUrl = results[0].toObject()
                         .value("artworkUrl100").toString();
        artUrl.replace("100x100bb", "600x600bb");
        return artUrl;
    }

    return {};
}

QPixmap CoverArt::fetchSync(const QString &artist, const QString &album)
{
    qDebug() << "CoverArt: iTunes search" << artist << album;

    QString artUrl = searchITunes(artist, album);
    if (artUrl.isEmpty()) {
        qWarning() << "CoverArt: not found on iTunes";
        return {};
    }

    qDebug() << "CoverArt: downloading" << artUrl;
    QByteArray imgData = syncGet(m_nam, QUrl(artUrl));
    if (imgData.isEmpty()) return {};

    QPixmap pix;
    pix.loadFromData(imgData);
    qDebug() << "CoverArt: success" << pix.size();
    return pix;
}
