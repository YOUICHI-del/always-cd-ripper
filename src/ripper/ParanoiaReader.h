#pragma once
#include <QObject>
#include <QVector>
#include <functional>
#include "CdDrive.h"

// セクタ読み取り結果
struct SectorResult {
    DWORD   lba;
    int     retries;     // 再試行回数
    bool    hasError;    // 最終的にエラーあり
    bool    perfect;     // 1回で完全一致
    int     speedKBps;  // 読み取り速度（KB/s）
};

using SectorCallback   = std::function<void(const SectorResult &)>;
using ProgressCallback = std::function<void(int trackNum, int done, int total)>;

class ParanoiaReader : public QObject
{
    Q_OBJECT
public:
    explicit ParanoiaReader(QObject *parent = nullptr);

    void setSectorCallback(SectorCallback cb)    { m_sectorCb   = cb; }
    void setProgressCallback(ProgressCallback cb){ m_progressCb = cb; }

    // 最大再試行回数（デフォルト20）
    void setMaxRetries(int n) { m_maxRetries = n; }

    // 物理CDモードを強制（CUEトラック情報でも投票方式を使う）
    void setForcePhysicalMode(bool v) { m_forcePhysical = v; }

    // トラックを読み取り PCMデータを返す
    QVector<qint16> readTrack(CdDrive &drive, const TrackInfo &track);

    void abort() { m_abort = true; }

    QVector<SectorResult> lastResults() const { return m_results; }

private:
    // セクタを最大 maxRetries 回読み、一致したデータを返す
    // 一致判定: 複数回読んで同じバイト列が得られたら信頼とみなす
    QByteArray readSectorWithVerify(CdDrive &drive, DWORD lba, SectorResult &outResult);

    SectorCallback        m_sectorCb;
    ProgressCallback      m_progressCb;
    bool                  m_abort         = false;
    bool                  m_forcePhysical = false;
    int                   m_maxRetries = 20;
    QVector<SectorResult> m_results;
};
