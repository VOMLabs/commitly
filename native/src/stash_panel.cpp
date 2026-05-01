#include "stash_panel.h"
#include <QVBoxLayout>
#include <QLabel>

namespace commitly {

StashPanel::StashPanel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel("STASH", this));
    m_list = new QListWidget(this);
    layout->addWidget(m_list);
}

void StashPanel::loadRepository(const QString &path) {
    m_list->clear();
    // Later: call commitly_core_list_stash
    m_list->addItem("stash@{0}: WIP on main");
}

} // namespace commitly
