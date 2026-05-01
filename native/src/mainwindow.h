#pragma once

#include <QMainWindow>
#include <memory>

namespace commitly {

class RepositoryListPanel;
class ToolbarWidget;
class HistoryPanel;
class ChangesPanel;
class DiffViewer;
class CommitWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRepositorySelected(const QString &path);
    void onCommitSelected(const QString &sha);

private:
    void setupUi();

    RepositoryListPanel *m_repoList;
    ToolbarWidget       *m_toolbar;
    HistoryPanel        *m_history;
    ChangesPanel        *m_changes;
    DiffViewer          *m_diffViewer;
    CommitWidget        *m_commitWidget;

    QString m_currentRepoPath;
};

} // namespace commitly
