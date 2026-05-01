#include "changes_panel.h"
#include "git_worker.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QThreadPool>
#include <rust/cxx.h>

namespace commitly {

ChangesPanel::ChangesPanel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout->addWidget(splitter);

    QWidget *unstagedContainer = new QWidget(this);
    QVBoxLayout *unstagedLayout = new QVBoxLayout(unstagedContainer);
    unstagedLayout->addWidget(new QLabel("UNSTAGED CHANGES", this));
    m_unstagedList = new QListWidget(this);
    unstagedLayout->addWidget(m_unstagedList);
    splitter->addWidget(unstagedContainer);

    QWidget *stagedContainer = new QWidget(this);
    QVBoxLayout *stagedLayout = new QVBoxLayout(stagedContainer);
    stagedLayout->addWidget(new QLabel("STAGED CHANGES", this));
    m_stagedList = new QListWidget(this);
    stagedLayout->addWidget(m_stagedList);
    splitter->addWidget(stagedContainer);
}

void ChangesPanel::loadRepository(const QString &path) {
    QThreadPool::globalInstance()->start(new GitWorker<rust::Vec<FfiStatusEntry>>([=]() {
        return core_repo_status(path.toStdString());
    }, [=](rust::Vec<FfiStatusEntry> status) {
        m_unstagedList->clear();
        m_stagedList->clear();
        for (const auto &e : status) {
            QString filePath = QString::fromStdString(std::string(e.path));
            if (!std::string(e.index_status).empty() && std::string(e.index_status) != "untracked") {
                m_stagedList->addItem(filePath);
            } else {
                m_unstagedList->addItem(filePath);
            }
        }
    }));
}

} // namespace commitly
