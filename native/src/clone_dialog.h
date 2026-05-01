#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

namespace commitly {

class CloneDialog : public QDialog
{
    Q_OBJECT
public:
    CloneDialog(QWidget *parent = nullptr);

    QString url() const;
    QString path() const;

private slots:
    void onBrowseClicked();

private:
    QLineEdit *m_urlEdit;
    QLineEdit *m_pathEdit;
};

} // namespace commitly
