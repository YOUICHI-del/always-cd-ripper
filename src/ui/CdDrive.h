#pragma once
#include <QString>
#include <QVector>
#include <QFile>
#include <windows.h>

struct TrackInfo {
    int     number;
    QString title;
    DWORD   startSector;   // LBA
    DWORD   endSector;     // LBA
    int     durationSec;
    QString binFile;       // bin/cueモード時のbinファイルパス
};

struct DiscInfo {
    QString            discId;
    QVector<TrackInfo> tracks;
    DWORD              totalSectors;
    bool               isCueMode = false;
};

enum class DriveMode {
    Physical,   // 物理CDドライブ
    CueBin      // bin/cueファイル
};

class CdDrive
{
public:
    CdDrive();
    ~CdDrive();

    // ── 物理CDドライブ ──
    static QStringList availableDrives();
    bool open(const QString &driveLetter);

    // ── bin/cueファイル ──
    bool openCue(const QString &cuePath);

    void    close();
    bool    isOpen() const;
    QString sourceName() const { return m_sourceName; }
    DriveMode mode() const { return m_mode; }

    // TOC読み取り（物理 / cue 両対応）
    DiscInfo readToc();

    // セクタ単位で生PCMを読む（2352バイト/セクタ）
    QByteArray readSectorRaw(DWORD lba);

    int  sampleOffset() const { return m_sampleOffset; }
    void setSampleOffset(int v) { m_sampleOffset = v; }

private:
    // 物理ドライブ
    HANDLE    m_handle       = INVALID_HANDLE_VALUE;

    // bin/cueモード
    DiscInfo  m_cueDisc;
    QFile     m_binFile;

    // 共通
    DriveMode m_mode         = DriveMode::Physical;
    QString   m_sourceName;
    int       m_sampleOffset = 0;

    // cue解析
    DiscInfo  parseCue(const QString &cuePath);
    QByteArray readSectorFromBin(DWORD lba);
    QByteArray readSectorFromDrive(DWORD lba);
};
