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
    out.lba       = lba;
    out.retries   = 0;
    out.hasError  = false;
    out.perfect   = false;
    out.speedKBps = 0;

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
    // キャッシュ掃き出し: 再読み取り前に隣接セクタを読んでキャッシュを無効化
    for (int i = 2; i < m_maxRetries && !m_abort; ++i) {
        // 隣接セクタ（lba+1）を読んでドライブキャッシュを追い出す
        // 読み取り結果は使わない（キャッシュフラッシュ目的のみ）
        DWORD flushLba = (lba > 0) ? lba - 1 : lba + 1;
        drive.readSectorRaw(flushLba);

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

    // 物理CDモード強制フラグが立っている場合は投票方式を使う
    // （runPhysicalCD Step3でCUEトラック情報を使う場合に必要）
    bool isCueMode = !m_forcePhysical && (track.isWave || !track.binFile.isEmpty());

    for (DWORD lba = track.startSector;
         lba <= track.endSector && !m_abort; ++lba)
    {
        SectorResult sr;
        QByteArray data;

        if (isCueMode) {
            // CUEモード: 直接1回読み取り（常にperfect）
            data = drive.readSectorRaw(lba);
            sr.lba        = lba;
            sr.retries    = 0;
            sr.hasError   = data.isEmpty();
            sr.perfect    = !data.isEmpty();
            sr.speedKBps  = 0;
            if (data.isEmpty())
                data = QByteArray(CD_RAW_SECTOR_SIZE, '\0');
        } else {
            // 物理CDモード: 投票方式
            data = readSectorWithVerify(drive, lba, sr);
        }

        m_results.append(sr);

        const qint16 *samples = reinterpret_cast<const qint16 *>(data.constData());
        for (int i = 0; i < samplesPer; ++i)
            pcm.append(samples[i]);

        if (m_sectorCb)   m_sectorCb(sr);
        if (m_progressCb) m_progressCb(
            track.number,
            static_cast<int>(lba - track.startSector + 1),
            sectorsTotal);
    }

    if (m_abort) return QVector<qint16>();

    // ── サンプルオフセット補正 ──────────────────────────────
    // ドライブごとの読み取りズレを補正する
    // 正値: 先頭をその分だけ削除して末尾に無音を追加
    // 負値: 先頭に無音を追加して末尾をその分だけ削除
    int offset = drive.sampleOffset();
    if (offset != 0) {
        if (offset > 0) {
            // 先頭offset個を削除、末尾にoffset個の無音を追加
            if (offset < pcm.size()) {
                pcm.remove(0, offset);
                pcm.append(QVector<qint16>(offset, 0));
            }
        } else {
            // 先頭に|offset|個の無音を追加、末尾|offset|個を削除
            int abs_offset = -offset;
            if (abs_offset < pcm.size()) {
                pcm = QVector<qint16>(abs_offset, 0) + pcm;
                pcm.resize(pcm.size() - abs_offset);
            }
        }
    }
    // ────────────────────────────────────────────────────────

    return pcm;
}
