use crate::{error::Result, models::StashEntry};
use git2::Repository;

pub fn list_stash(path: &str) -> Result<Vec<StashEntry>> {
    let mut repo = Repository::open(path)?;
    let mut entries = Vec::new();
    repo.stash_foreach(|index, message, oid| {
        entries.push(StashEntry {
            index,
            message: message.to_owned(),
            sha:     oid.to_string(),
        });
        true
    })?;
    Ok(entries)
}

pub fn stash_push(path: &str, message: Option<&str>) -> Result<StashEntry> {
    let mut repo = Repository::open(path)?;
    let cfg  = repo.config()?;
    let name  = cfg.get_string("user.name").unwrap_or_else(|_| "Unknown".into());
    let email = cfg.get_string("user.email").unwrap_or_else(|_| "unknown@example.com".into());
    let sig   = git2::Signature::now(&name, &email)?;
    let msg   = message.unwrap_or("WIP on current branch");
    let oid   = repo.stash_save(&sig, msg, None)?;
    Ok(StashEntry { index: 0, message: msg.to_owned(), sha: oid.to_string() })
}

pub fn stash_pop(path: &str, index: usize) -> Result<()> {
    let mut repo = Repository::open(path)?;
    repo.stash_pop(index, None)?;
    Ok(())
}

pub fn stash_drop(path: &str, index: usize) -> Result<()> {
    let mut repo = Repository::open(path)?;
    repo.stash_drop(index)?;
    Ok(())
}
