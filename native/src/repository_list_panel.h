#pragma once

#include <QWidget>
#include <QListWidget>

namespace commitly {

class RepositoryListPanel : public QWidget
{
    Q_OBJECT

public:
    RepositoryListPanel(QWidget *parent = nullptr);

signals:
    void repositorySelected(const QString &path);

private slots:
    void onItemDoubleClicked(QListWidgetItem *item);

private:
    QListWidget *m_list;
};

} // namespace commitly
