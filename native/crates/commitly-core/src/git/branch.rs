use crate::{error::Result, models::{Branch, BranchType}};
use git2::{BranchType as G2BranchType, Repository};

pub fn list_branches(path: &str) -> Result<Vec<Branch>> {
    let repo = Repository::open(path)?;
    let branches = repo.branches(None)?;
    let mut out = Vec::new();

    for item in branches {
        let (branch, kind) = item?;
        let name    = branch.name()?.unwrap_or("").to_owned();
        let ref_name = branch.get().name().unwrap_or("").to_owned();
        let tip_sha = branch.get().peel_to_commit()
            .map(|c| c.id().to_string())
            .unwrap_or_default();
        let upstream = branch.upstream().ok()
            .and_then(|u| u.name().ok().flatten().map(String::from));
        let btype = match kind {
            G2BranchType::Local  => BranchType::Local,
            G2BranchType::Remote => BranchType::Remote,
        };
        out.push(Branch { name, ref_name, tip_sha, upstream, kind: btype });
    }

    out.sort_by(|a, b| a.kind.cmp(&b.kind).then(a.name.cmp(&b.name)));
    Ok(out)
}

pub fn current_branch(path: &str) -> Result<Option<Branch>> {
    let repo = Repository::open(path)?;
    if repo.head_is_detached() {
        return Ok(None);
    }
    let head = repo.head()?;
    let name = head.shorthand().unwrap_or("").to_owned();
    let ref_name = head.name().unwrap_or("").to_owned();
    let tip_sha = head.peel_to_commit()?.id().to_string();
    Ok(Some(Branch { name, ref_name, tip_sha, upstream: None, kind: BranchType::Local }))
}

pub fn create_branch(path: &str, name: &str, from_sha: Option<&str>) -> Result<Branch> {
    let repo   = Repository::open(path)?;
    let target = match from_sha {
        Some(sha) => repo.find_commit(repo.revparse_single(sha)?.id())?,
        None      => repo.head()?.peel_to_commit()?,
    };
    let branch = repo.branch(name, &target, false)?;
    let ref_name = branch.get().name().unwrap_or("").to_owned();
    Ok(Branch {
        name:     name.to_owned(),
        ref_name,
        tip_sha:  target.id().to_string(),
        upstream: None,
        kind:     BranchType::Local,
    })
}

pub fn delete_branch(path: &str, name: &str) -> Result<()> {
    let repo = Repository::open(path)?;
    let mut branch = repo.find_branch(name, G2BranchType::Local)?;
    branch.delete()?;
    Ok(())
}

pub fn rename_branch(path: &str, old_name: &str, new_name: &str) -> Result<Branch> {
    let repo = Repository::open(path)?;
    let mut branch = repo.find_branch(old_name, G2BranchType::Local)?;
    let renamed = branch.rename(new_name, false)?;
    let ref_name = renamed.get().name().unwrap_or("").to_owned();
    let tip_sha  = renamed.get().peel_to_commit()?.id().to_string();
    Ok(Branch { name: new_name.to_owned(), ref_name, tip_sha, upstream: None, kind: BranchType::Local })
}

pub fn checkout_branch(path: &str, name: &str) -> Result<()> {
    let repo   = Repository::open(path)?;
    let branch = repo.find_branch(name, G2BranchType::Local)?;
    let obj    = branch.get().peel_to_commit()?.into_object();
    repo.checkout_tree(&obj, None)?;
    repo.set_head(branch.get().name().unwrap_or(""))?;
    Ok(())
}
