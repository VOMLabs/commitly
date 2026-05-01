#[cxx::bridge(namespace = "commitly")]
pub mod ffi {
    // ── Shared structs (Plain-Old-Data, CXX copies them) ──────────────────

    struct FfiCommit {
        sha:         String,
        short_sha:   String,
        summary:     String,
        body:        String,
        author_name: String,
        author_email: String,
        author_date:  String,
        parent_count: u32,
        tags:         Vec<String>,
    }

    struct FfiBranch {
        name:     String,
        ref_name: String,
        tip_sha:  String,
        upstream: String, // empty if None
        is_remote: bool,
    }

    struct FfiStatusEntry {
        path:            String,
        old_path:        String, // empty if None
        index_status:    String,
        worktree_status: String,
    }

    struct FfiDiffLine {
        kind:       String, // "context" | "addition" | "deletion" | "header"
        content:    String,
        old_lineno: i64, // -1 if None
        new_lineno: i64, // -1 if None
    }

    struct FfiDiffHunk {
        header: String,
        lines:  Vec<FfiDiffLine>,
    }

    struct FfiFileDiff {
        old_path:  String,
        new_path:  String,
        hunks:     Vec<FfiDiffHunk>,
        is_binary: bool,
    }

    struct FfiStashEntry {
        index:   u64,
        message: String,
        sha:     String,
    }

    struct FfiRemote {
        name: String,
        url:  String,
    }

    // ── Rust functions exposed to C++ ─────────────────────────────────────
    extern "Rust" {
        // Status
        fn core_repo_status(path: &str) -> Result<Vec<FfiStatusEntry>>;

        // Log
        fn core_repo_log(path: &str, max_count: u32) -> Result<Vec<FfiCommit>>;

        // Diff
        fn core_commit_diff(path: &str, sha: &str) -> Result<Vec<FfiFileDiff>>;
        fn core_index_diff(path: &str)              -> Result<Vec<FfiFileDiff>>;
        fn core_workdir_diff(path: &str)            -> Result<Vec<FfiFileDiff>>;

        // Branch
        fn core_list_branches(path: &str)                             -> Result<Vec<FfiBranch>>;
        fn core_current_branch(path: &str)                            -> Result<FfiBranch>;
        fn core_create_branch(path: &str, name: &str, from: &str)    -> Result<FfiBranch>;
        fn core_delete_branch(path: &str, name: &str)                 -> Result<()>;
        fn core_rename_branch(path: &str, old: &str, new: &str)       -> Result<FfiBranch>;
        fn core_checkout_branch(path: &str, name: &str)               -> Result<()>;

        // Commit
        fn core_stage_paths(path: &str, paths: &[&str])                    -> Result<()>;
        fn core_unstage_paths(path: &str, paths: &[&str])                  -> Result<()>;
        fn core_create_commit(path: &str, summary: &str, body: &str)       -> Result<FfiCommit>;
        fn core_amend_commit(path: &str, summary: &str, body: &str)        -> Result<FfiCommit>;

        // Remote / fetch / push / pull
        fn core_list_remotes(path: &str)                              -> Result<Vec<FfiRemote>>;
        fn core_add_remote(path: &str, name: &str, url: &str)        -> Result<FfiRemote>;
        fn core_remove_remote(path: &str, name: &str)                 -> Result<()>;
        fn core_fetch(path: &str, remote: &str)                       -> Result<()>;
        fn core_pull(path: &str, remote: &str, branch: &str)         -> Result<()>;
        fn core_push(path: &str, remote: &str, branch: &str)         -> Result<()>;

        // Clone
        fn core_clone_repo(url: &str, dest: &str) -> Result<String>;

        // Stash
        fn core_list_stash(path: &str)                    -> Result<Vec<FfiStashEntry>>;
        fn core_stash_push(path: &str, message: &str)     -> Result<FfiStashEntry>;
        fn core_stash_pop(path: &str, index: u64)         -> Result<()>;
        fn core_stash_drop(path: &str, index: u64)        -> Result<()>;

        // VM
        fn core_run_bytecode(
            bytecode: &[u8],
            hook_name: &str,
            staged_files: Vec<String>,
            commit_summary: &str,
        ) -> String; // "ok" | "rejected:<msg>" | "error:<msg>"
    }
}

// ── Implementation ─────────────────────────────────────────────────────────

use crate::{
    git::{branch, clone, commit, diff, log, remote, stash, status},
    models::{DiffLineKind, FileStatus},
    vm::{HookContext, VmResult},
};

