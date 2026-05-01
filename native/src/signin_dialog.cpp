#include "signin_dialog.h"
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QUrl>

namespace commitly {

SignInDialog::SignInDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Sign in to GitHub");
    setFixedSize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);

    QLabel *icon = new QLabel("GH", this); // Placeholder icon
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("font-size: 48px; border: 2px solid #555; border-radius: 20px;");
    layout->addWidget(icon);

    m_status = new QLabel("Sign in to sync your repositories and open pull requests.", this);
    m_status->setWordWrap(true);
    m_status->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_status);

    m_signInBtn = new QPushButton("Continue with Browser", this);
    m_signInBtn->setStyleSheet("background-color: #2ea44f; color: white; padding: 10px; font-weight: bold; border-radius: 6px;");
    layout->addWidget(m_signInBtn);

    connect(m_signInBtn, &QPushButton::clicked, this, &SignInDialog::onSignInClicked);
}

void SignInDialog::onSignInClicked() {
    // In a real app, we'd start a local HTTP server and open the OAuth URL
    QDesktopServices::openUrl(QUrl("https://github.com/login/oauth/authorize?client_id=PLACEHOLDER"));
    m_status->setText("Check your browser to complete sign in...");
    m_signInBtn->setEnabled(false);
}

} // namespace commitly
