#include "changestab.h"
#include "mockdata.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QTimer>
#include <QScrollBar>

static const QString CHANGES_STYLE = R"(
    QWidget#changesTab {
        background-color: #121212;
    }
    QCheckBox {
        color: #e6edf3;
        font-size: 11px;
        spacing: 6px;
    }
    QCheckBox::indicator {
        width: 14px;
        height: 14px;
        border-radius: 3px;
        border: 1px solid #30363d;
        background-color: #010101;
    }
    QCheckBox::indicator:checked {
        background-color: #1f6feb;
        border-color: #1f6feb;
    }
    QListWidget {
        background-color: #010101;
        border: none;
        border-radius: 4px;
        color: #e6edf3;
        font-size: 12px;
        padding: 4px;
        outline: none;
    }
    QListWidget::item {
        padding: 6px 8px;
        border-radius: 4px;
        border: none;
    }
    QListWidget::item:selected {
        background-color: #1f6feb;
        color: #ffffff;
    }
    QListWidget::item:hover:!selected {
        background-color: #1a1a1a;
    }
    QLineEdit, QTextEdit {
        background-color: #010101;
        border: 1px solid #30363d;
        border-radius: 4px;
        color: #e6edf3;
        font-size: 12px;
        padding: 6px 8px;
    }
    QLineEdit:focus, QTextEdit:focus {
        border-color: #1f6feb;
    }
    QLabel#sectionTitle {
        color: #8b949e;
        font-size: 10px;
        font-weight: bold;
        text-transform: uppercase;
        padding: 0 4px;
    }
    QLabel#fileStatusLabel {
        font-size: 10px;
        font-weight: bold;
        padding: 1px 4px;
        border-radius: 3px;
        min-width: 16px;
        max-height: 14px;
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
    QPushButton#actionBtn {
        color: #8b949e;
        background: transparent;
        border: none;
        font-size: 11px;
        padding: 4px 8px;
        border-radius: 4px;
    }
    QPushButton#actionBtn:hover {
        background-color: #1a1a1a;
        color: #e6edf3;
    }
    QPushButton#commitBtn {
        background-color: #1f6feb;
        color: #ffffff;
        border: none;
        border-radius: 4px;
        font-size: 12px;
        font-weight: bold;
        padding: 8px 16px;
    }
    QPushButton#commitBtn:hover {
        background-color: #388bfd;
    }
    QPushButton#commitBtn:disabled {
        background-color: #21262d;
        color: #484f58;
    }
)";

ChangesTab::ChangesTab(DiffViewer *diffViewer, QWidget *parent)
    : QWidget(parent), m_diffViewer(diffViewer)
{
    setObjectName("changesTab");
    setStyleSheet(CHANGES_STYLE);

    m_files = MockData::mockFiles();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // File list header with select all
    auto *headerWidget = new QWidget;
    headerWidget->setStyleSheet("background-color: #121212; padding: 8px 12px;");
    auto *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(12, 8, 12, 4);

    auto *changesLabel = new QLabel("CHANGES");
    changesLabel->setObjectName("sectionTitle");
    headerLayout->addWidget(changesLabel);

    headerLayout->addStretch();

    m_selectAllCheck = new QCheckBox("Select all");
    connect(m_selectAllCheck, &QCheckBox::toggled, this, &ChangesTab::onStagingChanged);
    headerLayout->addWidget(m_selectAllCheck);

    layout->addWidget(headerWidget);

    // File list
    m_fileList = new QListWidget;
    m_fileList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_fileList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_fileList->setSpacing(2);
    layout->addWidget(m_fileList, 1);

    connect(m_fileList, &QListWidget::itemClicked, this, &ChangesTab::onFileClicked);

    // Commit message area
    auto *commitWidget = new QWidget;
    commitWidget->setStyleSheet("background-color: #121212;");
    auto *commitLayout = new QVBoxLayout(commitWidget);
    commitLayout->setContentsMargins(12, 8, 12, 8);
    commitLayout->setSpacing(6);

    // Author avatar area
    auto *authorWidget = new QWidget;
    auto *authorLayout = new QHBoxLayout(authorWidget);
    authorLayout->setContentsMargins(0, 0, 0, 0);

    auto *avatarLabel = new QLabel("\xF0\x9F\x91\xA4"); // user emoji
    avatarLabel->setStyleSheet("font-size: 20px;");
    authorLayout->addWidget(avatarLabel);

    auto *authorName = new QLabel("You");
    authorName->setStyleSheet("color: #e6edf3; font-size: 12px; font-weight: bold;");
    authorLayout->addWidget(authorName);

    authorLayout->addStretch();
    commitLayout->addWidget(authorWidget);

    // Summary input
    auto *summaryWidget = new QWidget;
    auto *summaryLayout = new QHBoxLayout(summaryWidget);
    summaryLayout->setContentsMargins(0, 0, 0, 0);

    m_summaryInput = new QLineEdit;
    m_summaryInput->setPlaceholderText("Summary (required)");
    summaryLayout->addWidget(m_summaryInput);

    auto *genSummaryBtn = new QPushButton("\xE2\x9C\xA8"); // ✨
    genSummaryBtn->setObjectName("sparkleBtn");
    genSummaryBtn->setToolTip("Generate AI commit summary");
    connect(genSummaryBtn, &QPushButton::clicked, this, &ChangesTab::onGenerateSummaryClicked);
    summaryLayout->addWidget(genSummaryBtn);

    commitLayout->addWidget(summaryWidget);

    // Description input
    auto *descWidget = new QWidget;
    auto *descLayout = new QHBoxLayout(descWidget);
    descLayout->setContentsMargins(0, 0, 0, 0);

    m_descInput = new QTextEdit;
    m_descInput->setPlaceholderText("Description (optional)");
    m_descInput->setMaximumHeight(60);
    m_descInput->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    descLayout->addWidget(m_descInput);

    auto *genDescBtn = new QPushButton("\xE2\x9C\xA8"); // ✨
    genDescBtn->setObjectName("sparkleBtn");
    genDescBtn->setToolTip("Generate AI commit description");
    connect(genDescBtn, &QPushButton::clicked, this, &ChangesTab::onGenerateDescriptionClicked);
    descLayout->addWidget(genDescBtn);

    commitLayout->addWidget(descWidget);

    // Utility buttons
    auto *utilWidget = new QWidget;
    auto *utilLayout = new QHBoxLayout(utilWidget);
    utilLayout->setContentsMargins(0, 0, 0, 0);

    auto *coAuthorBtn = new QPushButton("+ Add Co-author");
    coAuthorBtn->setObjectName("actionBtn");
    utilLayout->addWidget(coAuthorBtn);

    auto *optionsBtn = new QPushButton("Options");
    optionsBtn->setObjectName("actionBtn");
    utilLayout->addWidget(optionsBtn);

    utilLayout->addStretch();
    commitLayout->addWidget(utilWidget);

    // Commit button
    m_commitBtn = new QPushButton("Commit to main");
    m_commitBtn->setObjectName("commitBtn");
    connect(m_commitBtn, &QPushButton::clicked, this, &ChangesTab::onCommitClicked);
    commitLayout->addWidget(m_commitBtn);

    layout->addWidget(commitWidget);

    populateFileList();
    updateCommitButton();
}