fn file_status_str(s: FileStatus) -> String {
    match s {
        FileStatus::New        => "new",
        FileStatus::Modified   => "modified",
        FileStatus::Deleted    => "deleted",
        FileStatus::Renamed    => "renamed",
        FileStatus::Copied     => "copied",
        FileStatus::Conflicted => "conflicted",
        FileStatus::Untracked  => "untracked",
        FileStatus::Ignored    => "ignored",
    }
    .into()
}

fn commit_to_ffi(c: crate::models::Commit) -> ffi::FfiCommit {
    ffi::FfiCommit {
        sha:          c.sha,
        short_sha:    c.short_sha,
        summary:      c.summary,
        body:         c.body,
        author_name:  c.author.name,
        author_email: c.author.email,
        author_date:  c.author.when.to_rfc3339(),
        parent_count: c.parent_shas.len() as u32,
        tags:         c.tags,
    }
}

pub fn core_repo_status(path: &str) -> Result<Vec<ffi::FfiStatusEntry>, cxx::Exception> {
    status::repo_status(path)
        .map(|v| {
            v.into_iter()
                .map(|e| ffi::FfiStatusEntry {
                    path:            e.path,
                    old_path:        e.old_path.unwrap_or_default(),
                    index_status:    file_status_str(e.index_status),
                    worktree_status: file_status_str(e.worktree_status),
                })
                .collect()
        })
        .map_err(|e| cxx::Exception::new(e.to_string()))
}

pub fn core_repo_log(path: &str, max_count: u32) -> Result<Vec<ffi::FfiCommit>, cxx::Exception> {
    log::repo_log(path, max_count as usize)
        .map(|v| v.into_iter().map(commit_to_ffi).collect())
        .map_err(|e| cxx::Exception::new(e.to_string()))
}

fn hunks_to_ffi(hunks: Vec<crate::models::DiffHunk>) -> Vec<ffi::FfiDiffHunk> {
    hunks
        .into_iter()
        .map(|h| ffi::FfiDiffHunk {
            header: h.header,
            lines: h.lines.into_iter().map(|l| ffi::FfiDiffLine {
                kind: match l.kind {
                    DiffLineKind::Addition => "addition",
                    DiffLineKind::Deletion => "deletion",
                    DiffLineKind::Header   => "header",
                    DiffLineKind::Context  => "context",
                }.into(),
                content:    l.content,
                old_lineno: l.old_lineno.map(|n| n as i64).unwrap_or(-1),
                new_lineno: l.new_lineno.map(|n| n as i64).unwrap_or(-1),
            }).collect(),
        })
        .collect()
}

fn filediffs_to_ffi(diffs: Vec<crate::models::FileDiff>) -> Vec<ffi::FfiFileDiff> {
    diffs.into_iter().map(|d| ffi::FfiFileDiff {
        old_path:  d.old_path.unwrap_or_default(),
        new_path:  d.new_path.unwrap_or_default(),
        hunks:     hunks_to_ffi(d.hunks),
        is_binary: d.is_binary,
    }).collect()
}

