#include "clone_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QStandardPaths>

namespace commitly {

CloneDialog::CloneDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Clone Repository");
    setMinimumWidth(500);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("Repository URL (HTTPS or SSH):", this));
    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText("https://github.com/owner/repo.git");
    layout->addWidget(m_urlEdit);

    layout->addWidget(new QLabel("Local Path:", this));
    QHBoxLayout *pathLayout = new QHBoxLayout();
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/GitHub");
    pathLayout->addWidget(m_pathEdit);
    
    QPushButton *browseBtn = new QPushButton("Browse...", this);
    pathLayout->addWidget(browseBtn);
    layout->addLayout(pathLayout);

    QHBoxLayout *btns = new QHBoxLayout();
    btns->addStretch();
    QPushButton *cancel = new QPushButton("Cancel", this);
    QPushButton *clone = new QPushButton("Clone", this);
    clone->setDefault(true);
    clone->setStyleSheet("background-color: #2a82da; color: white; padding: 5px 20px;");
    
    btns->addWidget(cancel);
    btns->addWidget(clone);
    layout->addLayout(btns);

    connect(browseBtn, &QPushButton::clicked, this, &CloneDialog::onBrowseClicked);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(clone, &QPushButton::clicked, this, &QDialog::accept);
}

QString CloneDialog::url() const { return m_urlEdit->text(); }
QString CloneDialog::path() const { return m_pathEdit->text(); }

void CloneDialog::onBrowseClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Clone Destination");
    if (!dir.isEmpty()) {
        m_pathEdit->setText(dir);
    }
}

} // namespace commitly
