use chrono::{DateTime, FixedOffset};
use serde::{Deserialize, Serialize};

// ── CommitIdentity ─────────────────────────────────────────────────────────

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CommitIdentity {
    pub name:   String,
    pub email:  String,
    pub when:   DateTime<FixedOffset>,
}

impl CommitIdentity {
    pub fn from_git2(sig: &git2::Signature<'_>) -> Self {
        let name  = sig.name().unwrap_or("").to_owned();
        let email = sig.email().unwrap_or("").to_owned();
        let time  = sig.when();
        let offset = FixedOffset::east_opt(time.offset_minutes() * 60)
            .unwrap_or(FixedOffset::east_opt(0).unwrap());
        let when = DateTime::from_timestamp(time.seconds(), 0)
            .unwrap_or_default()
            .with_timezone(&offset);
        Self { name, email, when }
    }
}

// ── Commit ─────────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Commit {
    pub sha:         String,
    pub short_sha:   String,
    pub summary:     String,
    pub body:        String,
    pub author:      CommitIdentity,
    pub committer:   CommitIdentity,
    pub parent_shas: Vec<String>,
    pub tags:        Vec<String>,
}

impl Commit {
    pub fn is_merge_commit(&self) -> bool {
        self.parent_shas.len() > 1
    }
}

// ── Branch ────────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum BranchType {
    Local,
    Remote,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Branch {
    pub name:     String,
    pub ref_name: String,
    pub tip_sha:  String,
    pub upstream: Option<String>,
    pub kind:     BranchType,
}

impl Branch {
    pub fn name_without_remote(&self) -> &str {
        match self.kind {
            BranchType::Local => &self.name,
            BranchType::Remote => {
                self.name.find('/').map(|i| &self.name[i + 1..]).unwrap_or(&self.name)
            }
        }
    }
}

// ── AheadBehind ───────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub struct AheadBehind {
    pub ahead:  usize,
    pub behind: usize,
}

// ── Remote ────────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Remote {
    pub name: String,
    pub url:  String,
}

// ── Repository ────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Repository {
    pub id:      u64,
    pub path:    String,
    pub name:    String,
    pub missing: bool,
    pub remotes: Vec<Remote>,
}

// ── File status ───────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum FileStatus {
    New,
    Modified,
    Deleted,
    Renamed,
    Copied,
    Conflicted,
    Untracked,
    Ignored,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StatusEntry {
    pub path:          String,
    pub old_path:      Option<String>,
    pub index_status:  FileStatus,
    pub worktree_status: FileStatus,
}

// ── Diff ──────────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum DiffLineKind {
    Context,
    Addition,
    Deletion,
    Header,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DiffLine {
    pub kind:    DiffLineKind,
    pub content: String,
    pub old_lineno: Option<u32>,
    pub new_lineno: Option<u32>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DiffHunk {
    pub header: String,
    pub lines:  Vec<DiffLine>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FileDiff {
    pub old_path: Option<String>,
    pub new_path: Option<String>,
    pub hunks:    Vec<DiffHunk>,
    pub is_binary: bool,
}

// ── Stash ─────────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StashEntry {
    pub index:   usize,
    pub message: String,
    pub sha:     String,
}
