#include "LogViewer.h"
#include <QVBoxLayout>

LogViewer::LogViewer(QWidget *parent) : QWidget(parent)
{
    m_edit = new QTextEdit(this);
    m_edit->setReadOnly(true);
    m_edit->setFont(QFont("Consolas", 9));
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_edit);
}

void LogViewer::appendLine(const QString &line)
{
    m_edit->append(line);
}

void LogViewer::clear()
{
    m_edit->clear();
}
