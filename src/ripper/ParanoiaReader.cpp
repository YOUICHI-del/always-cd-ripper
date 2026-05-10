#include "ParanoiaReader.h"
#include <QDebug>
#include <QMap>

static constexpr int CD_RAW_SECTOR_SIZE = 2352;

ParanoiaReader::ParanoiaReader(QObject *parent)
    : QObject(parent)
{}

// セクタを複数回読んで最も頻出するデータを「正解」とする
QByteArray ParanoiaReader::readSectorWithVerify(
    CdDrive &drive, DWORD lba, SectorResult &out)
{
    out.lba      = lba;
    out.retries  = 0;
    out.hasError = false;
    out.perfect  = false;

    // 読み取り結果の投票マップ: データ → 出現回数
    QMap<QByteArray, int> votes;
    QByteArray first = drive.readSectorRaw(lba);

    if (first.isEmpty()) {
        // 1回目から失敗
        out.hasError = true;
        return QByteArray(CD_RAW_SECTOR_SIZE, '\0');
    }
    votes[first]++;

    // 2回目を読んで1回目と一致すれば即 perfect
    QByteArray second = drive.readSectorRaw(lba);
    if (!second.isEmpty() && second == first) {
        out.perfect = true;
        return first;
    }
    if (!second.isEmpty()) votes[second]++;
    out.retries++;

    // 不一致 → 最大 m_maxRetries 回まで再試行
    for (int i = 2; i < m_maxRetries && !m_abort; ++i) {
        QByteArray buf = drive.readSectorRaw(lba);
        if (buf.isEmpty()) {
            out.retries++;
            continue;
        }
        votes[buf]++;
        out.retries++;

        // 過半数一致で確定
        for (auto it = votes.begin(); it != votes.end(); ++it) {
            if (it.value() >= 3) {
                return it.key();
            }
        }
    }

    // 最終的に最多得票を採用
    QByteArray best;
    int bestCount = 0;
    for (auto it = votes.begin(); it != votes.end(); ++it) {
        if (it.value() > bestCount) {
            bestCount = it.value();
            best      = it.key();
        }
    }

    if (bestCount < 2) {
        out.hasError = true;
    }

    return best.isEmpty() ? QByteArray(CD_RAW_SECTOR_SIZE, '\0') : best;
}

QVector<qint16> ParanoiaReader::readTrack(CdDrive &drive, const TrackInfo &track)
{
    m_abort = false;
    m_results.clear();

    int sectorsTotal = static_cast<int>(track.endSector - track.startSector + 1);
    int samplesPer  = CD_RAW_SECTOR_SIZE / sizeof(qint16);

    QVector<qint16> pcm;
    pcm.reserve(sectorsTotal * samplesPer);

    for (DWORD lba = track.startSector;
         lba <= track.endSector && !m_abort; ++lba)
    {
        SectorResult sr;
        QByteArray data = readSectorWithVerify(drive, lba, sr);
        m_results.append(sr);

        // バイト列を qint16 に変換（リトルエンディアン）
        const qint16 *samples = reinterpret_cast<const qint16 *>(data.constData());
        for (int i = 0; i < samplesPer; ++i)
            pcm.append(samples[i]);

        if (m_sectorCb)   m_sectorCb(sr);
        if (m_progressCb) m_progressCb(
            track.number,
            static_cast<int>(lba - track.startSector + 1),
            sectorsTotal);
    }

    return m_abort ? QVector<qint16>() : pcm;
}
