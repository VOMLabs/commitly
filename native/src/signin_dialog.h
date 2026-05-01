#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>

namespace commitly {

class SignInDialog : public QDialog
{
    Q_OBJECT
public:
    SignInDialog(QWidget *parent = nullptr);

signals:
    void authenticated(const QString &token);

private slots:
    void onSignInClicked();

private:
    QLabel *m_status;
    QPushButton *m_signInBtn;
};

} // namespace commitly
