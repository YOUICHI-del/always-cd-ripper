#pragma once
#include <QWidget>
#include "../metadata/MusicBrainz.h"

class MetadataEditor : public QWidget
{
    Q_OBJECT
public:
    explicit MetadataEditor(QWidget *parent = nullptr);
    void populate(const MbRelease &release);
};
