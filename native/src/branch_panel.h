#pragma once

#include <QWidget>
#include <QListWidget>

namespace commitly {

class BranchPanel : public QWidget
{
    Q_OBJECT
public:
    BranchPanel(QWidget *parent = nullptr);

    void loadRepository(const QString &path);

signals:
    void branchSelected(const QString &name);

private:
    QListWidget *m_list;
};

} // namespace commitly
