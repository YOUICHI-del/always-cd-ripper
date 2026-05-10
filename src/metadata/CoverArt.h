#pragma once
#include <QObject>
#include <QPixmap>
#include <QNetworkAccessManager>

class CoverArt : public QObject
{
    Q_OBJECT
public:
    explicit CoverArt(QObject *parent = nullptr);

    // アーティスト名・アルバム名でMusicBrainzを検索してアートを取得
    void fetch(const QString &artist, const QString &album);

    // 同期版（RipWorkerスレッドから使用）
    QPixmap fetchSync(const QString &artist, const QString &album);

signals:
    void imageReady(QPixmap pixmap);
    void error(const QString &msg);

private:
    QPixmap downloadImage(const QString &url);
    QString searchITunes(const QString &artist, const QString &album);

    QNetworkAccessManager *m_nam = nullptr;

    static constexpr const char *USER_AGENT =
        "AlwaysCDRipper/1.0.0 ( https://github.com/YOUICHI-del )";
};
