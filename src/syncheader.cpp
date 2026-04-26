#include "syncheader.h"
#include <QHBoxLayout>

static const QString SYNC_STYLE = R"(
    QWidget#syncHeader {
        background-color: #121212;
        border-top: 1px solid #1a1a1a;
        padding: 4px 12px;
    }
    QLabel#syncLabel {
        color: #8b949e;
        font-size: 11px;
        padding: 2px 0;
    }
    QLabel#syncIcon {
        font-size: 11px;
        margin-right: 4px;
    }
)";

SyncHeader::SyncHeader(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("syncHeader");
    setStyleSheet(SYNC_STYLE);
    setFixedHeight(28);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 2, 12, 2);
    layout->setSpacing(12);

    // Push indicator
    auto *pushIcon = new QLabel("\xE2\x86\x91"); // ↑
    pushIcon->setStyleSheet("color: #8b949e; font-size: 11px;");
    pushIcon->setObjectName("syncIcon");
    layout->addWidget(pushIcon);

    m_pushLabel = new QLabel("0 pending");
    m_pushLabel->setObjectName("syncLabel");
    layout->addWidget(m_pushLabel);

    // Pull indicator
    auto *pullIcon = new QLabel("\xE2\x86\x93"); // ↓
    pullIcon->setStyleSheet("color: #8b949e; font-size: 11px;");
    pullIcon->setObjectName("syncIcon");
    layout->addWidget(pullIcon);

    m_pullLabel = new QLabel("0 pending");
    m_pullLabel->setObjectName("syncLabel");
    layout->addWidget(m_pullLabel);

    layout->addStretch();
}

void SyncHeader::setPendingPush(int count) {
    m_pushLabel->setText(QString::number(count) + " pending");
}

void SyncHeader::setPendingPull(int count) {
    m_pullLabel->setText(QString::number(count) + " pending");
}