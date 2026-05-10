#include "MetadataEditor.h"
#include <QVBoxLayout>
#include <QLabel>

MetadataEditor::MetadataEditor(QWidget *parent) : QWidget(parent)
{
    auto *lay = new QVBoxLayout(this);
    lay->addWidget(new QLabel("Metadata Editor — coming soon", this));
}

void MetadataEditor::populate(const MbRelease &) {}
