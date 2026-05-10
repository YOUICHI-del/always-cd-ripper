#include "CdDrive.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <ntddcdrm.h>

static constexpr int CD_RAW_SECTOR_SIZE = 2352;

CdDrive::CdDrive() = default;

CdDrive::~CdDrive()
{
    close();
}

bool CdDrive::isOpen() const
{
    if (m_mode == DriveMode::Physical)
        return m_handle != INVALID_HANDLE_VALUE;
    else
        return m_binFile.isOpen();
}

// ────────────────────────────────────────────
// 物理CDドライブ
// ────────────────────────────────────────────

QStringList CdDrive::availableDrives()
{
    QStringList result;
    DWORD mask = GetLogicalDrives();
    for (int i = 0; i < 26; ++i) {
        if (!(mask & (1 << i))) continue;
        QString letter = QString("%1:").arg(QChar('A' + i));
        if (GetDriveTypeW(reinterpret_cast<LPCWSTR>((letter + "\\").utf16()))
                == DRIVE_CDROM) {
            result << letter;
        }
    }
    return result;
}

bool CdDrive::open(const QString &driveLetter)
{
    close();
    m_mode = DriveMode::Physical;
    QString path = "\\\\.\\" + driveLetter;
    m_handle = CreateFileW(
        reinterpret_cast<LPCWSTR>(path.utf16()),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (m_handle == INVALID_HANDLE_VALUE) {
        qWarning() << "CdDrive: cannot open" << driveLetter;
        return false;
    }
    m_sourceName = driveLetter;
    return true;
}

// ────────────────────────────────────────────
// bin/cue ファイル
// ────────────────────────────────────────────

bool CdDrive::openCue(const QString &cuePath)
{
    close();
    m_mode = DriveMode::CueBin;

    m_cueDisc = parseCue(cuePath);
    if (m_cueDisc.tracks.isEmpty()) {
        qWarning() << "CdDrive: cue parse failed:" << cuePath;
        return false;
    }

    // 最初のトラックのbinファイルを開く
    QString binPath = m_cueDisc.tracks.first().binFile;
    m_binFile.setFileName(binPath);
    if (!m_binFile.open(QIODevice::ReadOnly)) {
        qWarning() << "CdDrive: cannot open bin:" << binPath;
        return false;
    }

    m_sourceName = QFileInfo(cuePath).fileName();
    return true;
}

void CdDrive::close()
{
    if (m_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
    if (m_binFile.isOpen())
        m_binFile.close();
    m_cueDisc = DiscInfo();
}

// ────────────────────────────────────────────
// cueファイル解析
// ────────────────────────────────────────────

DiscInfo CdDrive::parseCue(const QString &cuePath)
{
    DiscInfo disc;
    disc.isCueMode = true;

    QFile f(cuePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return disc;

    QTextStream in(&f);
    QString currentBin;
    QFileInfo cueInfo(cuePath);

    // cueシートのパース
    struct RawTrack {
        int     number;
        QString binFile;
        DWORD   indexSector; // INDEX 01 の位置（LBA）
    };
    QVector<RawTrack> rawTracks;

    auto msfToLba = [](int m, int s, int fr) -> DWORD {
        return (DWORD)(m * 60 + s) * 75 + fr;
    };

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.startsWith("FILE", Qt::CaseInsensitive)) {
            // FILE "name.bin" BINARY
            int q1 = line.indexOf('"');
            int q2 = line.lastIndexOf('"');
            if (q1 >= 0 && q2 > q1) {
                QString name = line.mid(q1+1, q2-q1-1);
                // 相対パスをcueファイルの場所に対して解決
                currentBin = QDir(cueInfo.absolutePath()).filePath(name);
            }
        }
        else if (line.startsWith("TRACK", Qt::CaseInsensitive)) {
            RawTrack rt;
            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 2)
                rt.number = parts[1].toInt();
            rt.binFile    = currentBin;
            rt.indexSector= 0;
            rawTracks.append(rt);
        }
        else if (line.startsWith("INDEX 01", Qt::CaseInsensitive)) {
            // INDEX 01 MM:SS:FF
            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 3 && !rawTracks.isEmpty()) {
                QStringList msf = parts[2].split(':');
                if (msf.size() == 3) {
                    rawTracks.last().indexSector =
                        msfToLba(msf[0].toInt(), msf[1].toInt(), msf[2].toInt());
                }
            }
        }
    }
    f.close();

    // TrackInfo に変換
    for (int i = 0; i < rawTracks.size(); ++i) {
        TrackInfo ti;
        ti.number      = rawTracks[i].number;
        ti.startSector = rawTracks[i].indexSector;
        ti.binFile     = rawTracks[i].binFile;

        if (i + 1 < rawTracks.size()) {
            ti.endSector = rawTracks[i+1].indexSector - 1;
        } else {
            // 最終トラック: binファイルサイズから計算
            QFile bin(ti.binFile);
            if (bin.open(QIODevice::ReadOnly)) {
                qint64 totalBytes = bin.size();
                DWORD  totalSectors = (DWORD)(totalBytes / CD_RAW_SECTOR_SIZE);
                ti.endSector = totalSectors - 1;
                bin.close();
            }
        }
        ti.durationSec = (int)(ti.endSector - ti.startSector + 1) / 75;
        disc.tracks.append(ti);
    }

    if (!disc.tracks.isEmpty())
        disc.totalSectors = disc.tracks.last().endSector + 1;

    return disc;
}

