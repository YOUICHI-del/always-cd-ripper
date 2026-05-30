#include "RipWorker.h"
#include "../metadata/TagWriter.h"
#include "../metadata/CoverArt.h"
#include "../metadata/MusicBrainz.h"
#include "../metadata/GnuDb.h"
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QDebug>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

RipWorker::RipWorker(QObject *parent)
    : QObject(parent)
{}

void RipWorker::setup(CdDrive *drive, const DiscInfo &disc,
                      const QString &outDir, const QString &artist,
                      const QString &album, const QString &year,
                      const QPixmap &coverArt, const QString &discogsToken,
                      int discNumber, int totalDiscs, int readSpeedKBps)
{
    m_drive         = drive;
    m_disc          = disc;
    m_outDir        = outDir;
    m_artist        = artist;
    m_album         = album;
    m_year          = year;
    m_coverArt      = coverArt;
    m_discogsToken  = discogsToken;
    m_discNumber    = discNumber;
    m_totalDiscs    = totalDiscs;
    m_readSpeedKBps = readSpeedKBps;
}

// WAVヘッダ書き込み
static void writeWavHeader(QFile &f, quint32 dataSize)
{
    quint32 sampleRate    = 44100;
    quint16 numChannels   = 2;
    quint16 bitsPerSample = 16;
    quint32 byteRate      = sampleRate * numChannels * bitsPerSample / 8;
    quint16 blockAlign    = numChannels * bitsPerSample / 8;
    quint32 riffSize      = 36 + dataSize;

    f.write("RIFF");
    f.write(reinterpret_cast<const char*>(&riffSize), 4);
    f.write("WAVE");
    f.write("fmt ");
    quint32 fmtSize = 16;
    f.write(reinterpret_cast<const char*>(&fmtSize), 4);
    quint16 audioFmt = 1;
    f.write(reinterpret_cast<const char*>(&audioFmt), 2);
    f.write(reinterpret_cast<const char*>(&numChannels), 2);
    f.write(reinterpret_cast<const char*>(&sampleRate), 4);
    f.write(reinterpret_cast<const char*>(&byteRate), 4);
    f.write(reinterpret_cast<const char*>(&blockAlign), 2);
    f.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
    f.write("data");
    f.write(reinterpret_cast<const char*>(&dataSize), 4);
}

// セクタ数をMM:SS:FF形式に変換
static QString sectorToMsf(DWORD sector)
{
    int frames  = sector % 75;
    int seconds = (sector / 75) % 60;
    int minutes = sector / 75 / 60;
    return QString("%1:%2:%3")
        .arg(minutes,  2, 10, QChar('0'))
        .arg(seconds,  2, 10, QChar('0'))
        .arg(frames,   2, 10, QChar('0'));
}

