#include "titlebar.h"
#include <QMouseEvent>
#include <QFont>
#include <QApplication>

static const QString TITLE_BAR_STYLE = R"(
    QWidget#titleBar {
        background-color: #010101;
        border-bottom: 1px solid #1a1a1a;
        min-height: 40px;
        max-height: 40px;
    }
    QLabel#appName {
        color: #e6edf3;
        font-size: 13px;
        font-weight: bold;
        padding-left: 12px;
    }
    QLabel#appIcon {
        color: #1f6feb;
        font-size: 16px;
        padding-left: 12px;
    }
    QLabel#branchInfo {
        color: #8b949e;
        font-size: 11px;
        padding-left: 12px;
        background-color: #1a1a1a;
        border-radius: 4px;
        padding: 2px 8px;
    }
    QLabel#lastFetched {
        color: #8b949e;
        font-size: 10px;
        padding-left: 12px;
    }
    QPushButton#refreshBtn {
        color: #8b949e;
        background: transparent;
        border: none;
        font-size: 14px;
        padding: 4px 8px;
        border-radius: 4px;
    }
    QPushButton#refreshBtn:hover {
        color: #e6edf3;
        background-color: #1a1a1a;
    }
    QPushButton#winBtn,
    QPushButton#closeBtn {
        color: #8b949e;
        background: transparent;
        border: none;
        font-size: 12px;
        padding: 0px;
        width: 40px;
        height: 40px;
    }
    QPushButton#winBtn:hover {
        background-color: #1a1a1a;
        color: #e6edf3;
    }
    QPushButton#closeBtn:hover {
        background-color: #f85149;
        color: #ffffff;
    }
)";

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("titleBar");
    setStyleSheet(TITLE_BAR_STYLE);
    setFixedHeight(40);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    // App icon (unicode repo icon)
    auto *iconLabel = new QLabel("\xF0\x9F\x93\x81"); // file folder emoji
    iconLabel->setObjectName("appIcon");
    layout->addWidget(iconLabel);

    // App name
    auto *nameLabel = new QLabel("commitly");
    nameLabel->setObjectName("appName");
    layout->addWidget(nameLabel);

    // Branch
    m_branchLabel = new QLabel("main");
    m_branchLabel->setObjectName("branchInfo");
    layout->addWidget(m_branchLabel);

    // Spacer
    layout->addStretch();

    // Last fetched
    m_lastFetchedLabel = new QLabel("Last fetched: just now");
    m_lastFetchedLabel->setObjectName("lastFetched");
    layout->addWidget(m_lastFetchedLabel);

    // Refresh button
    m_refreshBtn = new QPushButton("\xE2\x9F\xB3"); // ⟳
    m_refreshBtn->setObjectName("refreshBtn");
    m_refreshBtn->setToolTip("Refresh repository state");
    connect(m_refreshBtn, &QPushButton::clicked, this, &TitleBar::refreshClicked);
    layout->addWidget(m_refreshBtn);

    // Spacer before window buttons
    layout->addSpacing(8);

    // Window buttons
    m_minimizeBtn = new QPushButton("\xE2\x80\x95"); // ―
    m_minimizeBtn->setObjectName("winBtn");
    connect(m_minimizeBtn, &QPushButton::clicked, this, &TitleBar::minimizeClicked);
    layout->addWidget(m_minimizeBtn);

    m_maximizeBtn = new QPushButton("\xE2\x96\xA1"); // □
    m_maximizeBtn->setObjectName("winBtn");
    connect(m_maximizeBtn, &QPushButton::clicked, this, &TitleBar::maximizeClicked);
    layout->addWidget(m_maximizeBtn);

    m_closeBtn = new QPushButton("\xE2\x9C\x95"); // ✕
    m_closeBtn->setObjectName("closeBtn");
    connect(m_closeBtn, &QPushButton::clicked, this, &TitleBar::closeClicked);
    layout->addWidget(m_closeBtn);
}

void TitleBar::setBranchName(const QString &branch) {
    m_branchLabel->setText(branch);
}

void TitleBar::setLastFetched(const QString &lastFetched) {
    m_lastFetchedLabel->setText("Last fetched: " + lastFetched);
}

void TitleBar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - parentWidget()->frameGeometry().topLeft();
        event->accept();
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        parentWidget()->move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit maximizeClicked();
    }
}