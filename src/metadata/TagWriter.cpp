#include "TagWriter.h"
#include <taglib/flacfile.h>
#include <taglib/xiphcomment.h>
#include <taglib/flacpicture.h>
#include <taglib/tstring.h>
#include <QBuffer>
#include <QImageWriter>
#include <QDebug>

static QString s_lastError;

// QString → TagLib::String（UTF-8）
static TagLib::String toTL(const QString &s)
{
    return TagLib::String(s.toUtf8().constData(), TagLib::String::UTF8);
}

bool TagWriter::write(const QString &flacPath, const TrackTag &tag)
{
    // TagLib はワイド文字ファイル名に対応
#ifdef _WIN32
    TagLib::FLAC::File file(
        reinterpret_cast<const wchar_t*>(flacPath.utf16()));
#else
    TagLib::FLAC::File file(flacPath.toUtf8().constData());
#endif

    if (!file.isValid()) {
        s_lastError = "Cannot open FLAC file: " + flacPath;
        return false;
    }

    // Xiph コメント（FLAC標準タグ）
    TagLib::Ogg::XiphComment *xiph = file.xiphComment(true);
    if (!xiph) {
        s_lastError = "Cannot get XiphComment";
        return false;
    }

    xiph->setTitle(toTL(tag.title));
    xiph->setArtist(toTL(tag.artist));
    xiph->setAlbum(toTL(tag.album));
    xiph->setYear(tag.date.left(4).toUInt());
    xiph->setTrack(tag.trackNumber);

    if (!tag.label.isEmpty())
        xiph->addField("ORGANIZATION", toTL(tag.label));

    // トラック総数
    if (tag.totalTracks > 0)
        xiph->addField("TRACKTOTAL",
            toTL(QString::number(tag.totalTracks)));

    // アルバムアート
    if (!tag.coverArt.isNull()) {
        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        QImageWriter writer(&buf, "JPEG");
        writer.setQuality(90);
        writer.write(tag.coverArt.toImage());
        buf.close();

        TagLib::FLAC::Picture *pic = new TagLib::FLAC::Picture;
        pic->setType(TagLib::FLAC::Picture::FrontCover);
        pic->setMimeType("image/jpeg");
        pic->setDescription("Cover");
        pic->setData(TagLib::ByteVector(
            buf.data().constData(), buf.data().size()));
        file.addPicture(pic);
    }

    if (!file.save()) {
        s_lastError = "Failed to save tags: " + flacPath;
        return false;
    }

    return true;
}

QString TagWriter::lastError()
{
    return s_lastError;
}

