# TODO

## Near-Term (v0.2 – v0.5)

### Git Integration (Replace Mock Data)

- [ ] Remove `mockdata.cpp`/`.h` – no longer needed once real Git is wired
- [ ] Choose Git backend:
      - **Option A:** `QProcess` + `git` CLI – simpler, works everywhere
      - **Option B:** `libgit2` – faster, more control, C API
- [ ] Implement `GitBackend` class wrapping all git operations
- [ ] `GitBackend::status()` – parse `git status --porcelain`, return `QList<FileEntry>`
- [ ] `GitBackend::diff(file)` – `git diff <file>`, parse into `FileDiff`
- [ ] `GitBackend::diffCached(file)` – staged diff
- [ ] `GitBackend::commit(summary, desc, files)` – `git commit`
- [ ] `GitBackend::log(branch, maxCount)` – `git log --format=...`, parse into `QList<CommitEntry>`
- [ ] `GitBackend::logSearch(query)` – `git log --grep`, interactive search
- [ ] `GitBackend::branch()` – current branch, list branches
- [ ] `GitBackend::checkout(branch)` – switch branches
- [ ] `GitBackend::stage(file)` / `GitBackend::unstage(file)` – `git add` / `git reset`
- [ ] `GitBackend::stageAll()` / `GitBackend::unstageAll()`
- [ ] `GitBackend::push(remote, branch)` – `git push`
- [ ] `GitBackend::pull(remote, branch)` – `git pull`
- [ ] `GitBackend::fetch(remote)` – `git fetch`
- [ ] `GitBackend::stash()` / `GitBackend::stashPop()`
- [ ] `GitBackend::clone(url, dir)` – `git clone`
- [ ] `GitBackend::init(dir)` – `git init`

### Repository Management

- [ ] "Clone a repository" dialog
- [ ] "Create a new repository" dialog
- [ ] "Add existing repository" dialog (point to local folder)
- [ ] Recent repositories list in sidebar or start screen
- [ ] Repository switching without restart

### App Structure & UX

- [ ] Start screen / landing page (no repo open)
- [ ] Graceful error handling for missing `git` or non-repo folders
- [ ] Progress bars for long operations (clone, push, pull, fetch)
- [ ] Undo commit (soft reset HEAD~1)
- [ ] Discard changes (git checkout -- <file>)
- [ ] Amend commit
- [ ] Branch creation / deletion UI
- [ ] Merge UI
- [ ] Cherry-pick UI

### Settings / Configuration

- [ ] Settings dialog
- [ ] Git config editor (name, email, signing key, etc.)
- [ ] Default clone directory
- [ ] Theme toggle (dark / light)
- [ ] Font family and size settings

---

## Medium-Term (v0.6 – v0.9)

### GitHub Integration

- [ ] GitHub authentication (PAT / OAuth)
- [ ] Fetch pull requests for current repo
- [ ] PR list tab
- [ ] PR detail view (status checks, review comments)
- [ ] Create PR from current branch
- [ ] View CI/check status
- [ ] GitHub Actions log viewer
- [ ] Issue creation from "create branch" flow
- [ ] GitHub Enterprise support (custom host)

### Diff & Review

- [ ] Syntax highlighting in diff viewer
- [ ] Side-by-side diff mode (vs unified)
- [ ] Inline comment anchors (stub for future review system)
- [ ] Image diff support
- [ ] Binary file handling
- [ ] Whitespace toggle in diffs
- [ ] Word-level diff highlighting

### History / Commit Graph

- [ ] Visual commit graph (branch topology) in history tab
- [ ] Commit filtering by file(s) changed
- [ ] Commit detail: full diff per file in the right panel
- [ ] "Revert this commit" action
- [ ] "Copy commit hash" context menu
- [ ] Tag display

### Performance

- [ ] Lazy-load diffs (only load visible files)
- [ ] Cache `git log` output, invalidate on new commits
- [ ] Background `git fetch` timer
- [ ] Large repo handling (partial file lists, pagination)

---

## Long-Term (v1.0+)

### Platform Polish

- [ ] **Linux (Priority)**
  - [ ] Native desktop notifications (libnotify / D-Bus)
  - [ ] XDG desktop entry and file icon
  - [ ] AppImage / Flatpak packaging
  - [ ] System title bar fallback option (GNOME/KDE)
  - [ ] Wayland support (drag, window positioning)
  - [ ] `$GIT_*` environment variable awareness
- [ ] **macOS**
  - [ ] Native menubar integration
  - [ ] macOS bundle (.app)
  - [ ] Dock badge (commit count / sync status)
  - [ ] Touch Bar support (if applicable)
  - [ ] Keychain integration for GitHub tokens
- [ ] **Windows**
  - [ ] Windows installer (NSIS / WiX)
  - [ ] System title bar fallback option
  - [ ] Credential Manager integration
  - [ ] File association (.gitattributes, .gitignore editing)

### Advanced Features

- [ ] Interactive rebase UI
- [ ] Submodule support
- [ ] LFS support
- [ ] GPG signing UI
- [ ] Sparse checkout
- [ ] Worktree management
- [ ] Git hooks manager
- [ ] Snapshot / restore work-in-progress
- [ ] Diff / patch export

### CI & Testing

- [ ] GitHub Actions CI (build on Linux, macOS, Windows)
- [ ] Unit tests for `GitBackend` (mock git repos)
- [ ] UI smoke tests (Qt Test / Squish)
- [ ] Static analysis (clang-tidy, cppcheck)
- [ ] Packaging automation (AppImage, .dmg, .exe)

### i18n

- [ ] Extract all user-facing strings with `tr()`
- [ ] `.ts` files for community translations
- [ ] Locale auto-detection

---

## Tech Debt / Refactoring

- [ ] Split `mainwindow.cpp` into logical sub-components if it grows too large
- [ ] Centralize stylesheet constants (maybe a `Theme` namespace or class)
- [ ] Replace emoji-based icons with SVG icon set
- [ ] Add DPI / high-DPI awareness (`Qt::AA_EnableHighDpiScaling`)
- [ ] Document signal flow for new contributors
