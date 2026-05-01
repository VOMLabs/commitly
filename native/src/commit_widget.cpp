#include "commit_widget.h"
#include "git_worker.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QThreadPool>
#include <QMessageBox>

namespace commitly {

CommitWidget::CommitWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(200);
    setStyleSheet("background-color: #252526; border-top: 1px solid #444;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);

    m_summary = new QLineEdit(this);
    m_summary->setPlaceholderText("Summary");
    m_summary->setStyleSheet("background-color: #3c3c3c; color: white; border: 1px solid #555;");
    layout->addWidget(m_summary);

    m_description = new QTextEdit(this);
    m_description->setPlaceholderText("Description (optional)");
    m_description->setStyleSheet("background-color: #3c3c3c; color: white; border: 1px solid #555;");
    layout->addWidget(m_description);

    m_commitBtn = new QPushButton("Commit to main", this);
    m_commitBtn->setStyleSheet("background-color: #2ea44f; color: white; font-weight: bold; border-radius: 3px; height: 30px;");
    layout->addWidget(m_commitBtn);

    connect(m_commitBtn, &QPushButton::clicked, this, &CommitWidget::onCommitClicked);
}

void CommitWidget::onCommitClicked() {
    QString summary = m_summary->text();
    QString body = m_description->toPlainText();
    if (summary.isEmpty()) return;

    m_commitBtn->setEnabled(false);
    m_commitBtn->setText("Committing...");

    // Later: passing the repo path somehow, assuming it's accessible or we emit signal
    // For now, assume MainWindow handles the actual call or we emit signal
}

} // namespace commitly
