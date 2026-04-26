#ifndef COMMITDETAILVIEW_H
#define COMMITDETAILVIEW_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QStackedLayout>
#include "models.h"

class CommitDetailView : public QWidget {
    Q_OBJECT

public:
    explicit CommitDetailView(QWidget *parent = nullptr);
    void showCommit(const CommitEntry &commit);
    void clear();

private:
    QLabel *m_summaryLabel;
    QLabel *m_authorLabel;
    QLabel *m_timestampLabel;
    QLabel *m_hashLabel;
    QLabel *m_descriptionLabel;
    QLabel *m_filesChangedLabel;
    QVBoxLayout *m_filesLayout;
    QWidget *m_filesWidget;
    QWidget *m_contentWidget;
    QStackedLayout *m_stack;
};

#endif // COMMITDETAILVIEW_H