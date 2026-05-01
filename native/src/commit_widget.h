#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>

namespace commitly {

class CommitWidget : public QWidget
{
    Q_OBJECT
public:
    CommitWidget(QWidget *parent = nullptr);

private slots:
    void onCommitClicked();

private:
    QLineEdit *m_summary;
    QTextEdit *m_description;
    QPushButton *m_commitBtn;
};

} // namespace commitly
