#pragma once
#include <QWidget>
#include <QTextEdit>

class LogViewer : public QWidget
{
    Q_OBJECT
public:
    explicit LogViewer(QWidget *parent = nullptr);
    void appendLine(const QString &line);
    void clear();

private:
    QTextEdit *m_edit = nullptr;
};
