#include "mockdata.h"
#include <QDateTime>

namespace MockData {

QList<FileEntry> mockFiles() {
    QList<FileEntry> files;

    files.append({"src/components/App.tsx", "App.tsx", FileStatus::Modified, true});
    files.append({"src/utils/helpers.ts", "helpers.ts", FileStatus::Added, false});
    files.append({"src/styles/main.css", "main.css", FileStatus::Modified, false});
    files.append({"docs/README.md", "README.md", FileStatus::Untracked, false});
    files.append({"src/hooks/useAuth.ts", "useAuth.ts", FileStatus::Added, true});
    files.append({"src/components/Navbar.tsx", "Navbar.tsx", FileStatus::Deleted, false});
    files.append({"package.json", "package.json", FileStatus::Modified, true});
    files.append({"src/constants.ts", "constants.ts", FileStatus::Modified, false});
    files.append({".env.example", ".env.example", FileStatus::Untracked, false});
    files.append({"src/types/index.ts", "index.ts", FileStatus::Added, true});
    files.append({"src/utils/api.ts", "api.ts", FileStatus::Modified, false});
    files.append({"src/styles/variables.css", "variables.css", FileStatus::Added, false});

    return files;
}

QList<CommitEntry> mockCommits() {
    QList<CommitEntry> commits;

    commits.append({
        "a1b2c3d4e5f6",
        "Add user authentication flow and protected routes",
        "Implement JWT-based authentication with refresh tokens. Added login, register, and logout endpoints. Protected routes now redirect unauthenticated users to the login page.\n\n- Added auth context and hook\n- Created private route wrapper component\n- Implemented token storage with automatic refresh",
        "Jane Doe",
        "jane@example.com",
        QDateTime::currentDateTime().addSecs(-3600),
        {"src/components/App.tsx", "src/hooks/useAuth.ts", "src/utils/api.ts"}
    });

    commits.append({
        "b2c3d4e5f6a7",
        "Refactor database schema and migration scripts",
        "Updated the database schema to support new user preferences table. Consolidated migration scripts into a single baseline.",
        "John Smith",
        "john@example.com",
        QDateTime::currentDateTime().addSecs(-7200),
        {"src/utils/helpers.ts", "src/types/index.ts"}
    });

    commits.append({
        "c3d4e5f6a7b8",
        "Fix responsive layout issues on mobile devices",
        "Fixed several CSS grid and flexbox issues that caused layout breakage on screens smaller than 768px.",
        "Jane Doe",
        "jane@example.com",
        QDateTime::currentDateTime().addSecs(-14400),
        {"src/styles/main.css", "src/styles/variables.css"}
    });

    commits.append({
        "d4e5f6a7b8c9",
        "Update project dependencies and fix security vulnerabilities",
        "Updated all npm packages to latest compatible versions. Patched critical security vulnerabilities in lodash and axios.",
        "Alex Wang",
        "alex@example.com",
        QDateTime::currentDateTime().addSecs(-28800),
        {"package.json"}
    });

    commits.append({
        "e5f6a7b8c9d0",
        "Initial project setup with React and TypeScript",
        "Scaffolded the project using Create React App with TypeScript template. Configured ESLint, Prettier, and Husky for code quality.",
        "Alex Wang",
        "alex@example.com",
        QDateTime::currentDateTime().addSecs(-86400),
        {"package.json", "src/constants.ts", ".env.example"}
    });

    return commits;
}

FileDiff mockDiffForFile(const QString &filePath) {
    FileDiff diff;
    diff.filePath = filePath;

    // Extract filename from path
    int lastSlash = filePath.lastIndexOf('/');
    diff.fileName = (lastSlash >= 0) ? filePath.mid(lastSlash + 1) : filePath;

    DiffHunk hunk1;
    hunk1.oldStart = 1;
    hunk1.newStart = 1;
    hunk1.oldCount = 10;
    hunk1.newCount = 14;
    hunk1.header = "@@ -1,10 +1,14 @@";

    hunk1.lines.append({DiffLine::Context, 1, 1, "import React from 'react';"});
    hunk1.lines.append({DiffLine::Context, 2, 2, "import { useState } from 'react';"});
    hunk1.lines.append({DiffLine::Context, 3, 3, ""});
    hunk1.lines.append({DiffLine::Removal, 4, -1, "const App = () => {"});
    hunk1.lines.append({DiffLine::Context, 5, 4, "  return ("});
    hunk1.lines.append({DiffLine::Context, 6, 5, "    <div className=\"app\">"});
    hunk1.lines.append({DiffLine::Addition, -1, 6, "      <header className=\"app-header\">"});
    hunk1.lines.append({DiffLine::Context, 7, 7, "        <h1>Hello World</h1>"});
    hunk1.lines.append({DiffLine::Addition, -1, 8, "        <nav>"});
    hunk1.lines.append({DiffLine::Addition, -1, 9, "          <a href=\"/login\">Login</a>"});
    hunk1.lines.append({DiffLine::Addition, -1, 10, "        </nav>"});
    hunk1.lines.append({DiffLine::Addition, -1, 11, "      </header>"});
    hunk1.lines.append({DiffLine::Context, 8, 12, "      <main>"});
    hunk1.lines.append({DiffLine::Context, 9, 13, "        <p>Welcome</p>"});
    hunk1.lines.append({DiffLine::Context, 10, 14, "      </main>"});
    hunk1.lines.append({DiffLine::Context, 11, 15, "    </div>"});
    hunk1.lines.append({DiffLine::Addition, -1, 16, "  );"});
    hunk1.lines.append({DiffLine::Addition, -1, 17, "};"});

    diff.hunks.append(hunk1);

    DiffHunk hunk2;
    hunk2.oldStart = 15;
    hunk2.newStart = 22;
    hunk2.oldCount = 5;
    hunk2.newCount = 8;
    hunk2.header = "@@ -15,5 +22,8 @@";

    hunk2.lines.append({DiffLine::Context, 15, 22, "const App = () => {"});
    hunk2.lines.append({DiffLine::Context, 16, 23, "  const [count, setCount] = useState(0);"});
    hunk2.lines.append({DiffLine::Removal, 17, -1, "  console.log('render');"});
    hunk2.lines.append({DiffLine::Addition, -1, 24, "  useEffect(() => {"});
    hunk2.lines.append({DiffLine::Addition, -1, 25, "    document.title = `Count: ${count}`;"});
    hunk2.lines.append({DiffLine::Addition, -1, 26, "  }, [count]);"});
    hunk2.lines.append({DiffLine::Context, 18, 27, "  return ("});
    hunk2.lines.append({DiffLine::Context, 19, 28, "    <div>"});
    hunk2.lines.append({DiffLine::Addition, -1, 29, "      <p>Count: {count}</p>"});
    hunk2.lines.append({DiffLine::Addition, -1, 30, "      <button onClick={() => setCount(c => c + 1)}>+"});
    hunk2.lines.append({DiffLine::Context, 20, 31, "    </div>"});

    diff.hunks.append(hunk2);

    return diff;
}

FileDiff mockDiffForCommit(const CommitEntry &commit) {
    // Generate a simple diff based on the commit info
    FileDiff diff;

    if (!commit.changedFiles.isEmpty()) {
        diff.filePath = commit.changedFiles.first();
        int lastSlash = diff.filePath.lastIndexOf('/');
        diff.fileName = (lastSlash >= 0) ? diff.filePath.mid(lastSlash + 1) : diff.filePath;

        DiffHunk hunk;
        hunk.oldStart = 1;
        hunk.newStart = 1;
        hunk.oldCount = 5;
        hunk.newCount = 8;
        hunk.header = "@@ -1,5 +1,8 @@";

        hunk.lines.append({DiffLine::Context, 1, 1, "import React from 'react';"});
        hunk.lines.append({DiffLine::Context, 2, 2, ""});
        hunk.lines.append({DiffLine::Removal, 3, -1, "const oldCode = () => {"});
        hunk.lines.append({DiffLine::Removal, 4, -1, "  return 'old implementation';"});
        hunk.lines.append({DiffLine::Removal, 5, -1, "};"});
        hunk.lines.append({DiffLine::Addition, -1, 3, "const newCode = () => {"});
        hunk.lines.append({DiffLine::Addition, -1, 4, "  return 'new implementation with better';"});
        hunk.lines.append({DiffLine::Addition, -1, 5, "  return 'functionality and fixes';"});
        hunk.lines.append({DiffLine::Addition, -1, 6, "};"});
        hunk.lines.append({DiffLine::Context, 6, 7, ""});
        hunk.lines.append({DiffLine::Context, 7, 8, "export default newCode;"});

        diff.hunks.append(hunk);
    }

    return diff;
}

}