# Architecture

## High-Level Structure

commitly follows a simple **component-based hierarchy** where a single `MainWindow` widget composes all visual elements. There is no MVC or MVVM framework — each widget is self-contained and communicates through Qt signals and slots.

```
QApplication
  └── MainWindow (QWidget, frameless)
        ├── TitleBar (QWidget)
        │     ├── QLabel (branch name)
        │     ├── QLabel (last fetched)
        │     ├── QPushButton (refresh, minimize, maximize, close)
        │     └── Custom drag handling via mousePressEvent / mouseMoveEvent
        ├── QHBoxLayout (content area)
        │     ├── Sidebar (QWidget, fixed 300px width)
        │     │     ├── QVBoxLayout
        │     │     │     ├── Tab buttons (Changes / History)
        │     │     │     ├── QStackedWidget
        │     │     │     │     ├── ChangesTab (index 0)
        │     │     │     │     │     ├── QCheckBox (Select all)
        │     │     │     │     │     ├── QListWidget (file list)
        │     │     │     │     │     └── Commit message area
        │     │     │     │     │           ├── QLineEdit (summary)
        │     │     │     │     │           ├── QTextEdit (description)
        │     │     │     │     │           └── QPushButton (commit)
        │     │     │     │     └── HistoryTab (index 1)
        │     │     │     │           ├── QLineEdit (search)
        │     │     │     │           └── QListWidget (commit list)
        │     │     │     └── SyncHeader (footer)
        │     │     │           ├── Push indicator (QLabel)
        │     │     │           └── Pull indicator (QLabel)
        │     └── QStackedLayout (right panel)
        │           ├── DiffViewer (index 0)
        │           │     ├── QLabel (file name header)
        │           │     ├── QScrollArea
        │           │     │     └── QWidget (diff content)
        │           │     │           └── Per-line widgets with line numbers, prefix, content
        │           │     └── Options buttons (settings, maximize)
        │           └── CommitDetailView (index 1)
        │                 ├── Empty state label
        │                 └── QScrollArea (per-commit content)
        │                       ├── Summary, author, hash, timestamp
        │                       ├── File list
        │                       └── Inline diff preview
```

## Data Flow

### Changes Tab → Diff Viewer (file diff)

1. `Sidebar` creates `ChangesTab` with a pointer to the `DiffViewer`.
2. When a file is clicked in the `QListWidget`, `ChangesTab::onFileClicked()` is called.
3. It retrieves the file path from the item's `Qt::UserRole` data.
4. It calls `MockData::mockDiffForFile(filePath)` to generate a `FileDiff`.
5. It passes the `FileDiff` to `m_diffViewer->showDiff(diff)`.
6. It emits `fileSelected(filePath)` signal (currently not connected to any consumer).

### History Tab → Commit Detail View

1. `Sidebar` creates `HistoryTab` with a pointer to `CommitDetailView`.
2. When a commit is clicked in the `QListWidget`, `HistoryTab::onCommitClicked()` is called.
3. It retrieves the commit hash from the item's `Qt::UserRole` data.
4. It finds the matching `CommitEntry` in `m_filteredCommits`.
5. It calls `m_detailView->showCommit(commit)`.
6. The `CommitDetailView` then calls `MockData::mockDiffForCommit(commit)` for inline diff rendering.

### Tab Switching

1. User clicks "Changes" or "History" buttons in the `Sidebar`.
2. The button sets its checked state, updates the `QStackedWidget` index, and emits `tabChanged(int)`.
3. `MainWindow` receives `tabChanged` and updates the right-panel `QStackedLayout`:
   - Index 0 (`Changes` tab): shows `DiffViewer`
   - Index 1 (`History` tab): shows `CommitDetailView`
4. `MainWindow` also receives keyboard shortcuts (`Ctrl+Left`/`Ctrl+Right`) and delegates to `Sidebar::switchToChanges()` / `Sidebar::switchToHistory()`.

### Window Controls

- `TitleBar` emits `minimizeClicked()`, `maximizeClicked()`, `closeClicked()`, and `refreshClicked()` signals.
- `MainWindow` connects these to `window()->showMinimized()`, `showMaximized()`/`showNormal()`, `QApplication::quit()`, and a handler that updates the "last fetched" label.
- Window dragging is handled by `TitleBar::mousePressEvent` and `mouseMoveEvent`, which move the parent widget (`MainWindow`) by the drag delta.
- Double-clicking the title bar emits `maximizeClicked()`.

### Staging & Commit Flow

