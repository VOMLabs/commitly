#pragma once

#include <QWidget>
#include <QListWidget>

namespace commitly {

class StashPanel : public QWidget
{
    Q_OBJECT
public:
    StashPanel(QWidget *parent = nullptr);

    void loadRepository(const QString &path);

private:
    QListWidget *m_list;
};

} // namespace commitly
