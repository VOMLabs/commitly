#include "repository_list_panel.h"
#include <QVBoxLayout>
#include <QLabel>
#include <git_bridge.h> // We'll need a wrapper for Rust FFI later

namespace commitly {

RepositoryListPanel::RepositoryListPanel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *header = new QLabel("REPOSITORIES", this);
    header->setStyleSheet("font-weight: bold; padding: 10px; color: #888;");
    layout->addWidget(header);

    m_list = new QListWidget(this);
    m_list->setObjectName("RepoList");
    layout->addWidget(m_list);

    // Dummy data for now
    m_list->addItem("commitly (active)");
    m_list->item(0)->setData(Qt::UserRole, "c:/Users/mateo/Documents/GitHub/VOMLabs/commitly");

    connect(m_list, &QListWidget::itemDoubleClicked, this, &RepositoryListPanel::onItemDoubleClicked);
}

void RepositoryListPanel::onItemDoubleClicked(QListWidgetItem *item)
{
    emit repositorySelected(item->data(Qt::UserRole).toString());
}

} // namespace commitly