1. Files are loaded from `MockData::mockFiles()` on construction.
2. The "Select all" checkbox toggles all files' `staged` state and calls `updateCommitButton()`.
3. The commit button is enabled only when `stagedCount > 0` AND the summary field is non-empty.
4. On commit click: fields are cleared, staging is reset, the button shows "Committed ✓" for 2 seconds via `QTimer::singleShot`.
5. No data persists — the mock files are regenerated on next construction.

### AI Generation (Simulated)

- The ✨ buttons in `ChangesTab` call `onGenerateSummaryClicked()` / `onGenerateDescriptionClicked()`.
- These set placeholder text ("Generating..."), then use `QTimer::singleShot` with 1500ms / 2000ms delays.
- After the delay, hardcoded placeholder responses are inserted.
- `generateAISummary()` and `generateAIDescription()` are member functions that would be replaced with real AI integration in the future.

## Signal/Slot Connection Map

| Sender | Signal | Receiver | Slot/Action |
|--------|--------|----------|-------------|
| `TitleBar` | `minimizeClicked` | `MainWindow` | `showMinimized()` |
| `TitleBar` | `maximizeClicked` | `MainWindow` | `showMaximized()` / `showNormal()` |
| `TitleBar` | `closeClicked` | `MainWindow` | `QApplication::quit()` |
| `TitleBar` | `refreshClicked` | `MainWindow` | Update last-fetched label |
| `Sidebar` | `tabChanged(int)` | `MainWindow` | Switch `QStackedLayout` index |
| `QShortcut (Ctrl+Right)` | `activated` | `MainWindow` | `Sidebar::switchToHistory()` |
| `QShortcut (Ctrl+Left)` | `activated` | `MainWindow` | `Sidebar::switchToChanges()` |
| `QCheckBox (Select all)` | `toggled` | `ChangesTab` | `onStagingChanged()` |
| `QListWidget (files)` | `itemClicked` | `ChangesTab` | `onFileClicked()` → `DiffViewer::showDiff()` |
| `QPushButton (commit)` | `clicked` | `ChangesTab` | `onCommitClicked()` |
| `QPushButton (✨ summary)` | `clicked` | `ChangesTab` | `onGenerateSummaryClicked()` |
| `QPushButton (✨ desc)` | `clicked` | `ChangesTab` | `onGenerateDescriptionClicked()` |
| `QListWidget (commits)` | `itemClicked` | `HistoryTab` | `onCommitClicked()` → `CommitDetailView::showCommit()` |
| `QLineEdit (search)` | `textChanged` | `HistoryTab` | `onSearchChanged()` |
| `QPushButton (Changes tab)` | `clicked` | `Sidebar` | Switch to index 0, emit `tabChanged(0)` |
| `QPushButton (History tab)` | `clicked` | `Sidebar` | Switch to index 1, emit `tabChanged(1)` |

## Design Decisions

1. **Frameless Window**: `Qt::FramelessWindowHint` is set on `MainWindow` to allow a fully custom title bar. The `TitleBar` widget handles drag-to-move and double-click-to-maximize.

2. **Stacked Layouts**: Two stacked widgets are used: one in `Sidebar` for the Changes/History tab content, and one in `MainWindow` for the right panel (DiffViewer/CommitDetailView). They switch in sync via the `tabChanged` signal.

3. **Mock Data as Namespace**: Mock data is provided through a `MockData` namespace with free functions rather than a class, keeping it stateless and easily replaceable with real Git integration.

4. **Qt6 with C++17**: Enables use of `QStringLiteral`, structured bindings, and `if constexpr` where needed. The build uses `CMAKE_AUTOMOC` for automatic Meta-Object Compilation of Q_OBJECT classes.

5. **Inline Stylesheets**: Each widget defines its stylesheet as a `static const QString` at the top of its `.cpp` file. This keeps styling co-located with widget implementation. The `MainWindow` also sets a global application stylesheet for scrollbars and tooltips.

6. **Emoji-Based Icons**: Instead of using icon files or Qt's built-in icons, the application uses Unicode emoji characters (folder emoji for app icon, up/down arrows for sync, sparkle for AI, etc.) rendered directly in text labels and buttons.

## Extension Points

| Feature | Approach |
|---------|----------|
| Real Git integration | Replace `MockData` namespace calls with libgit2 or QProcess-based `git` command execution |
| Live AI generation | Replace `generateAISummary()` / `generateAIDescription()` with API calls |
| Live sync status | Update `SyncHeader` counts by querying `git remote` state |
| File staging persistence | Replace mock `FileEntry` manipulation with `git add` / `git reset` |
| Commit persistence | Replace mock commit flow with `git commit` |
| Diff loading | Replace `mockDiffForFile` / `mockDiffForCommit` with `git diff` output parsing |