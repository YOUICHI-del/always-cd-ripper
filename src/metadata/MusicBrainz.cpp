#include "MusicBrainz.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDebug>

MusicBrainz::MusicBrainz(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    connect(m_nam, &QNetworkAccessManager::finished,
            this,  &MusicBrainz::onReply);
}

void MusicBrainz::lookup(const QString &discId)
{
    QString url = QString(
        "https://musicbrainz.org/ws/2/discid/%1"
        "?inc=recordings+artist-credits+labels&fmt=json"
    ).arg(discId);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", USER_AGENT);
    m_nam->get(req);
}

void MusicBrainz::onReply(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        emit error("JSON parse error");
        return;
    }

    QVector<MbRelease> releases;
    QJsonArray releaseArray = doc.object().value("releases").toArray();

    for (const QJsonValue &rv : releaseArray) {
        QJsonObject ro = rv.toObject();
        MbRelease rel;
        rel.id     = ro["id"].toString();
        rel.title  = ro["title"].toString();
        rel.date   = ro["date"].toString();

        QJsonArray ac = ro["artist-credit"].toArray();
        if (!ac.isEmpty())
            rel.artist = ac[0].toObject()["artist"].toObject()["name"].toString();

        QJsonArray media = ro["media"].toArray();
        if (!media.isEmpty()) {
            QJsonArray tracks = media[0].toObject()["tracks"].toArray();
            for (const QJsonValue &tv : tracks)
                rel.trackTitles.append(tv.toObject()["title"].toString());
        }

        releases.append(rel);
    }

    emit resultsReady(releases);
}