void ChangesTab::populateFileList() {
    m_fileList->clear();

    for (const auto &file : m_files) {
        auto *item = new QListWidgetItem;
        item->setText(file.fileName);
        item->setData(Qt::UserRole, file.filePath);
        item->setData(Qt::UserRole + 1, file.statusLabel());
        item->setData(Qt::UserRole + 2, file.statusColor());
        item->setData(Qt::UserRole + 3, file.staged ? "staged" : "unstaged");

        QString statusColor = file.statusColor();
        QString statusLabel = file.statusLabel();
        item->setText(QString("%1  %2").arg(statusLabel, file.fileName));
        item->setForeground(QColor("#e6edf3"));

        m_fileList->addItem(item);
    }
}

void ChangesTab::onFileClicked(QListWidgetItem *item) {
    QString filePath = item->data(Qt::UserRole).toString();
    FileDiff diff = MockData::mockDiffForFile(filePath);
    m_diffViewer->showDiff(diff);
    emit fileSelected(filePath);
}

void ChangesTab::onStagingChanged() {
    bool checked = m_selectAllCheck->isChecked();
    for (int i = 0; i < m_files.size(); ++i) {
        m_files[i].staged = checked;
    }
    updateCommitButton();
}

void ChangesTab::onCommitClicked() {
    if (m_summaryInput->text().trimmed().isEmpty()) return;

    QString summary = m_summaryInput->text().trimmed();
    QString description = m_descInput->toPlainText().trimmed();

    // Simulate commit
    m_summaryInput->clear();
    m_descInput->clear();

    for (auto &file : m_files) {
        file.staged = false;
    }

    updateCommitButton();

    // Show feedback
    m_commitBtn->setText("Committed ✓");
    QTimer::singleShot(2000, this, [this]() {
        updateCommitButton();
    });
}

void ChangesTab::selectNextFile() {
    int current = m_fileList->currentRow();
    if (current < m_fileList->count() - 1) {
        m_fileList->setCurrentRow(current + 1);
        onFileClicked(m_fileList->item(current + 1));
    }
}

void ChangesTab::selectPreviousFile() {
    int current = m_fileList->currentRow();
    if (current > 0) {
        m_fileList->setCurrentRow(current - 1);
        onFileClicked(m_fileList->item(current - 1));
    }
}

void ChangesTab::onGenerateSummaryClicked() {
    m_summaryInput->setPlaceholderText("Generating...");
    QTimer::singleShot(1500, this, [this]() {
        m_summaryInput->setText(generateAISummary());
        m_summaryInput->setPlaceholderText("Summary (required)");
    });
}

void ChangesTab::onGenerateDescriptionClicked() {
    m_descInput->setPlaceholderText("Generating...");
    QTimer::singleShot(2000, this, [this]() {
        m_descInput->setPlainText(generateAIDescription());
        m_descInput->setPlaceholderText("Description (optional)");
    });
}

QString ChangesTab::generateAISummary() {
    // Simulated AI generation
    QStringList stagedFiles;
    for (const auto &f : m_files) {
        if (f.staged) {
            stagedFiles << f.fileName;
        }
    }

    if (stagedFiles.isEmpty())
        return "Fix: update project configuration and dependencies";

    return "Refactor: update " + stagedFiles.first() + " and related files";
}

QString ChangesTab::generateAIDescription() {
    return "This commit introduces the following changes:\n\n"
           "- Refactored core components for improved maintainability\n"
           "- Updated styling to match design specifications\n"
           "- Fixed edge cases in error handling\n\n"
           "These changes address recent code review feedback.";
}

void ChangesTab::updateCommitButton() {
    int stagedCount = 0;
    for (const auto &f : m_files) {
        if (f.staged) stagedCount++;
    }

    bool canCommit = stagedCount > 0 && !m_summaryInput->text().trimmed().isEmpty();
    m_commitBtn->setEnabled(canCommit);
    m_commitBtn->setText(QString("Commit %1 file(s) to main").arg(stagedCount));
}