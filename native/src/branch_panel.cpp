#include "branch_panel.h"
#include "git_worker.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QThreadPool>
#include <rust/cxx.h>

namespace commitly {

BranchPanel::BranchPanel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel("BRANCHES", this));
    m_list = new QListWidget(this);
    layout->addWidget(m_list);

    connect(m_list, &QListWidget::itemClicked, [=](QListWidgetItem *item) {
        emit branchSelected(item->text());
    });
}

void BranchPanel::loadRepository(const QString &path) {
    QThreadPool::globalInstance()->start(new GitWorker<rust::Vec<FfiBranch>>([=]() {
        return core_list_branches(path.toStdString());
    }, [=](rust::Vec<FfiBranch> branches) {
        m_list->clear();
        for (const auto &b : branches) {
            m_list->addItem(QString::fromStdString(std::string(b.name)));
        }
    }));
}

} // namespace commitly
