#ifndef MODELS_H
#define MODELS_H

#include <QString>
#include <QDateTime>
#include <QList>
#include <QStringList>

enum class FileStatus {
    Added,
    Modified,
    Deleted,
    Untracked
};

struct FileEntry {
    QString filePath;
    QString fileName;
    FileStatus status;
    bool staged;

    QString statusIcon() const {
        switch (status) {
            case FileStatus::Added:    return "\xE2\x9C\x94"; // checkmark
            case FileStatus::Modified:  return "\xE2\x9C\x94";
            case FileStatus::Deleted:  return "\xE2\x9C\x98"; // x mark
            case FileStatus::Untracked: return "\xE2\x9E\x95"; // plus
        }
        return "";
    }

    QString statusColor() const {
        switch (status) {
            case FileStatus::Added:    return "#3fb950"; // green
            case FileStatus::Modified:  return "#d29922"; // yellow
            case FileStatus::Deleted:  return "#f85149"; // red
            case FileStatus::Untracked: return "#bc8cff"; // purple
        }
        return "";
    }

    QString statusLabel() const {
        switch (status) {
            case FileStatus::Added:    return "A";
            case FileStatus::Modified:  return "M";
            case FileStatus::Deleted:  return "D";
            case FileStatus::Untracked: return "U";
        }
        return "";
    }
};

struct DiffLine {
    enum Type { Context, Addition, Removal, HunkHeader };
    Type type;
    int oldLineNumber;
    int newLineNumber;
    QString content;
};

struct DiffHunk {
    int oldStart;
    int newStart;
    int oldCount;
    int newCount;
    QString header;
    QList<DiffLine> lines;
};

struct FileDiff {
    QString filePath;
    QString fileName;
    QList<DiffHunk> hunks;
};

struct CommitEntry {
    QString hash;
    QString summary;
    QString description;
    QString author;
    QString authorEmail;
    QDateTime timestamp;
    QStringList changedFiles;
};

#endif // MODELS_H