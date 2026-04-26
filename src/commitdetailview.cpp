#include "commitdetailview.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QFont>
#include <QStackedLayout>
#include "mockdata.h"
#include "diffviewer.h"

static const QString DETAIL_STYLE = R"(
    QWidget#commitDetailView {
        background-color: #010101;
    }
    QLabel#detailSummary {
        color: #e6edf3;
        font-size: 16px;
        font-weight: bold;
        padding: 16px 16px 4px 16px;
    }
    QLabel#detailMeta {
        color: #8b949e;
        font-size: 11px;
        padding: 4px 16px;
    }
    QLabel#detailDescription {
        color: #e6edf3;
        font-size: 12px;
        padding: 8px 16px;
        line-height: 1.5;
    }
    QLabel#detailSectionTitle {
        color: #8b949e;
        font-size: 10px;
        font-weight: bold;
        text-transform: uppercase;
        padding: 12px 16px 4px 16px;
    }
    QLabel#detailFileItem {
        color: #58a6ff;
        font-size: 12px;
        padding: 3px 16px;
    }
    QWidget#detailContent {
        background-color: #010101;
    }
    QLabel#emptyState {
        color: #8b949e;
        font-size: 14px;
    }
)";

CommitDetailView::CommitDetailView(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("commitDetailView");
    setStyleSheet(DETAIL_STYLE);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_stack = new QStackedLayout;

    // Empty state
    auto *emptyWidget = new QWidget;
    emptyWidget->setObjectName("detailContent");
    auto *emptyLayout = new QVBoxLayout(emptyWidget);
    auto *emptyLabel = new QLabel("Select a commit from the history to view details");
    emptyLabel->setObjectName("emptyState");
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLayout->addWidget(emptyLabel);

    m_stack->addWidget(emptyWidget);

    // Content widget
    m_contentWidget = new QWidget;
    m_contentWidget->setObjectName("detailContent");
    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(m_contentWidget);

    m_stack->addWidget(scrollArea);
    m_stack->setCurrentIndex(0);

    mainLayout->addLayout(m_stack);
}

void CommitDetailView::showCommit(const CommitEntry &commit) {
    m_stack->setCurrentIndex(1);

    // Clear old content
    QLayoutItem *item;
    while ((item = m_contentWidget->layout()) != nullptr) {
        delete item;
    }
    delete m_contentWidget->layout();

    auto *layout = new QVBoxLayout(m_contentWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Summary
    m_summaryLabel = new QLabel(commit.summary);
    m_summaryLabel->setObjectName("detailSummary");
    m_summaryLabel->setWordWrap(true);
    layout->addWidget(m_summaryLabel);

    // Author and timestamp
    m_authorLabel = new QLabel(
        QString("%1 authored %2 ago")
            .arg(commit.author)
            .arg("2 hours")); // simplified
    m_authorLabel->setObjectName("detailMeta");
    layout->addWidget(m_authorLabel);

    // Hash
    m_hashLabel = new QLabel("Commit: " + commit.hash.left(7));
    m_hashLabel->setObjectName("detailMeta");
    layout->addWidget(m_hashLabel);

    // Description
    if (!commit.description.isEmpty()) {
        m_descriptionLabel = new QLabel(commit.description);
        m_descriptionLabel->setObjectName("detailDescription");
        m_descriptionLabel->setWordWrap(true);
        layout->addWidget(m_descriptionLabel);
    }

    // Separator
    auto *sep = new QWidget;
    sep->setFixedHeight(1);
    sep->setStyleSheet("background-color: #1a1a1a;");
    layout->addWidget(sep);

    // Files changed section
    auto *filesSectionTitle = new QLabel(
        QString("FILES CHANGED (%1)").arg(commit.changedFiles.size()));
    filesSectionTitle->setObjectName("detailSectionTitle");
    layout->addWidget(filesSectionTitle);

    for (const auto &file : commit.changedFiles) {
        auto *fileLabel = new QLabel(file);
        fileLabel->setObjectName("detailFileItem");
        layout->addWidget(fileLabel);
    }

    // Diff preview for the first changed file
    if (!commit.changedFiles.isEmpty()) {
        auto *diffSectionTitle = new QLabel("DIFF");
        diffSectionTitle->setObjectName("detailSectionTitle");
        layout->addWidget(diffSectionTitle);

        FileDiff diff = MockData::mockDiffForCommit(commit);

        // Render diff inline
        for (const auto &hunk : diff.hunks) {
            auto *hunkHeader = new QLabel(hunk.header);
            hunkHeader->setStyleSheet(
                "color: #8b949e; background-color: #1a1a2a; "
                "font-family: monospace; font-size: 11px; padding: 2px 16px;");
            layout->addWidget(hunkHeader);

            for (const auto &line : hunk.lines) {
                QString bgColor;
                QString prefix;
                QString prefixColor;
                switch (line.type) {
                    case DiffLine::Addition:
                        bgColor = "#1a3a2a";
                        prefix = "+";
                        prefixColor = "#3fb950";
                        break;
                    case DiffLine::Removal:
                        bgColor = "#3a1a1a";
                        prefix = "-";
                        prefixColor = "#f85149";
                        break;
                    default:
                        bgColor = "transparent";
                        prefix = " ";
                        prefixColor = "#484f58";
                }

                auto *lineWidget = new QWidget;
                lineWidget->setStyleSheet(QString("background-color: %1;").arg(bgColor));
                auto *lineLayout = new QHBoxLayout(lineWidget);
                lineLayout->setContentsMargins(16, 0, 16, 0);

                auto *prefixLabel = new QLabel(prefix);
                prefixLabel->setFixedWidth(16);
                prefixLabel->setStyleSheet(
                    QString("color: %1; font-family: monospace; font-size: 11px;").arg(prefixColor));
                lineLayout->addWidget(prefixLabel);

                auto *content = new QLabel(line.content.toHtmlEscaped());
                content->setStyleSheet(
                    "color: #e6edf3; font-family: monospace; font-size: 11px; background: transparent;");
                lineLayout->addWidget(content);
                lineLayout->addStretch();

                layout->addWidget(lineWidget);
            }
        }
    }

    layout->addStretch();
}

void CommitDetailView::clear() {
    m_stack->setCurrentIndex(0);
}