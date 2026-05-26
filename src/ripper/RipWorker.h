#pragma once
#include <QObject>
#include <QString>
#include <QPixmap>
#include <QFileInfo>
#include "../ripper/CdDrive.h"
#include "../ripper/ParanoiaReader.h"
#include "../ripper/FlacEncoder.h"

struct RipResult {
    int     trackNumber;
    QString title;
    bool    success;
    int     errors;
    int     retries;
};

class RipWorker : public QObject
{
    Q_OBJECT
public:
    explicit RipWorker(QObject *parent = nullptr);

    void setup(CdDrive *drive, const DiscInfo &disc,
               const QString &outDir, const QString &artist,
               const QString &album, const QString &year,
               const QPixmap &coverArt = QPixmap(),
               const QString &discogsToken = QString(),
               int discNumber = 0, int totalDiscs = 0,
               int readSpeedKBps = CdDrive::SPEED_2X);

public slots:
    void run();

signals:
    void sectorDone(SectorResult sr);
    void trackStarted(int trackNum, int totalTracks, QString title);
    void trackDone(RipResult result);
    void metadataReady(QString artist, QString album, QString year,
                       QPixmap coverArt, QVector<TrackInfo> tracks);
    void finished();
    void error(QString message);

private:
    void runCueMode();
    void runPhysicalCD();

    CdDrive     *m_drive  = nullptr;
    DiscInfo     m_disc;
    QString      m_outDir;
    QString      m_artist;
    QString      m_album;
    QString      m_year;
    QPixmap      m_coverArt;
    QString      m_discogsToken;
    int          m_discNumber      = 0;
    int          m_totalDiscs      = 0;
    int          m_readSpeedKBps   = CdDrive::SPEED_2X;

    // 物理CDモード: WAV読み取り時の累積エラー集計
    int          m_physicalErrors  = 0;
    int          m_physicalRetries = 0;
};
