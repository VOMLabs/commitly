#include "preferences_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>

namespace commitly {

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Preferences");
    setMinimumSize(500, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QTabWidget *tabs = new QTabWidget(this);
    layout->addWidget(tabs);

    QWidget *appearance = new QWidget();
    setupAppearanceTab(appearance);
    tabs->addTab(appearance, "Appearance");

    QWidget *gitTab = new QWidget();
    setupGitTab(gitTab);
    tabs->addTab(gitTab, "Git");

    QHBoxLayout *btns = new QHBoxLayout();
    btns->addStretch();
    QPushButton *ok = new QPushButton("Close", this);
    btns->addWidget(ok);
    layout->addLayout(btns);

    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
}

void PreferencesDialog::setupAppearanceTab(QWidget *tab) {
    QVBoxLayout *layout = new QVBoxLayout(tab);
    
    layout->addWidget(new QLabel("Theme:", tab));
    QComboBox *themes = new QComboBox(tab);
    themes->addItem("Dark (Default)");
    themes->addItem("Light");
    layout->addWidget(themes);
    
    layout->addStretch();
}

void PreferencesDialog::setupGitTab(QWidget *tab) {
    QVBoxLayout *layout = new QVBoxLayout(tab);
    
    layout->addWidget(new QLabel("Git User Name:", tab));
    QLineEdit *name = new QLineEdit(tab);
    name->setPlaceholderText("Mateo");
    layout->addWidget(name);

    layout->addWidget(new QLabel("Git User Email:", tab));
    QLineEdit *email = new QLineEdit(tab);
    email->setPlaceholderText("mateo@example.com");
    layout->addWidget(email);

    layout->addStretch();
}

} // namespace commitly
