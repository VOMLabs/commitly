use crate::{
    error::Result,
    models::{DiffHunk, DiffLine, DiffLineKind, FileDiff},
};
use git2::{DiffFormat, DiffOptions, Repository};

/// Diff a commit against its first parent (or the empty tree for root commits).
pub fn commit_diff(path: &str, sha: &str) -> Result<Vec<FileDiff>> {
    let repo   = Repository::open(path)?;
    let oid    = repo.revparse_single(sha)?.id();
    let commit = repo.find_commit(oid)?;
    let new_tree = commit.tree()?;

    let old_tree = commit
        .parent(0)
        .ok()
        .and_then(|p| p.tree().ok());

    let mut opts = DiffOptions::new();
    opts.context_lines(3).interhunk_lines(0);

    let diff = repo.diff_tree_to_tree(
        old_tree.as_ref(),
        Some(&new_tree),
        Some(&mut opts),
    )?;

    let mut files: Vec<FileDiff> = Vec::new();
    let mut cur_file: Option<FileDiff> = None;
    let mut cur_hunk: Option<DiffHunk> = None;

    diff.print(DiffFormat::Patch, |delta, hunk, line| {
        // New file delta
        let new_path = delta.new_file().path().map(|p| p.to_string_lossy().into_owned());
        let old_path = delta.old_file().path().map(|p| p.to_string_lossy().into_owned());
        let is_binary = delta.new_file().is_binary();

        match cur_file.as_ref() {
            None => {
                cur_file = Some(FileDiff { old_path: old_path.clone(), new_path: new_path.clone(), hunks: vec![], is_binary });
            }
            Some(cf) if cf.new_path != new_path => {
                if let Some(h) = cur_hunk.take() {
                    cur_file.as_mut().unwrap().hunks.push(h);
                }
                files.push(cur_file.take().unwrap());
                cur_file = Some(FileDiff { old_path: old_path.clone(), new_path: new_path.clone(), hunks: vec![], is_binary });
            }
            _ => {}
        }

        if let Some(h) = &hunk {
            let header = String::from_utf8_lossy(h.header()).into_owned();
            match cur_hunk.as_ref() {
                None => cur_hunk = Some(DiffHunk { header: header.clone(), lines: vec![] }),
                Some(ch) if ch.header != header => {
                    let old = cur_hunk.replace(DiffHunk { header, lines: vec![] });
                    if let Some(o) = old {
                        cur_file.as_mut().unwrap().hunks.push(o);
                    }
                }
                _ => {}
            }
        }

        let content = String::from_utf8_lossy(line.content()).into_owned();
        let kind = match line.origin() {
            '+' => DiffLineKind::Addition,
            '-' => DiffLineKind::Deletion,
            'H' | 'F' => DiffLineKind::Header,
            _   => DiffLineKind::Context,
        };
        let dl = DiffLine {
            kind,
            content,
            old_lineno: line.old_lineno(),
            new_lineno: line.new_lineno(),
        };
        if let Some(hunk) = cur_hunk.as_mut() {
            hunk.lines.push(dl);
        }

        true
    })?;

    if let Some(h) = cur_hunk.take() {
        if let Some(cf) = cur_file.as_mut() { cf.hunks.push(h); }
    }
    if let Some(cf) = cur_file.take() {
        files.push(cf);
    }

    Ok(files)
}

/// Diff the index (staged changes) against HEAD.
pub fn index_diff(path: &str) -> Result<Vec<FileDiff>> {
    let repo = Repository::open(path)?;
    let head_tree = repo.head().ok().and_then(|h| {
        h.peel_to_commit().ok().and_then(|c| c.tree().ok())
    });
    let diff = repo.diff_tree_to_index(head_tree.as_ref(), None, None)?;
    diff_to_file_diffs(diff)
}

/// Diff the working directory against the index (unstaged changes).
pub fn workdir_diff(path: &str) -> Result<Vec<FileDiff>> {
    let repo = Repository::open(path)?;
    let diff = repo.diff_index_to_workdir(None, None)?;
    diff_to_file_diffs(diff)
}

fn diff_to_file_diffs(diff: git2::Diff<'_>) -> Result<Vec<FileDiff>> {
    let mut files: Vec<FileDiff> = Vec::new();
    let mut cur_file: Option<FileDiff> = None;
    let mut cur_hunk: Option<DiffHunk> = None;

    diff.print(DiffFormat::Patch, |delta, hunk, line| {
        let new_path = delta.new_file().path().map(|p| p.to_string_lossy().into_owned());
        let old_path = delta.old_file().path().map(|p| p.to_string_lossy().into_owned());
        let is_binary = delta.new_file().is_binary();

        match cur_file.as_ref() {
            None => {
                cur_file = Some(FileDiff { old_path: old_path.clone(), new_path: new_path.clone(), hunks: vec![], is_binary });
            }
            Some(cf) if cf.new_path != new_path => {
                if let Some(h) = cur_hunk.take() {
                    cur_file.as_mut().unwrap().hunks.push(h);
                }
                files.push(cur_file.take().unwrap());
                cur_file = Some(FileDiff { old_path: old_path.clone(), new_path: new_path.clone(), hunks: vec![], is_binary });
            }
            _ => {}
        }

        if let Some(h) = &hunk {
            let header = String::from_utf8_lossy(h.header()).into_owned();
            match cur_hunk.as_ref() {
                None => cur_hunk = Some(DiffHunk { header: header.clone(), lines: vec![] }),
                Some(ch) if ch.header != header => {
                    let old = cur_hunk.replace(DiffHunk { header, lines: vec![] });
                    if let Some(o) = old { cur_file.as_mut().unwrap().hunks.push(o); }
                }
                _ => {}
            }
        }

        let content = String::from_utf8_lossy(line.content()).into_owned();
        let kind = match line.origin() {
            '+' => DiffLineKind::Addition,
            '-' => DiffLineKind::Deletion,
            'H' | 'F' => DiffLineKind::Header,
            _   => DiffLineKind::Context,
        };
        if let Some(hunk) = cur_hunk.as_mut() {
            hunk.lines.push(DiffLine { kind, content, old_lineno: line.old_lineno(), new_lineno: line.new_lineno() });
        }
        true
    })?;

    if let Some(h) = cur_hunk.take() {
        if let Some(cf) = cur_file.as_mut() { cf.hunks.push(h); }
    }
    if let Some(cf) = cur_file.take() { files.push(cf); }

    Ok(files)
}
