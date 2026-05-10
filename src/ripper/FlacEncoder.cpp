#include "FlacEncoder.h"
#include <FLAC/stream_encoder.h>
#include <QDebug>
#include <cstdio>

FlacEncoder::FlacEncoder() = default;

bool FlacEncoder::encode(const QVector<qint16> &pcm,
                         const QString         &outputPath,
                         int sampleRate, int channels,
                         int bitsPerSample, int compressionLevel)
{
    FLAC__StreamEncoder *encoder = FLAC__stream_encoder_new();
    if (!encoder) {
        m_lastError = "FLAC encoder allocation failed";
        return false;
    }

    FLAC__stream_encoder_set_channels(encoder, channels);
    FLAC__stream_encoder_set_bits_per_sample(encoder, bitsPerSample);
    FLAC__stream_encoder_set_sample_rate(encoder, sampleRate);
    FLAC__stream_encoder_set_compression_level(encoder, compressionLevel);

    // Windowsで日本語パスを正しく扱うため _wfopen を使用
#ifdef _WIN32
    FILE *fp = _wfopen(reinterpret_cast<const wchar_t*>(outputPath.utf16()), L"wb");
#else
    FILE *fp = fopen(outputPath.toUtf8().constData(), "wb");
#endif

    if (!fp) {
        m_lastError = "Cannot open output file: " + outputPath;
        FLAC__stream_encoder_delete(encoder);
        return false;
    }

    FLAC__StreamEncoderInitStatus initStatus =
        FLAC__stream_encoder_init_FILE(encoder, fp, nullptr, nullptr);

    if (initStatus != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
        m_lastError = QString("FLAC init failed: %1")
            .arg(FLAC__StreamEncoderInitStatusString[initStatus]);
        fclose(fp);
        FLAC__stream_encoder_delete(encoder);
        return false;
    }

    const int blockSize = 4096;
    int totalSamples = pcm.size();
    QVector<FLAC__int32> buf(blockSize * channels);

    for (int offset = 0; offset < totalSamples; offset += blockSize * channels) {
        int count = qMin(blockSize * channels, totalSamples - offset);
        for (int i = 0; i < count; ++i)
            buf[i] = pcm[offset + i];

        int frames = count / channels;
        if (!FLAC__stream_encoder_process_interleaved(encoder, buf.data(), frames)) {
            m_lastError = "FLAC encode error during write";
            FLAC__stream_encoder_finish(encoder);
            FLAC__stream_encoder_delete(encoder);
            return false;
        }
    }

    FLAC__stream_encoder_finish(encoder);
    FLAC__stream_encoder_delete(encoder);
    return true;
}
