#include "SectorMap.h"
#include <QPainter>
#include <QColor>
#include <cmath>

SectorMap::SectorMap(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(200, 120);
}

void SectorMap::reset(int totalSectors)
{
    m_totalSectors = totalSectors;
    m_results.clear();
    update();
}

void SectorMap::addResult(const SectorResult &sr)
{
    m_results.append(sr);
    update();
}

void SectorMap::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    int w = width();
    int h = height();

    // 背景
    p.fillRect(rect(), QColor(0x05, 0x05, 0x08));

    if (m_totalSectors == 0) return;

    // グリッドの列数を幅から計算
    int cols = qMax(1, w / 7);
    int rows = static_cast<int>(std::ceil(
        static_cast<double>(m_totalSectors) / cols));
    int cellW = w / cols;
    int cellH = qMax(4, h / qMax(1, rows));

    for (int i = 0; i < m_totalSectors; ++i) {
        int col = i % cols;
        int row = i / cols;
        QRect cell(col * cellW + 1, row * cellH + 1,
                   cellW - 2, cellH - 2);

        QColor color(0x0A, 0x0A, 0x10); // デフォルト=未読取
        if (i < m_results.size()) {
            const SectorResult &sr = m_results[i];
            if (sr.hasError)
                color = QColor(0x8A, 0x1A, 0x1A);
            else if (sr.retries > 5)
                color = QColor(0xFF, 0x8C, 0x42);
            else if (sr.retries > 0)
                color = QColor(0x1A, 0x5A, 0x9A);
            else
                color = QColor(0x4D, 0xB8, 0xFF);
        }
        p.fillRect(cell, color);
    }
}
