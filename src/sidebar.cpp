#include "sidebar.h"
#include <QFont>

static const QString SIDEBAR_STYLE = R"(
    QWidget#sidebar {
        background-color: #121212;
        border-right: 1px solid #1a1a1a;
    }
    QPushButton#tabBtn {
        color: #8b949e;
        background: transparent;
        border: none;
        font-size: 12px;
        font-weight: bold;
        padding: 10px 16px;
        text-align: left;
        border-bottom: 2px solid transparent;
    }
    QPushButton#tabBtn:hover {
        color: #e6edf3;
        background-color: #1a1a1a;
    }
    QPushButton#tabBtn:checked {
        color: #e6edf3;
        border-bottom: 2px solid #1f6feb;
    }
)";

Sidebar::Sidebar(DiffViewer *diffViewer, CommitDetailView *detailView,
                 QWidget *parent)
    : QWidget(parent)
{
    setObjectName("sidebar");
    setStyleSheet(SIDEBAR_STYLE);
    setFixedWidth(300);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Tab buttons
    setupTabButtons(layout);

    // Stacked content
    m_stack = new QStackedWidget;
    m_stack->setStyleSheet("background-color: transparent;");

    m_changesTab = new ChangesTab(diffViewer);
    m_stack->addWidget(m_changesTab);

    m_historyTab = new HistoryTab(detailView);
    m_stack->addWidget(m_historyTab);

    layout->addWidget(m_stack, 1);

    // Sync header at bottom
    m_syncHeader = new SyncHeader;
    m_syncHeader->setPendingPush(2);
    m_syncHeader->setPendingPull(1);
    layout->addWidget(m_syncHeader);

    // Default to changes tab
    m_stack->setCurrentIndex(0);
    m_changesBtn->setChecked(true);
}

void Sidebar::setupTabButtons(QVBoxLayout *layout) {
    auto *btnWidget = new QWidget;
    btnWidget->setStyleSheet("background-color: #121212;");
    auto *btnLayout = new QVBoxLayout(btnWidget);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(0);

    m_changesBtn = new QPushButton("  Changes");
    m_changesBtn->setObjectName("tabBtn");
    m_changesBtn->setCheckable(true);
    m_changesBtn->setCursor(Qt::PointingHandCursor);
    connect(m_changesBtn, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(0);
        m_changesBtn->setChecked(true);
        m_historyBtn->setChecked(false);
        emit tabChanged(0);
    });

    m_historyBtn = new QPushButton("  History");
    m_historyBtn->setObjectName("tabBtn");
    m_historyBtn->setCheckable(true);
    m_historyBtn->setCursor(Qt::PointingHandCursor);
    connect(m_historyBtn, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(1);
        m_historyBtn->setChecked(true);
        m_changesBtn->setChecked(false);
        emit tabChanged(1);
    });

    btnLayout->addWidget(m_changesBtn);
    btnLayout->addWidget(m_historyBtn);
    layout->addWidget(btnWidget);
}

void Sidebar::switchToChanges() {
    m_changesBtn->click();
}

void Sidebar::switchToHistory() {
    m_historyBtn->click();
}