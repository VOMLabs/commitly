#ifndef HISTORYTAB_H
#define HISTORYTAB_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include "models.h"
#include "commitdetailview.h"

class HistoryTab : public QWidget {
    Q_OBJECT

public:
    explicit HistoryTab(CommitDetailView *detailView, QWidget *parent = nullptr);
    QListWidget* commitListWidget() const { return m_commitList; }

private slots:
    void onCommitClicked(QListWidgetItem *item);
    void onSearchChanged(const QString &text);

private:
    QList<CommitEntry> m_commits;
    QList<CommitEntry> m_filteredCommits;
    QListWidget *m_commitList;
    QLineEdit *m_searchInput;
    CommitDetailView *m_detailView;

    void populateCommitList(const QList<CommitEntry> &commits);
};

#endif // HISTORYTAB_H