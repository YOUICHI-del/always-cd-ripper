#pragma once
#include <QString>
#include <QVector>
#include "../ripper/CdDrive.h"

struct MbTrack {
    int     number;
    QString title;
    int     discNumber = 1;
};

struct MbDisc {
    int              position;
    QVector<MbTrack> tracks;
};

struct MbRelease {
    QString          title;
    QString          artist;
    QString          date;
    QString          mbid;
    int              totalDiscs = 1;
    QVector<MbDisc>  discs;

    // 後方互換：disc1のトラックを返す
    QVector<MbTrack> tracks;
};

class MusicBrainz
{
public:
    static MbRelease  lookup(const DiscInfo &disc);
    static QString    calcDiscId(const DiscInfo &disc);
    static QByteArray httpGet(const QString &urlStr);

    // ★ 追加：アルバムアート取得
    static QByteArray fetchCoverArt(const QString &mbid);
};
