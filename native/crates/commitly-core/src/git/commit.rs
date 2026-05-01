use crate::{error::Result, models::Commit};
use git2::{IndexAddOption, Repository, Signature};

/// Stage the given paths (relative to repo root). Pass `&["*"]` to stage all.
pub fn stage_paths(path: &str, paths: &[&str]) -> Result<()> {
    let repo = Repository::open(path)?;
    let mut index = repo.index()?;
    index.add_all(paths.iter().copied(), IndexAddOption::DEFAULT, None)?;
    index.write()?;
    Ok(())
}

/// Unstage the given paths.
pub fn unstage_paths(path: &str, paths: &[&str]) -> Result<()> {
    let repo = Repository::open(path)?;
    let head = repo.head().ok().and_then(|h| h.peel_to_commit().ok());
    match head {
        Some(commit) => {
            repo.reset_default(Some(commit.as_object()), paths.iter().copied())?;
        }
        _ => {
            // No HEAD yet — just remove from index
            let mut index = repo.index()?;
            for p in paths {
                let _ = index.remove_path(std::path::Path::new(p));
            }
            index.write()?;
        }
    }
    Ok(())
}

/// Create a commit from the current index.
pub fn create_commit(path: &str, summary: &str, body: Option<&str>) -> Result<Commit> {
    let repo   = Repository::open(path)?;
    let cfg    = repo.config()?;
    let name   = cfg.get_string("user.name").unwrap_or_else(|_| "Unknown".into());
    let email  = cfg.get_string("user.email").unwrap_or_else(|_| "unknown@example.com".into());
    let sig    = Signature::now(&name, &email)?;

    let mut index  = repo.index()?;
    let tree_oid   = index.write_tree()?;
    let tree       = repo.find_tree(tree_oid)?;

    let message = match body {
        Some(b) if !b.trim().is_empty() => format!("{summary}\n\n{b}"),
        _ => summary.to_owned(),
    };

    let parent_commits: Vec<git2::Commit<'_>> = repo.head()
        .ok()
        .and_then(|h| h.peel_to_commit().ok())
        .into_iter()
        .collect();
    let parents: Vec<&git2::Commit<'_>> = parent_commits.iter().collect();

    let oid = repo.commit(Some("HEAD"), &sig, &sig, &message, &tree, &parents)?;
    let commit = repo.find_commit(oid)?;
    let sha = oid.to_string();
    Ok(Commit {
        short_sha:   sha[..7].to_owned(),
        summary:     commit.summary().unwrap_or("").to_owned(),
        body:        commit.body().unwrap_or("").to_owned(),
        author:      crate::models::CommitIdentity::from_git2(&commit.author()),
        committer:   crate::models::CommitIdentity::from_git2(&commit.committer()),
        parent_shas: commit.parent_ids().map(|id| id.to_string()).collect(),
        tags:        vec![],
        sha,
    })
}

/// Amend the last commit.
pub fn amend_commit(path: &str, summary: &str, body: Option<&str>) -> Result<Commit> {
    let repo = Repository::open(path)?;
    let head = repo.head()?.peel_to_commit()?;
    let cfg  = repo.config()?;
    let name  = cfg.get_string("user.name").unwrap_or_else(|_| "Unknown".into());
    let email = cfg.get_string("user.email").unwrap_or_else(|_| "unknown@example.com".into());
    let sig   = Signature::now(&name, &email)?;

    let mut index  = repo.index()?;
    let tree_oid   = index.write_tree()?;
    let tree       = repo.find_tree(tree_oid)?;

    let message = match body {
        Some(b) if !b.trim().is_empty() => format!("{summary}\n\n{b}"),
        _ => summary.to_owned(),
    };

    let oid = head.amend(Some("HEAD"), Some(&sig), Some(&sig), None, Some(&message), Some(&tree))?;
    let commit = repo.find_commit(oid)?;
    let sha = oid.to_string();
    Ok(Commit {
        short_sha:   sha[..7].to_owned(),
        summary:     commit.summary().unwrap_or("").to_owned(),
        body:        commit.body().unwrap_or("").to_owned(),
        author:      crate::models::CommitIdentity::from_git2(&commit.author()),
        committer:   crate::models::CommitIdentity::from_git2(&commit.committer()),
        parent_shas: commit.parent_ids().map(|id| id.to_string()).collect(),
        tags:        vec![],
        sha,
    })
}
