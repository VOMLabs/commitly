#include "diffviewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QPushButton>

static const QString DIFF_STYLESHEET = R"(
    QWidget#diffViewer {
        background-color: #010101;
    }
    QWidget#diffHeader {
        background-color: #121212;
        border-bottom: 1px solid #1a1a1a;
        padding: 8px 12px;
    }
    QLabel#diffFileName {
        color: #e6edf3;
        font-size: 12px;
        font-weight: bold;
    }
    QTextEdit#diffRender {
        background-color: #010101;
        color: #e6edf3;
        font-family: "Fira Code", "Cascadia Code", "JetBrains Mono", "Consolas", monospace;
        font-size: 12px;
        border: none;
        padding: 8px;
        selection-background-color: #1f6feb;
    }
)";

static QString lineBgColor(const DiffLine &line) {
    switch (line.type) {
        case DiffLine::Addition:   return "#1a3a2a";
        case DiffLine::Removal:    return "#3a1a1a";
        case DiffLine::HunkHeader: return "#1a1a2a";
        default:                   return "transparent";
    }
}

DiffViewer::DiffViewer(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("diffViewer");
    setStyleSheet(DIFF_STYLESHEET);
    setupUi();
}

void DiffViewer::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Header bar
    auto *headerWidget = new QWidget;
    headerWidget->setObjectName("diffHeader");
    auto *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(12, 8, 12, 8);

    m_headerLabel = new QLabel("No file selected");
    m_headerLabel->setObjectName("diffFileName");
    headerLayout->addWidget(m_headerLabel);
    headerLayout->addStretch();

    
    layout->addWidget(headerWidget);

    // Scroll area for diff
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_diffContent = new QWidget;
    m_diffContent->setObjectName("diffContent");
    m_scrollArea->setWidget(m_diffContent);

    layout->addWidget(m_scrollArea);
}

void DiffViewer::showDiff(const FileDiff &diff) {
    m_headerLabel->setText(diff.filePath);

    // Remove old content
    QLayoutItem *item;
    while ((item = m_diffContent->layout()) != nullptr) {
        delete item;
    }
    delete m_diffContent->layout();

    auto *contentLayout = new QVBoxLayout(m_diffContent);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    if (diff.hunks.isEmpty()) {
        auto *emptyLabel = new QLabel("No changes in this file.");
        emptyLabel->setStyleSheet("color: #8b949e; padding: 24px; font-size: 13px;");
        emptyLabel->setAlignment(Qt::AlignCenter);
        contentLayout->addWidget(emptyLabel);
    } else {
        for (const auto &hunk : diff.hunks) {
            // Hunk header
            auto *hunkHeader = new QLabel(hunk.header);
            hunkHeader->setStyleSheet(
                "color: #8b949e; background-color: #1a1a2a; "
                "font-family: monospace; font-size: 11px; padding: 2px 12px;");
            contentLayout->addWidget(hunkHeader);

            // Determine max digits for line numbers
            int maxOldDigits = 1;
            int maxNewDigits = 1;
            for (const auto &line : hunk.lines) {
                if (line.oldLineNumber > 0) {
                    int digits = QString::number(line.oldLineNumber).length();
                    if (digits > maxOldDigits) maxOldDigits = digits;
                }
                if (line.newLineNumber > 0) {
                    int digits = QString::number(line.newLineNumber).length();
                    if (digits > maxNewDigits) maxNewDigits = digits;
                }
            }

            for (const auto &line : hunk.lines) {
                auto *lineWidget = createLineWidget(line, maxOldDigits, maxNewDigits);
                contentLayout->addWidget(lineWidget);
            }
        }
    }

    contentLayout->addStretch();
}

QWidget* DiffViewer::createLineWidget(const DiffLine &line, int maxOldDigits, int maxNewDigits) {
    auto *widget = new QWidget;
    QString bg = lineBgColor(line);
    widget->setStyleSheet(QString("background-color: %1;").arg(bg));

    auto *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Left gutter (old line number)
    auto *oldNumLabel = new QLabel;
    if (line.oldLineNumber > 0) {
        oldNumLabel->setText(QString::number(line.oldLineNumber));
    }
    oldNumLabel->setFixedWidth(48);
    oldNumLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    oldNumLabel->setStyleSheet(
        "color: #484f58; background-color: transparent; "
        "font-family: monospace; font-size: 11px; padding-right: 8px;");

    // Right gutter (new line number)
    auto *newNumLabel = new QLabel;
    if (line.newLineNumber > 0) {
        newNumLabel->setText(QString::number(line.newLineNumber));
    }
    newNumLabel->setFixedWidth(48);
    newNumLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    newNumLabel->setStyleSheet(
        "color: #484f58; background-color: transparent; "
        "font-family: monospace; font-size: 11px; padding-right: 8px;");

    // Prefix symbol
    QString prefix;
    QString prefixColor;
    switch (line.type) {
        case DiffLine::Addition:
            prefix = "+";
            prefixColor = "#3fb950";
            break;
        case DiffLine::Removal:
            prefix = "-";
            prefixColor = "#f85149";
            break;
        default:
            prefix = " ";
            prefixColor = "#484f58";
    }

    auto *prefixLabel = new QLabel(prefix);
    prefixLabel->setFixedWidth(16);
    prefixLabel->setAlignment(Qt::AlignCenter);
    prefixLabel->setStyleSheet(
        QString("color: %1; background-color: transparent; font-weight: bold; "
                "font-family: monospace; font-size: 11px;").arg(prefixColor));

    // Content
    auto *contentLabel = new QLabel(line.content.toHtmlEscaped());
    contentLabel->setStyleSheet(
        "color: #e6edf3; background-color: transparent; "
        "font-family: monospace; font-size: 11px; white-space: pre;");
    contentLabel->setWordWrap(false);

    layout->addWidget(oldNumLabel);
    layout->addWidget(newNumLabel);
    layout->addWidget(prefixLabel);
    layout->addWidget(contentLabel, 1);

    return widget;
}

void DiffViewer::clear() {
    m_headerLabel->setText("No file selected");
    QLayoutItem *item;
    while ((item = m_diffContent->layout()) != nullptr) {
        delete item;
    }
    delete m_diffContent->layout();

    auto *contentLayout = new QVBoxLayout(m_diffContent);
    auto *emptyLabel = new QLabel("Select a file to view its diff");
    emptyLabel->setStyleSheet("color: #8b949e; padding: 24px; font-size: 13px;");
    emptyLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(emptyLabel);
    contentLayout->addStretch();
}