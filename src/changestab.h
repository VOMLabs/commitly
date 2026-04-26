#ifndef CHANGESTAB_H
#define CHANGESTAB_H

#include <QWidget>
#include <QListWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include "models.h"
#include "diffviewer.h"

class ChangesTab : public QWidget {
    Q_OBJECT

public:
    explicit ChangesTab(DiffViewer *diffViewer, QWidget *parent = nullptr);
    QListWidget* fileListWidget() const { return m_fileList; }

signals:
    void fileSelected(const QString &filePath);

public slots:
    void selectNextFile();
    void selectPreviousFile();

private slots:
    void onFileClicked(QListWidgetItem *item);
    void onStagingChanged();
    void onCommitClicked();
    void onGenerateSummaryClicked();
    void onGenerateDescriptionClicked();

private:
    QList<FileEntry> m_files;
    QListWidget *m_fileList;
    QCheckBox *m_selectAllCheck;
    DiffViewer *m_diffViewer;

    // Commit message area
    QLineEdit *m_summaryInput;
    QTextEdit *m_descInput;
    QPushButton *m_commitBtn;

    void populateFileList();
    void updateCommitButton();
    QString generateAISummary();
    QString generateAIDescription();
};

#endif // CHANGESTAB_H