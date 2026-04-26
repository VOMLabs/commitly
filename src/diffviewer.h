#ifndef DIFFVIEWER_H
#define DIFFVIEWER_H

#include <QWidget>
#include <QTextEdit>
#include <QLabel>
#include <QScrollArea>
#include "models.h"

class DiffViewer : public QWidget {
    Q_OBJECT

public:
    explicit DiffViewer(QWidget *parent = nullptr);
    void showDiff(const FileDiff &diff);
    void clear();

private:
    QLabel *m_headerLabel;
    QWidget *m_diffContent;
    QScrollArea *m_scrollArea;

    void setupUi();
    QWidget* createLineWidget(const DiffLine &line, int maxOldDigits, int maxNewDigits);
    QString renderDiffHtml(const FileDiff &diff);
};

#endif // DIFFVIEWER_H