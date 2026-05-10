#pragma once
#include <QObject>
#include <QString>
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
               const QString &album, const QString &year);

public slots:
    void run();

signals:
    void sectorDone(SectorResult sr);
    void trackStarted(int trackNum, int totalTracks, QString title);
    void trackDone(RipResult result);
    void finished();
    void error(QString message);

private:
    CdDrive     *m_drive  = nullptr;
    DiscInfo     m_disc;
    QString      m_outDir;
    QString      m_artist;
    QString      m_album;
    QString      m_year;
};
