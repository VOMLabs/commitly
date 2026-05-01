#pragma once

#include <QWidget>
#include <QListWidget>
#include <QLabel>

namespace commitly {

class ChangesPanel : public QWidget
{
    Q_OBJECT
public:
    ChangesPanel(QWidget *parent = nullptr);

    void loadRepository(const QString &path);

private:
    QListWidget *m_unstagedList;
    QListWidget *m_stagedList;
};

} // namespace commitly
