#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include "changestab.h"
#include "historytab.h"
#include "diffviewer.h"
#include "commitdetailview.h"
#include "syncheader.h"

class Sidebar : public QWidget {
    Q_OBJECT

public:
    explicit Sidebar(DiffViewer *diffViewer, CommitDetailView *detailView,
                     QWidget *parent = nullptr);

    void switchToChanges();
    void switchToHistory();
    int currentIndex() const { return m_stack->currentIndex(); }

    ChangesTab* changesTab() const { return m_changesTab; }
    HistoryTab* historyTab() const { return m_historyTab; }

signals:
    void tabChanged(int index);

private:
    QPushButton *m_changesBtn;
    QPushButton *m_historyBtn;
    QStackedWidget *m_stack;
    ChangesTab *m_changesTab;
    HistoryTab *m_historyTab;
    SyncHeader *m_syncHeader;

    void setupTabButtons(QVBoxLayout *layout);
};

#endif // SIDEBAR_H