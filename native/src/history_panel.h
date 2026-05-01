#pragma once

#include <QWidget>
#include <QTableView>
#include <QAbstractTableModel>
#include <vector>

namespace commitly {

struct CommitDisplay {
    QString sha;
    QString summary;
    QString author;
    QString date;
};

class CommitListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CommitListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setCommits(const std::vector<CommitDisplay> &commits);
    QString getShaAt(int row) const;

private:
    std::vector<CommitDisplay> m_commits;
};

class HistoryPanel : public QWidget
{
    Q_OBJECT
public:
    HistoryPanel(QWidget *parent = nullptr);

    void loadRepository(const QString &path);

signals:
    void commitSelected(const QString &sha);

private slots:
    void onSelectionChanged();

private:
    QTableView      *m_view;
    CommitListModel *m_model;
};

} // namespace commitly
