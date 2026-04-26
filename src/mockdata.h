#ifndef MOCKDATA_H
#define MOCKDATA_H

#include "models.h"
#include <QList>

namespace MockData {

QList<FileEntry> mockFiles();
QList<CommitEntry> mockCommits();
FileDiff mockDiffForFile(const QString &filePath);
FileDiff mockDiffForCommit(const CommitEntry &commit);

}

#endif // MOCKDATA_H