#include "mainwindow.h"
#include "repository_list_panel.h"
#include "toolbar_widget.h"
#include "history_panel.h"
#include "changes_panel.h"
#include "diff_viewer.h"
#include "commit_widget.h"
#include "git_worker.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QThreadPool>
#include <QMessageBox>

namespace commitly {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    resize(1200, 800);
    setWindowTitle("Commitly");
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_toolbar = new ToolbarWidget(this);
    mainLayout->addWidget(m_toolbar);

    QSplitter *hSplitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(hSplitter);

    m_repoList = new RepositoryListPanel(this);
    hSplitter->addWidget(m_repoList);

    QSplitter *centerSplitter = new QSplitter(Qt::Horizontal, this);
    hSplitter->addWidget(centerSplitter);

    QWidget *middleWidget = new QWidget(this);
    QVBoxLayout *middleLayout = new QVBoxLayout(middleWidget);
    middleLayout->setContentsMargins(0, 0, 0, 0);

    m_history = new HistoryPanel(this);
    m_changes = new ChangesPanel(this);
    m_commitWidget = new CommitWidget(this);

    QSplitter *vSplitter = new QSplitter(Qt::Vertical, this);
    vSplitter->addWidget(m_history);
    vSplitter->addWidget(m_changes);
    vSplitter->addWidget(m_commitWidget);
    vSplitter->setStretchFactor(0, 2);
    vSplitter->setStretchFactor(1, 1);
    vSplitter->setStretchFactor(2, 0);

    middleLayout->addWidget(vSplitter);
    centerSplitter->addWidget(middleWidget);

    m_diffViewer = new DiffViewer(this);
    centerSplitter->addWidget(m_diffViewer);

    hSplitter->setStretchFactor(0, 0);
    hSplitter->setStretchFactor(1, 1);
    centerSplitter->setStretchFactor(0, 0);
    centerSplitter->setStretchFactor(1, 1);

    connect(m_repoList, &RepositoryListPanel::repositorySelected, this, &MainWindow::onRepositorySelected);
    connect(m_history, &HistoryPanel::commitSelected, this, &MainWindow::onCommitSelected);
}

void MainWindow::onRepositorySelected(const QString &path)
{
    m_currentRepoPath = path;
    m_toolbar->setRepositoryPath(path);

    // Async Fetch History
    QThreadPool::globalInstance()->start(new GitWorker<rust::Vec<FfiCommit>>([=]() {
        return core_repo_log(path.toStdString(), 100);
    }, [=](rust::Vec<FfiCommit> logs) {
        std::vector<CommitDisplay> activeLogs;
        for (const auto &c : logs) {
            activeLogs.push_back({
                QString::fromStdString(std::string(c.sha)),
                QString::fromStdString(std::string(c.summary)),
                QString::fromStdString(std::string(c.author_name)),
                QString::fromStdString(std::string(c.author_date))
            });
        }
        m_history->setCommits(activeLogs);
    }, [=](const QString &err) {
        QMessageBox::critical(this, "Git Error", err);
    }));

    m_changes->loadRepository(path);
}

void MainWindow::onCommitSelected(const QString &sha)
{
    QThreadPool::globalInstance()->start(new GitWorker<rust::Vec<FfiFileDiff>>([=]() {
        return core_commit_diff(m_currentRepoPath.toStdString(), sha.toStdString());
    }, [=](rust::Vec<FfiFileDiff> diffs) {
        m_diffViewer->setFileDiffs(diffs);
    }, [=](const QString &err) {
        QMessageBox::critical(this, "Diff Error", err);
    }));
}

} // namespace commitly
