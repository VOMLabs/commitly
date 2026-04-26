#include "historytab.h"
#include "mockdata.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>

static const QString HISTORY_STYLE = R"(
    QWidget#historyTab {
        background-color: #121212;
    }
    QLineEdit {
        background-color: #010101;
        border: 1px solid #30363d;
        border-radius: 4px;
        color: #e6edf3;
        font-size: 12px;
        padding: 6px 8px;
    }
    QLineEdit:focus {
        border-color: #1f6feb;
    }
    QListWidget {
        background-color: transparent;
        border: none;
        color: #e6edf3;
        font-size: 12px;
        padding: 4px;
        outline: none;
    }
    QListWidget::item {
        padding: 8px 12px;
        border-bottom: 1px solid #1a1a1a;
    }
    QListWidget::item:selected {
        background-color: #1f6feb;
        color: #ffffff;
    }
    QListWidget::item:hover:!selected {
        background-color: #1a1a1a;
    }
    QPushButton#sparkleBtn {
        color: #d2a8ff;
        background: transparent;
        border: none;
        font-size: 14px;
        padding: 2px 6px;
        border-radius: 4px;
    }
    QPushButton#sparkleBtn:hover {
        background-color: rgba(210, 168, 255, 0.1);
    }
)";

HistoryTab::HistoryTab(CommitDetailView *detailView, QWidget *parent)
    : QWidget(parent), m_detailView(detailView)
{
    setObjectName("historyTab");
    setStyleSheet(HISTORY_STYLE);

    m_commits = MockData::mockCommits();
    m_filteredCommits = m_commits;

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Search header
    auto *searchWidget = new QWidget;
    searchWidget->setStyleSheet("background-color: #121212; padding: 8px 12px;");
    auto *searchLayout = new QHBoxLayout(searchWidget);
    searchLayout->setContentsMargins(12, 8, 12, 4);

    m_searchInput = new QLineEdit;
    m_searchInput->setPlaceholderText("Search commits by message or hash...");
    connect(m_searchInput, &QLineEdit::textChanged, this, &HistoryTab::onSearchChanged);
    searchLayout->addWidget(m_searchInput);

    auto *sparkleBtn = new QPushButton("\xE2\x9C\xA8"); // ✨
    sparkleBtn->setObjectName("sparkleBtn");
    sparkleBtn->setToolTip("AI analysis (coming soon)");
    searchLayout->addWidget(sparkleBtn);

    layout->addWidget(searchWidget);

    // Commit list
    m_commitList = new QListWidget;
    m_commitList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_commitList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_commitList->setSpacing(0);
    layout->addWidget(m_commitList, 1);

    connect(m_commitList, &QListWidget::itemClicked, this, &HistoryTab::onCommitClicked);

    populateCommitList(m_filteredCommits);
}

void HistoryTab::populateCommitList(const QList<CommitEntry> &commits) {
    m_commitList->clear();

    for (const auto &commit : commits) {
        auto *item = new QListWidgetItem;

        // Store data for later use
        item->setData(Qt::UserRole, commit.hash);

        // Format: summary on first line, author + time on second line
        QString displayText = QString("%1\n%2  •  %3  \xE2\x96\xB6")
            .arg(commit.summary, commit.author, "2 hours ago");

        item->setText(commit.summary);
        item->setToolTip(
            QString("Author: %1\nHash: %2\nFiles: %3")
                .arg(commit.author, commit.hash.left(7))
                .arg(commit.changedFiles.size()));

        item->setForeground(QColor("#e6edf3"));
        m_commitList->addItem(item);
    }
}

void HistoryTab::onCommitClicked(QListWidgetItem *item) {
    QString hash = item->data(Qt::UserRole).toString();

    for (const auto &commit : m_filteredCommits) {
        if (commit.hash == hash) {
            m_detailView->showCommit(commit);
            break;
        }
    }
}

void HistoryTab::onSearchChanged(const QString &text) {
    if (text.trimmed().isEmpty()) {
        m_filteredCommits = m_commits;
    } else {
        m_filteredCommits.clear();
        QString lowerText = text.toLower();
        for (const auto &commit : m_commits) {
            if (commit.summary.toLower().contains(lowerText) ||
                commit.hash.toLower().contains(lowerText)) {
                m_filteredCommits.append(commit);
            }
        }
    }
    populateCommitList(m_filteredCommits);
}