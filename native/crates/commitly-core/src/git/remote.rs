use crate::{error::Result, models::Remote};
use git2::{FetchOptions, PushOptions, RemoteCallbacks, Repository};

pub fn list_remotes(path: &str) -> Result<Vec<Remote>> {
    let repo = Repository::open(path)?;
    let names = repo.remotes()?;
    let mut out = Vec::new();
    for name in names.iter().flatten() {
        let remote = repo.find_remote(name)?;
        out.push(Remote {
            name: name.to_owned(),
            url:  remote.url().unwrap_or("").to_owned(),
        });
    }
    Ok(out)
}

pub fn add_remote(path: &str, name: &str, url: &str) -> Result<Remote> {
    let repo = Repository::open(path)?;
    repo.remote(name, url)?;
    Ok(Remote { name: name.to_owned(), url: url.to_owned() })
}

pub fn remove_remote(path: &str, name: &str) -> Result<()> {
    let repo = Repository::open(path)?;
    repo.remote_delete(name)?;
    Ok(())
}

pub fn fetch(path: &str, remote_name: &str) -> Result<()> {
    let repo   = Repository::open(path)?;
    let mut remote = repo.find_remote(remote_name)?;
    let mut opts = FetchOptions::new();
    opts.download_tags(git2::AutotagOption::Unspecified);
    let refspecs: &[&str] = &[][..];
    remote.fetch(refspecs, Some(&mut opts), None)?;
    Ok(())
}

pub fn pull(path: &str, remote_name: &str, branch: &str) -> Result<()> {
    fetch(path, remote_name)?;
    let repo     = Repository::open(path)?;
    let fetch_head = repo.find_reference("FETCH_HEAD")?;
    let fetch_commit = repo.reference_to_annotated_commit(&fetch_head)?;
    let (analysis, _) = repo.merge_analysis(&[&fetch_commit])?;

    if analysis.is_fast_forward() {
        let refname = format!("refs/heads/{branch}");
        let mut reference = repo.find_reference(&refname)?;
        reference.set_target(fetch_commit.id(), "pull: fast-forward")?;
        repo.set_head(&refname)?;
        repo.checkout_head(Some(git2::build::CheckoutBuilder::default().force()))?;
    } else if analysis.is_normal() {
        let head_commit = repo.reference_to_annotated_commit(&repo.head()?)?;
        repo.merge(&[&fetch_commit], None, None)?;
        // Auto-commit merge if no conflicts
        let mut index = repo.index()?;
        if !index.has_conflicts() {
            let cfg   = repo.config()?;
            let name  = cfg.get_string("user.name").unwrap_or_else(|_| "Unknown".into());
            let email = cfg.get_string("user.email").unwrap_or_else(|_| "unknown@example.com".into());
            let sig   = git2::Signature::now(&name, &email)?;
            let tree_oid = index.write_tree()?;
            let tree     = repo.find_tree(tree_oid)?;
            let local    = repo.find_commit(head_commit.id())?;
            let remote_c = repo.find_commit(fetch_commit.id())?;
            repo.commit(Some("HEAD"), &sig, &sig, "Merge", &tree, &[&local, &remote_c])?;
            repo.cleanup_state()?;
        }
    }
    Ok(())
}

pub fn push(path: &str, remote_name: &str, branch: &str) -> Result<()> {
    let repo = Repository::open(path)?;
    let mut remote = repo.find_remote(remote_name)?;
    let refspec = format!("refs/heads/{branch}:refs/heads/{branch}");
    let mut callbacks = RemoteCallbacks::new();
    callbacks.credentials(|_url, username, _allowed| {
        git2::Cred::ssh_key_from_agent(username.unwrap_or("git"))
    });
    let mut opts = PushOptions::new();
    opts.remote_callbacks(callbacks);
    remote.push(&[refspec.as_str()][..], Some(&mut opts))?;
    Ok(())
}
