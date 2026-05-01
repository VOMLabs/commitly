#include "toolbar_widget.h"
#include <QHBoxLayout>
#include <QFileInfo>

namespace commitly {

ToolbarWidget::ToolbarWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(50);
    setStyleSheet("background-color: #2d2d2d; border-bottom: 1px solid #444;");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);

    m_repoName = new QLabel("No Repository Selected", this);
    m_repoName->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(m_repoName);

    layout->addStretch();

    m_branchSelector = new QComboBox(this);
    m_branchSelector->setMinimumWidth(150);
    layout->addWidget(m_branchSelector);

    m_syncBtn = new QPushButton("Sync Branch", this);
    m_syncBtn->setStyleSheet("background-color: #2a82da; color: white; padding: 5px 15px; border-radius: 3px;");
    layout->addWidget(m_syncBtn);
}

void ToolbarWidget::setRepositoryPath(const QString &path)
{
    QFileInfo info(path);
    m_repoName->setText(info.fileName());
    
    // In a real impl, we'd call Rust core to get branches here
    m_branchSelector->clear();
    m_branchSelector->addItem("main");
}

} // namespace commitly
