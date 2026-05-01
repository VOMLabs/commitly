#include "diff_viewer.h"
#include <QFontDatabase>

namespace commitly {

DiffHighlighter::DiffHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
}

void DiffHighlighter::highlightBlock(const QString &text) {
    QTextCharFormat format;
    if (text.startsWith('+')) {
        format.setForeground(QColor("#2cbb33"));
        setFormat(0, text.length(), format);
    } else if (text.startsWith('-')) {
        format.setForeground(QColor("#cb2431"));
        setFormat(0, text.length(), format);
    } else if (text.startsWith('@@')) {
        format.setForeground(QColor("#6f42c1"));
        setFormat(0, text.length(), format);
    } else if (text.startsWith("diff ") || text.startsWith("---") || text.startsWith("+++")) {
        format.setFontWeight(QFont::Bold);
        format.setForeground(Qt::gray);
        setFormat(0, text.length(), format);
    }
}

DiffViewer::DiffViewer(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setReadOnly(true);
    QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    monoFont.setPointSize(10);
    setFont(monoFont);

    m_highlighter = new DiffHighlighter(document());
    
    setPlaceholderText("Select a commit to view diff");
    setStyleSheet("background-color: #1e1e1e; color: #d1d1d1;");
}

void DiffViewer::loadCommitDiff(const QString &repoPath, const QString &sha) {
    // Later: call commitly_core_commit_diff via GitWorker
    clear();
    appendPlainText("--- a/example.cpp");
    appendPlainText("+++ b/example.cpp");
    appendPlainText("@@ -1,3 +1,3 @@");
    appendPlainText(" void main() {");
    appendPlainText("-    printf(\"hello\");");
    appendPlainText("+    printf(\"hello world\");");
    appendPlainText(" }");
}

void DiffViewer::clearDiff() {
    clear();
}

} // namespace commitly
