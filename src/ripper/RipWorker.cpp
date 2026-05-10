#include "RipWorker.h"
#include "../metadata/TagWriter.h"
#include "../metadata/CoverArt.h"
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QDebug>

RipWorker::RipWorker(QObject *parent)
    : QObject(parent)
{}

void RipWorker::setup(CdDrive *drive, const DiscInfo &disc,
                      const QString &outDir, const QString &artist,
                      const QString &album, const QString &year)
{
    m_drive  = drive;
    m_disc   = disc;
    m_outDir = outDir;
    m_artist = artist;
    m_album  = album;
    m_year   = year;
}

void RipWorker::run()
{
    if (!m_drive || m_disc.tracks.isEmpty()) {
        emit error("No drive or tracks");
        emit finished();
        return;
    }

    // 出力フォルダ作成
    QDir().mkpath(m_outDir);

    // ── アルバムアートを先に取得 ──
    QPixmap coverArt;
    CoverArt coverFetcher;
    emit trackStarted(0, m_disc.tracks.size(), "Fetching cover art...");

    coverArt = coverFetcher.fetchSync(m_artist, m_album);

    // cover.jpg として保存
    if (!coverArt.isNull()) {
        QString coverPath = m_outDir + "/cover.jpg";
        coverArt.save(coverPath, "JPEG", 90);
        emit trackStarted(0, m_disc.tracks.size(), "Cover art: OK");
    } else {
        // ネット取得失敗 → 同フォルダのcover.jpgを探す
        QString localCover = QFileInfo(m_disc.tracks.first().binFile)
            .absolutePath() + "/cover.jpg";
        if (QFile::exists(localCover)) {
            coverArt.load(localCover);
            if (!coverArt.isNull())
                QFile::copy(localCover, m_outDir + "/cover.jpg");
        }
        emit trackStarted(0, m_disc.tracks.size(), "Cover art: not found");
    }

    ParanoiaReader reader;

    // セクタコールバック → シグナルで転送
    reader.setSectorCallback([this](const SectorResult &sr) {
        emit sectorDone(sr);
    });

    int total = m_disc.tracks.size();
    for (int i = 0; i < total; ++i) {
        const TrackInfo &track = m_disc.tracks[i];
        QString title = track.title.isEmpty()
            ? QString("Track %1").arg(track.number) : track.title;

        emit trackStarted(i + 1, total, title);

        // セクタ読み取り
        QVector<qint16> pcm = reader.readTrack(*m_drive, track);

        RipResult result;
        result.trackNumber = track.number;
        result.title       = title;

        if (pcm.isEmpty()) {
            result.success = false;
            result.errors  = -1;
            result.retries = 0;
            emit trackDone(result);
            continue;
        }

        // エラー集計
        int errors  = 0;
        int retries = 0;
        for (auto &sr : reader.lastResults()) {
            if (sr.hasError) errors++;
            retries += sr.retries;
        }

        // FLACファイル名生成
        QString fileName = QString("%1. %2.flac")
            .arg(track.number, 2, 10, QChar('0'))
            .arg(title);
        fileName.remove(QRegularExpression(R"([\\/:*?"<>|])"));
        QString flacPath = m_outDir + "/" + fileName;

        FlacEncoder encoder;
        bool ok = encoder.encode(pcm, flacPath);

        if (ok) {
            // タグ書き込み
            TrackTag tag;
            tag.title       = title;
            tag.artist      = m_artist;
            tag.album       = m_album;
            tag.date        = m_year;
            tag.trackNumber = track.number;
            tag.totalTracks = m_disc.tracks.size();
            tag.coverArt    = coverArt;
            TagWriter::write(flacPath, tag);
        }

        result.success = ok;
        result.errors  = errors;
        result.retries = retries;
        emit trackDone(result);
    }

    emit finished();
}
