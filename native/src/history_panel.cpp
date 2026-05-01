#include "history_panel.h"
#include <QVBoxLayout>
#include <QHeaderView>

namespace commitly {

CommitListModel::CommitListModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int CommitListModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_commits.size());
}

int CommitListModel::columnCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : 3;
}

QVariant CommitListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return QVariant();
    switch (section) {
        case 0: return "Summary";
        case 1: return "Author";
        case 2: return "Date";
    }
    return QVariant();
}

QVariant CommitListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) return QVariant();
    const auto &c = m_commits[index.row()];
    switch (index.column()) {
        case 0: return c.summary;
        case 1: return c.author;
        case 2: return c.date;
    }
    return QVariant();
}

void CommitListModel::setCommits(const std::vector<CommitDisplay> &commits) {
    beginResetModel();
    m_commits = commits;
    endResetModel();
}

QString CommitListModel::getShaAt(int row) const {
    if (row < 0 || row >= static_cast<int>(m_commits.size())) return "";
    return m_commits[row].sha;
}

HistoryPanel::HistoryPanel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_view = new QTableView(this);
    m_model = new CommitListModel(this);
    m_view->setModel(m_model);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->horizontalHeader()->setStretchLastSection(true);
    m_view->verticalHeader()->setVisible(false);
    m_view->setShowGrid(false);

    layout->addWidget(m_view);

    connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &HistoryPanel::onSelectionChanged);
}

void HistoryPanel::loadRepository(const QString &path) {
    // Dummy data for now. Later: call commitly_core_repo_log
    std::vector<CommitDisplay> dummy;
    dummy.push_back({"sha1", "Initial commit", "Mateo", "2026-04-30"});
    dummy.push_back({"sha2", "Update README", "Mateo", "2026-04-30"});
    m_model->setCommits(dummy);
}

void HistoryPanel::onSelectionChanged() {
    QModelIndexList selected = m_view->selectionModel()->selectedRows();
    if (!selected.isEmpty()) {
        emit commitSelected(m_model->getShaAt(selected.first().row()));
    }
}

} // namespace commitly
