#include "mainwindow.h"
#include <QKeyEvent>
#include <QShortcut>
#include <QApplication>
#include <QStackedWidget>
#include <QScrollBar>

static const QString APP_STYLESHEET = R"(
    QWidget {
        background-color: #010101;
        color: #e6edf3;
        font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
    }
    QScrollBar:vertical {
        background-color: #010101;
        width: 8px;
        border: none;
    }
    QScrollBar::handle:vertical {
        background-color: #30363d;
        border-radius: 4px;
        min-height: 24px;
    }
    QScrollBar::handle:vertical:hover {
        background-color: #484f58;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
        height: 0px;
    }
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
        background: none;
    }
    QScrollBar:horizontal {
        background-color: #010101;
        height: 8px;
        border: none;
    }
    QScrollBar::handle:horizontal {
        background-color: #30363d;
        border-radius: 4px;
        min-width: 24px;
    }
    QScrollBar::handle:horizontal:hover {
        background-color: #484f58;
    }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
        width: 0px;
    }
    QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
        background: none;
    }
    QToolTip {
        background-color: #121212;
        color: #e6edf3;
        border: 1px solid #30363d;
        padding: 4px 8px;
        font-size: 11px;
        border-radius: 4px;
    }
)";

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setStyleSheet(APP_STYLESHEET);

    setupUi();
    setupShortcuts();
}

void MainWindow::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Title bar
    m_titleBar = new TitleBar;
    m_titleBar->setBranchName("main");
    m_titleBar->setLastFetched("just now");

    connect(m_titleBar, &TitleBar::minimizeClicked, this, [this]() {
        window()->showMinimized();
    });
    connect(m_titleBar, &TitleBar::maximizeClicked, this, [this]() {
        if (window()->isMaximized()) {
            window()->showNormal();
        } else {
            window()->showMaximized();
        }
    });
    connect(m_titleBar, &TitleBar::closeClicked, this, [this]() {
        QApplication::quit();
    });
    connect(m_titleBar, &TitleBar::refreshClicked, this, [this]() {
        m_titleBar->setLastFetched("just now");
    });

    mainLayout->addWidget(m_titleBar);

    // Content area: sidebar + main content
    auto *contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // Right panel: stacked diff viewer and commit detail view
    auto *rightPanel = new QWidget;
    auto *rightLayout = new QStackedLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    m_diffViewer = new DiffViewer;
    m_detailView = new CommitDetailView;

    rightLayout->addWidget(m_diffViewer);   // index 0
    rightLayout->addWidget(m_detailView);   // index 1

    // Sidebar (contains both tabs and connects to both viewers)
    m_sidebar = new Sidebar(m_diffViewer, m_detailView);

    // When sidebar tab changes, show the appropriate right panel
    connect(m_sidebar, &Sidebar::tabChanged, this, [rightLayout](int index) {
        if (index == 0) {
            rightLayout->setCurrentIndex(0); // diff viewer for changes
        } else {
            rightLayout->setCurrentIndex(1); // commit detail view for history
        }
    });

    contentLayout->addWidget(m_sidebar);
    contentLayout->addWidget(rightPanel, 1);

    mainLayout->addLayout(contentLayout, 1);

    // Start maximized
    showMaximized();
}

void MainWindow::setupShortcuts() {
    // Ctrl+ArrowRight: switch to History tab
    auto *nextTabShortcut = new QShortcut(QKeySequence("Ctrl+Right"), this);
    connect(nextTabShortcut, &QShortcut::activated, this, [this]() {
        if (m_sidebar->currentIndex() == 0) {
            m_sidebar->switchToHistory();
        }
    });

    // Ctrl+ArrowLeft: switch to Changes tab
    auto *prevTabShortcut = new QShortcut(QKeySequence("Ctrl+Left"), this);
    connect(prevTabShortcut, &QShortcut::activated, this, [this]() {
        if (m_sidebar->currentIndex() == 1) {
            m_sidebar->switchToChanges();
        }
    });
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Down:
            if (m_sidebar->currentIndex() == 0) {
                m_sidebar->changesTab()->selectNextFile();
            }
            break;
        case Qt::Key_Up:
            if (m_sidebar->currentIndex() == 0) {
                m_sidebar->changesTab()->selectPreviousFile();
            }
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}