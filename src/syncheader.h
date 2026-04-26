#ifndef SYNCHHEADER_H
#define SYNCHHEADER_H

#include <QWidget>
#include <QLabel>

class SyncHeader : public QWidget {
    Q_OBJECT

public:
    explicit SyncHeader(QWidget *parent = nullptr);
    void setPendingPush(int count);
    void setPendingPull(int count);

private:
    QLabel *m_pushLabel;
    QLabel *m_pullLabel;
};

#endif // SYNCHHEADER_H