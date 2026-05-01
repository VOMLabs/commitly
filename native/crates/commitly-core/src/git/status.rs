use crate::{
    error::Result,
    models::{FileStatus, StatusEntry},
};
use git2::{Repository, Status, StatusOptions};

pub fn repo_status(path: &str) -> Result<Vec<StatusEntry>> {
    let repo = Repository::open(path)?;
    let mut opts = StatusOptions::new();
    opts.include_untracked(true)
        .renames_head_to_index(true)
        .renames_index_to_workdir(true);

    let statuses = repo.statuses(Some(&mut opts))?;
    let entries = statuses
        .iter()
        .filter_map(|entry| {
            let st = entry.status();
            if st.is_ignored() || st == Status::CURRENT {
                return None;
            }
            let path     = entry.path().unwrap_or("").to_owned();
            let old_path = entry.head_to_index()
                .and_then(|d| d.old_file().path())
                .map(|p| p.to_string_lossy().into_owned())
                .filter(|p| p != &path);

            Some(StatusEntry {
                path,
                old_path,
                index_status:   map_status_index(st),
                worktree_status: map_status_worktree(st),
            })
        })
        .collect();

    Ok(entries)
}

fn map_status_index(st: Status) -> FileStatus {
    if st.contains(Status::INDEX_NEW)        { return FileStatus::New; }
    if st.contains(Status::INDEX_MODIFIED)   { return FileStatus::Modified; }
    if st.contains(Status::INDEX_DELETED)    { return FileStatus::Deleted; }
    if st.contains(Status::INDEX_RENAMED)    { return FileStatus::Renamed; }
    if st.contains(Status::INDEX_TYPECHANGE) { return FileStatus::Modified; }
    if st.contains(Status::CONFLICTED)       { return FileStatus::Conflicted; }
    FileStatus::Untracked
}

fn map_status_worktree(st: Status) -> FileStatus {
    if st.contains(Status::WT_NEW)        { return FileStatus::Untracked; }
    if st.contains(Status::WT_MODIFIED)   { return FileStatus::Modified; }
    if st.contains(Status::WT_DELETED)    { return FileStatus::Deleted; }
    if st.contains(Status::WT_RENAMED)    { return FileStatus::Renamed; }
    if st.contains(Status::WT_TYPECHANGE) { return FileStatus::Modified; }
    if st.contains(Status::IGNORED)       { return FileStatus::Ignored; }
    if st.contains(Status::CONFLICTED)    { return FileStatus::Conflicted; }
    FileStatus::Untracked
}