void RipWorker::run()
{
    if (!m_drive || m_disc.tracks.isEmpty()) {
        emit error("No drive or tracks");
        emit finished();
        return;
    }

    // ── USB最適化・開始 ──────────────────────────────────
    // タイマー精度を1msに設定（USBパケット揺らぎを平滑化）
    timeBeginPeriod(1);
    // スレッド優先度を上げる（他プロセスのUSB割り込みに負けにくくする）
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    // プロセス優先度を上げる
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    // USBセレクティブサスペンドを無効化（省電力によるUSB電源断を防ぐ）
    EXECUTION_STATE prevExecState = SetThreadExecutionState(
        ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);
    // リッピング速度設定（物理CDのみ・CUEモードは不要）
    if (!m_disc.isCueMode) {
        m_drive->setReadSpeed(m_readSpeedKBps);
        qDebug() << "[RipWorker] Read speed set to" << m_readSpeedKBps << "KB/s";
    }
    // ────────────────────────────────────────────────────

    QDir().mkpath(m_outDir);

    // ── メタデータ自動取得（物理CDモードのみ）──
    if (!m_disc.isCueMode) {
        emit trackStarted(0, m_disc.tracks.size(), "Fetching metadata...");

        // ① MusicBrainz
        MbRelease mb = MusicBrainz::lookup(m_disc);
        if (!mb.title.isEmpty()) {
            qDebug() << "[RipWorker] MusicBrainz OK:" << mb.title;
            if (m_artist.isEmpty()) m_artist = mb.artist;
            if (m_album.isEmpty())  m_album  = mb.title;
            if (m_year.isEmpty())   m_year   = mb.date;

            for (int i = 0; i < m_disc.tracks.size(); ++i) {
                if (m_disc.tracks[i].title.isEmpty() && i < mb.tracks.size())
                    m_disc.tracks[i].title = mb.tracks[i].title;
            }

            if (m_coverArt.isNull() && !mb.mbid.isEmpty()) {
                QString artUrl = QString(
                    "https://coverartarchive.org/release/%1/front-500")
                    .arg(mb.mbid);
                QByteArray imgData = MusicBrainz::httpGet(artUrl);
                if (!imgData.isEmpty())
                    m_coverArt.loadFromData(imgData);
            }
        }

        // ② GnuDB フォールバック
        if (m_album.isEmpty()) {
            qDebug() << "[RipWorker] MusicBrainz not found, trying GnuDB";
            GnuDbResult gnu = GnuDb::lookup(m_disc);
            if (gnu.isValid()) {
                qDebug() << "[RipWorker] GnuDB OK:" << gnu.album;
                if (m_artist.isEmpty()) m_artist = gnu.artist;
                if (m_album.isEmpty())  m_album  = gnu.album;
                if (m_year.isEmpty())   m_year   = gnu.year;

                for (int i = 0; i < m_disc.tracks.size(); ++i) {
                    if (m_disc.tracks[i].title.isEmpty() && i < gnu.tracks.size())
                        m_disc.tracks[i].title = gnu.tracks[i].title;
                }
            }
        }

        // ③ アートワークがまだなければ CoverArt で検索
        if (m_coverArt.isNull() && !m_album.isEmpty()) {
            qDebug() << "[RipWorker] Fetching cover art via CoverArt";
            CoverArt ca;
            if (!m_discogsToken.isEmpty())
                ca.setDiscogsToken(m_discogsToken);
            m_coverArt = ca.fetch(m_artist, m_album);
        }

        emit metadataReady(m_artist, m_album, m_year, m_coverArt, m_disc.tracks);
    }

    if (m_disc.isCueMode) {
        runCueMode();
    } else {
        runPhysicalCD();
    }

    // ── USB最適化・終了（元に戻す）────────────────────────
    timeEndPeriod(1);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
    // USBセレクティブサスペンドを復元
    SetThreadExecutionState(prevExecState);
    // ────────────────────────────────────────────────────

    emit finished();
}

