#pragma once

#include <QPlainTextEdit>
#include <QSyntaxHighlighter>

namespace commitly {

class DiffHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    DiffHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;
};

class DiffViewer : public QPlainTextEdit
{
    Q_OBJECT
public:
    DiffViewer(QWidget *parent = nullptr);

    void loadCommitDiff(const QString &repoPath, const QString &sha);
    void clearDiff();

private:
    DiffHighlighter *m_highlighter;
};

} // namespace commitly