pub fn core_commit_diff(path: &str, sha: &str) -> Result<Vec<ffi::FfiFileDiff>, cxx::Exception> {
    diff::commit_diff(path, sha).map(filediffs_to_ffi).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_index_diff(path: &str) -> Result<Vec<ffi::FfiFileDiff>, cxx::Exception> {
    diff::index_diff(path).map(filediffs_to_ffi).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_workdir_diff(path: &str) -> Result<Vec<ffi::FfiFileDiff>, cxx::Exception> {
    diff::workdir_diff(path).map(filediffs_to_ffi).map_err(|e| cxx::Exception::new(e.to_string()))
}

fn branch_to_ffi(b: crate::models::Branch) -> ffi::FfiBranch {
    ffi::FfiBranch {
        name:      b.name,
        ref_name:  b.ref_name,
        tip_sha:   b.tip_sha,
        upstream:  b.upstream.unwrap_or_default(),
        is_remote: b.kind == crate::models::BranchType::Remote,
    }
}

pub fn core_list_branches(path: &str) -> Result<Vec<ffi::FfiBranch>, cxx::Exception> {
    branch::list_branches(path).map(|v| v.into_iter().map(branch_to_ffi).collect()).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_current_branch(path: &str) -> Result<ffi::FfiBranch, cxx::Exception> {
    branch::current_branch(path)
        .map_err(|e| cxx::Exception::new(e.to_string()))
        .and_then(|opt| opt.map(branch_to_ffi).ok_or_else(|| cxx::Exception::new("detached HEAD".into())))
}
pub fn core_create_branch(path: &str, name: &str, from: &str) -> Result<ffi::FfiBranch, cxx::Exception> {
    let from_opt = if from.is_empty() { None } else { Some(from) };
    branch::create_branch(path, name, from_opt).map(branch_to_ffi).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_delete_branch(path: &str, name: &str) -> Result<(), cxx::Exception> {
    branch::delete_branch(path, name).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_rename_branch(path: &str, old: &str, new: &str) -> Result<ffi::FfiBranch, cxx::Exception> {
    branch::rename_branch(path, old, new).map(branch_to_ffi).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_checkout_branch(path: &str, name: &str) -> Result<(), cxx::Exception> {
    branch::checkout_branch(path, name).map_err(|e| cxx::Exception::new(e.to_string()))
}

pub fn core_stage_paths(path: &str, paths: &[&str]) -> Result<(), cxx::Exception> {
    commit::stage_paths(path, paths).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_unstage_paths(path: &str, paths: &[&str]) -> Result<(), cxx::Exception> {
    commit::unstage_paths(path, paths).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_create_commit(path: &str, summary: &str, body: &str) -> Result<ffi::FfiCommit, cxx::Exception> {
    let body_opt = if body.is_empty() { None } else { Some(body) };
    commit::create_commit(path, summary, body_opt).map(commit_to_ffi).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_amend_commit(path: &str, summary: &str, body: &str) -> Result<ffi::FfiCommit, cxx::Exception> {
    let body_opt = if body.is_empty() { None } else { Some(body) };
    commit::amend_commit(path, summary, body_opt).map(commit_to_ffi).map_err(|e| cxx::Exception::new(e.to_string()))
}

pub fn core_list_remotes(path: &str) -> Result<Vec<ffi::FfiRemote>, cxx::Exception> {
    remote::list_remotes(path).map(|v| v.into_iter().map(|r| ffi::FfiRemote { name: r.name, url: r.url }).collect()).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_add_remote(path: &str, name: &str, url: &str) -> Result<ffi::FfiRemote, cxx::Exception> {
    remote::add_remote(path, name, url).map(|r| ffi::FfiRemote { name: r.name, url: r.url }).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_remove_remote(path: &str, name: &str) -> Result<(), cxx::Exception> {
    remote::remove_remote(path, name).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_fetch(path: &str, remote_name: &str) -> Result<(), cxx::Exception> {
    remote::fetch(path, remote_name).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_pull(path: &str, remote_name: &str, branch: &str) -> Result<(), cxx::Exception> {
    remote::pull(path, remote_name, branch).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_push(path: &str, remote_name: &str, branch: &str) -> Result<(), cxx::Exception> {
    remote::push(path, remote_name, branch).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_clone_repo(url: &str, dest: &str) -> Result<String, cxx::Exception> {
    clone::clone_repo(url, dest).map_err(|e| cxx::Exception::new(e.to_string()))
}

pub fn core_list_stash(path: &str) -> Result<Vec<ffi::FfiStashEntry>, cxx::Exception> {
    stash::list_stash(path).map(|v| v.into_iter().map(|s| ffi::FfiStashEntry { index: s.index as u64, message: s.message, sha: s.sha }).collect()).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_stash_push(path: &str, message: &str) -> Result<ffi::FfiStashEntry, cxx::Exception> {
    let m = if message.is_empty() { None } else { Some(message) };
    stash::stash_push(path, m).map(|s| ffi::FfiStashEntry { index: s.index as u64, message: s.message, sha: s.sha }).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_stash_pop(path: &str, index: u64) -> Result<(), cxx::Exception> {
    stash::stash_pop(path, index as usize).map_err(|e| cxx::Exception::new(e.to_string()))
}
pub fn core_stash_drop(path: &str, index: u64) -> Result<(), cxx::Exception> {
    stash::stash_drop(path, index as usize).map_err(|e| cxx::Exception::new(e.to_string()))
}

pub fn core_run_bytecode(
    bytecode: &[u8],
    hook_name: &str,
    staged_files: Vec<String>,
    commit_summary: &str,
) -> String {
    let ctx = HookContext {
        hook_name:      hook_name.to_owned(),
        staged_files,
        commit_summary: commit_summary.to_owned(),
    };
    match crate::vm::run_bytecode(bytecode, &ctx) {
        VmResult::Ok             => "ok".into(),
        VmResult::Rejected(msg)  => format!("rejected:{msg}"),
        VmResult::Error(msg)     => format!("error:{msg}"),
    }
}
