#pragma once
#include <QObject>
#include <QVector>
#include <QNetworkAccessManager>

struct MbRelease {
    QString id;
    QString title;
    QString artist;
    QString date;
    QString label;
    QVector<QString> trackTitles;
};

class MusicBrainz : public QObject
{
    Q_OBJECT
public:
    explicit MusicBrainz(QObject *parent = nullptr);

    // Disc ID から候補リリースを非同期で取得
    void lookup(const QString &discId);

signals:
    void resultsReady(QVector<MbRelease> releases);
    void error(const QString &message);

private slots:
    void onReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_nam;
    // User-Agent は MusicBrainz の API ポリシー上必須
    static constexpr const char *USER_AGENT =
        "AlwaysCDRipper/1.0.0 ( https://github.com/YOUICHI-del )";
};