// ────────────────────────────────────────────
// TOC読み取り
// ────────────────────────────────────────────

DiscInfo CdDrive::readToc()
{
    if (m_mode == DriveMode::CueBin)
        return m_cueDisc;

    DiscInfo info;
    if (m_handle == INVALID_HANDLE_VALUE) return info;

    CDROM_TOC toc = {};
    DWORD bytesReturned = 0;

    BOOL ok = DeviceIoControl(
        m_handle, IOCTL_CDROM_READ_TOC,
        nullptr, 0, &toc, sizeof(toc),
        &bytesReturned, nullptr);

    if (!ok) {
        qWarning() << "CdDrive: IOCTL_CDROM_READ_TOC failed:" << GetLastError();
        return info;
    }

    int firstTrack = toc.FirstTrack;
    int lastTrack  = toc.LastTrack;

    for (int t = firstTrack; t <= lastTrack; ++t) {
        int idx = t - firstTrack;
        TRACK_DATA &td   = toc.TrackData[idx];
        TRACK_DATA &next = toc.TrackData[idx + 1];

        DWORD startLba = ((DWORD)td.Address[1]*60   + td.Address[2])*75   + td.Address[3]   - 150;
        DWORD endLba   = ((DWORD)next.Address[1]*60 + next.Address[2])*75 + next.Address[3] - 150 - 1;

        TrackInfo ti;
        ti.number      = t;
        ti.startSector = startLba;
        ti.endSector   = endLba;
        ti.durationSec = (int)(endLba - startLba + 1) / 75;
        info.tracks.append(ti);
    }

    TRACK_DATA &leadout = toc.TrackData[lastTrack - firstTrack + 1];
    info.totalSectors = ((DWORD)leadout.Address[1]*60 + leadout.Address[2])*75
                        + leadout.Address[3] - 150;
    return info;
}

// ────────────────────────────────────────────
// セクタ読み取り
// ────────────────────────────────────────────

QByteArray CdDrive::readSectorRaw(DWORD lba)
{
    if (m_mode == DriveMode::CueBin)
        return readSectorFromBin(lba);
    else
        return readSectorFromDrive(lba);
}

QByteArray CdDrive::readSectorFromBin(DWORD lba)
{
    if (!m_binFile.isOpen()) return {};

    qint64 offset = (qint64)lba * CD_RAW_SECTOR_SIZE;
    if (!m_binFile.seek(offset)) return {};

    QByteArray buf = m_binFile.read(CD_RAW_SECTOR_SIZE);
    if (buf.size() != CD_RAW_SECTOR_SIZE) return {};
    return buf;
}

QByteArray CdDrive::readSectorFromDrive(DWORD lba)
{
    RAW_READ_INFO rri = {};
    rri.DiskOffset.QuadPart = (LONGLONG)lba * 2048;
    rri.SectorCount = 1;
    rri.TrackMode   = CDDA;

    QByteArray buf(CD_RAW_SECTOR_SIZE, '\0');
    DWORD bytesReturned = 0;

    BOOL ok = DeviceIoControl(
        m_handle, IOCTL_CDROM_RAW_READ,
        &rri, sizeof(rri),
        buf.data(), CD_RAW_SECTOR_SIZE,
        &bytesReturned, nullptr);

    if (!ok || bytesReturned != (DWORD)CD_RAW_SECTOR_SIZE)
        return {};
    return buf;
}
