#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

namespace commitly {

class ToolbarWidget : public QWidget
{
    Q_OBJECT

public:
    ToolbarWidget(QWidget *parent = nullptr);

    void setRepositoryPath(const QString &path);

private:
    QLabel    *m_repoName;
    QComboBox *m_branchSelector;
    QPushButton *m_syncBtn;
};

} // namespace commitly
