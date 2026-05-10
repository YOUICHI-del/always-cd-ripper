#pragma once
#include <QString>
#include <QVector>

class FlacEncoder
{
public:
    FlacEncoder();

    // PCM データを FLAC ファイルとして保存
    // pcm: インターリーブされた 16bit ステレオサンプル
    bool encode(const QVector<qint16> &pcm,
                const QString         &outputPath,
                int                    sampleRate    = 44100,
                int                    channels      = 2,
                int                    bitsPerSample = 16,
                int                    compressionLevel = 8);

    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;
};