void RipWorker::runCueMode()
{
    QPixmap coverArt = m_coverArt;

    if (!coverArt.isNull()) {
        QString coverPath = m_outDir + "/cover.jpg";
        coverArt.save(coverPath, "JPEG", 90);
        emit trackStarted(0, m_disc.tracks.size(), "Cover art: OK");
    } else {
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
    reader.setSectorCallback([this](const SectorResult &sr) {
        emit sectorDone(sr);
    });

    int total = m_disc.tracks.size();
    for (int i = 0; i < total; ++i) {
        const TrackInfo &track = m_disc.tracks[i];
        QString title = track.title.isEmpty()
            ? QString("Track %1").arg(track.number) : track.title;

        emit trackStarted(i + 1, total, title);

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

        int errors = 0, retries = 0;
        for (const auto &sr : reader.lastResults()) {
            if (sr.hasError) errors++;
            retries += sr.retries;
        }

        QString fileName = QString("%1. %2.flac")
            .arg(track.number, 2, 10, QChar('0'))
            .arg(title);
        fileName.remove(QRegularExpression(R"([\\/:*?"<>|])"));
        QString flacPath = m_outDir + "/" + fileName;

        FlacEncoder encoder;
        bool ok = encoder.encode(pcm, flacPath);

        if (ok) {
            TrackTag tag;
            tag.title       = title;
            tag.artist      = m_artist;
            tag.album       = m_album;
            tag.date        = m_year;
            tag.trackNumber = track.number;
            tag.totalTracks = m_disc.tracks.size();
            tag.discNumber  = m_discNumber;
            tag.totalDiscs  = m_totalDiscs;
            tag.coverArt    = coverArt;
            TagWriter::write(flacPath, tag);
        }

        result.success = ok;
        result.errors  = errors;
        result.retries = retries;
        emit trackDone(result);
    }
}

// ────────────────────────────────────────────────────────────
//  runPhysicalCD
//  元の実装（WAV→CUE→FLAC）+ USB最適化・速度制御
// ────────────────────────────────────────────────────────────
void RipWorker::runPhysicalCD()
{
    int total = m_disc.tracks.size();

    emit trackStarted(0, total, "Reading CD...");

    QString wavPath = m_outDir + "/disc.wav";
    QString cuePath = m_outDir + "/disc.cue";

    QFile wavFile(wavPath);
    if (!wavFile.open(QIODevice::WriteOnly)) {
        emit error("WAVファイルを開けません: " + wavPath);
        return;
    }

    DWORD firstSector = m_disc.tracks.first().startSector;
    DWORD lastSector  = m_disc.tracks.last().endSector;  // 修正: totalSectors+firstSector-1 は範囲超過バグ
    quint32 dataSize  = (lastSector - firstSector + 1) * 2352;
    writeWavHeader(wavFile, dataSize);

    // WAV読み取りループ：セクタ単位でエラーを検出し sectorDone を emit する
    // トラック単位のエラー数も集計する
    m_physicalErrors  = 0;
    m_physicalRetries = 0;

    // トラック別エラー集計テーブル（後でtrackDoneに使う）
    QVector<int> trackErrors(m_disc.tracks.size(), 0);
    QVector<int> trackRetries(m_disc.tracks.size(), 0);

    for (DWORD lba = firstSector; lba <= lastSector; ++lba) {
        QByteArray sector = m_drive->readSectorRaw(lba);

        // ── キープアライブ処理 ────────────────────────────────
        // 100セクタごとにTOCを軽く読んでUSBドライバーのタイムアウトを防ぐ
        // fre:acと同様の「定期的な軽いコマンド送信」でUSB接続を維持する
        if ((lba - firstSector) % 100 == 0 && lba > firstSector) {
            m_drive->readToc();  // 軽いコマンドでUSB接続を維持
        }

        // ── トラック境界での小休止 ───────────────────────────
        // トラックが変わるタイミングでドライブを少し休ませる
        // これによりレーザーの熱を逃がし、次のトラックの読み取り精度が上がる
        for (int ti = 0; ti < m_disc.tracks.size(); ++ti) {
            if (lba == m_disc.tracks[ti].startSector && lba > firstSector) {
                Sleep(500);  // トラック境界で500ms休憩
                break;
            }
        }
        // ────────────────────────────────────────────────────

        SectorResult sr;
        sr.lba        = lba;
        sr.hasError   = sector.isEmpty();
        sr.speedKBps  = m_drive->lastReadSpeedKBps();
        // 速度からretriesを推定（SectorMapの色分けに使用）
        // 設定速度の半分以下なら重度、75%以下なら軽度と判定
        if (sr.hasError) {
            sr.retries = 99;
            sr.perfect = false;
        } else {
            int setSpeed = m_readSpeedKBps > 0 ? m_readSpeedKBps : 300;
            if      (sr.speedKBps < setSpeed * 25 / 100) sr.retries = 8; // オレンジ
            else if (sr.speedKBps < setSpeed * 60 / 100) sr.retries = 2; // 暗い青
            else                                          sr.retries = 0; // 明るい青
            sr.perfect = (sr.retries == 0);
        }

        if (sr.hasError)  m_physicalErrors++;
        if (!sr.perfect)  m_physicalRetries += sr.retries;

        // どのトラックに属するか判定
        for (int ti = 0; ti < m_disc.tracks.size(); ++ti) {
            DWORD tStart = m_disc.tracks[ti].startSector;
            DWORD tEnd   = (ti + 1 < m_disc.tracks.size())
                ? m_disc.tracks[ti + 1].startSector - 1
                : lastSector;
            if (lba >= tStart && lba <= tEnd) {
                if (sr.hasError)  trackErrors[ti]++;
                if (!sr.perfect)  trackRetries[ti] += sr.retries;
                break;
            }
        }

        emit sectorDone(sr);

        if (sector.isEmpty())
            sector = QByteArray(2352, '\0');
        wavFile.write(sector);
    }
    wavFile.close();

    // CUEシートを生成
    QFile cueFile(cuePath);
    if (cueFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream cue(&cueFile);
        cue.setEncoding(QStringConverter::Utf8);
        cue << "PERFORMER \"" << m_artist << "\"\n";
        cue << "TITLE \""     << m_album  << "\"\n";
        cue << "FILE \"disc.wav\" WAVE\n";

        DWORD offset = m_disc.tracks.first().startSector;
        for (const TrackInfo &t : m_disc.tracks) {
            QString title = t.title.isEmpty()
                ? QString("Track %1").arg(t.number) : t.title;
            cue << QString("  TRACK %1 AUDIO\n").arg(t.number, 2, 10, QChar('0'));
            cue << "    TITLE \""     << title    << "\"\n";
            cue << "    PERFORMER \"" << m_artist << "\"\n";
            DWORD relSector = t.startSector - offset;
            cue << "    INDEX 01 " << sectorToMsf(relSector) << "\n";
        }
        cueFile.close();
    }

    emit trackStarted(0, total, "Converting to FLAC...");

    QPixmap coverArt = m_coverArt;
    if (!coverArt.isNull())
        coverArt.save(m_outDir + "/cover.jpg", "JPEG", 90);

    // 修正: openCueでドライブをCUEモードに切り替えてからreadTrackする
    // parseCueのみでは m_drive が物理CDモードのままで読み取り先がWAVにならない
    m_drive->openCue(cuePath);
    DiscInfo cueDisc = m_drive->readToc();
    for (int i = 0; i < cueDisc.tracks.size() && i < m_disc.tracks.size(); ++i) {
        if (!m_disc.tracks[i].title.isEmpty())
            cueDisc.tracks[i].title = m_disc.tracks[i].title;
    }

    ParanoiaReader reader;
    // CUEモードで読むので forcePhysical は不要（WAVファイルから直接読む）
    reader.setForcePhysicalMode(false);
    reader.setSectorCallback([this](const SectorResult &sr) {
        // Step3（FLAC変換）ではsectorDoneを送らない
        // Purityは既にStep1（WAV読み取り）で集計済み
        Q_UNUSED(sr)
    });

    for (int i = 0; i < cueDisc.tracks.size(); ++i) {
        const TrackInfo &track = cueDisc.tracks[i];
        QString title = track.title.isEmpty()
            ? QString("Track %1").arg(track.number) : track.title;

        emit trackStarted(i + 1, total, title);

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

        QString fileName = QString("%1. %2.flac")
            .arg(track.number, 2, 10, QChar('0'))
            .arg(title);
        fileName.remove(QRegularExpression(R"([\\/:*?"<>|])"));
        QString flacPath = m_outDir + "/" + fileName;

        FlacEncoder encoder;
        bool ok = encoder.encode(pcm, flacPath);

        if (ok) {
            TrackTag tag;
            tag.title       = title;
            tag.artist      = m_artist;
            tag.album       = m_album;
            tag.date        = m_year;
            tag.trackNumber = track.number;
            tag.totalTracks = cueDisc.tracks.size();
            tag.discNumber  = m_discNumber;
            tag.totalDiscs  = m_totalDiscs;
            tag.coverArt    = coverArt;
            TagWriter::write(flacPath, tag);
        }

        // Step1（WAV読み取り）で集計したトラック別エラー数を設定
        result.success = ok;
        result.errors  = (i < trackErrors.size())  ? trackErrors[i]  : 0;
        result.retries = (i < trackRetries.size()) ? trackRetries[i] : 0;
        emit trackDone(result);
    }

    // disc.wav を削除
    QFile::remove(wavPath);
}
