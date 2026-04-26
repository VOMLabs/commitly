#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "titlebar.h"
#include "sidebar.h"
#include "diffviewer.h"
#include "commitdetailview.h"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    TitleBar *m_titleBar;
    Sidebar *m_sidebar;
    DiffViewer *m_diffViewer;
    CommitDetailView *m_detailView;

    void setupUi();
    void setupShortcuts();
};

#endif // MAINWINDOW_H