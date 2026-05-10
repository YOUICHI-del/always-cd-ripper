#pragma once
#include <QString>
#include <QPixmap>

struct TrackTag {
    QString title;
    QString artist;
    QString album;
    QString date;
    QString label;
    int     trackNumber = 0;
    int     totalTracks = 0;
    QPixmap coverArt;
};

class TagWriter
{
public:
    // FLAC ファイルにタグを書き込む（TagLib使用）
    static bool write(const QString &flacPath, const TrackTag &tag);
    static QString lastError();
};
