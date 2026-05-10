#pragma once
#include <QWidget>
#include <QVector>
#include "../ripper/ParanoiaReader.h"

class SectorMap : public QWidget
{
    Q_OBJECT
public:
    explicit SectorMap(QWidget *parent = nullptr);

    void reset(int totalSectors);
    void addResult(const SectorResult &sr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<SectorResult> m_results;
    int m_totalSectors = 0;

    // 色定義（ネオンブルー系）
    static constexpr QRgb COLOR_PERFECT = 0xFF4DB8FF; // 明るいネオンブルー
    static constexpr QRgb COLOR_OK      = 0xFF1A5A9A; // 中程度
    static constexpr QRgb COLOR_RETRY   = 0xFFFF8C42; // オレンジ（要注意）
    static constexpr QRgb COLOR_ERROR   = 0xFF8A1A1A; // 暗い赤（エラー）
    static constexpr QRgb COLOR_EMPTY   = 0xFF0A0A10; // 未読取
};
