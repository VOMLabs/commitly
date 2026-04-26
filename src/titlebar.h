#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class TitleBar : public QWidget {
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);
    void setBranchName(const QString &branch);
    void setLastFetched(const QString &lastFetched);

signals:
    void minimizeClicked();
    void maximizeClicked();
    void closeClicked();
    void refreshClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QLabel *m_branchLabel;
    QLabel *m_lastFetchedLabel;
    QPushButton *m_refreshBtn;
    QPushButton *m_minimizeBtn;
    QPushButton *m_maximizeBtn;
    QPushButton *m_closeBtn;
    QPoint m_dragPosition;
};

#endif // TITLEBAR_H