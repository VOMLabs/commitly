use crate::error::Result;
use git2::{build::RepoBuilder, FetchOptions, RemoteCallbacks};

/// Clone a repository from `url` into `dest_path`.
/// Returns the absolute path of the newly cloned repo.
pub fn clone_repo(url: &str, dest_path: &str) -> Result<String> {
    let mut callbacks = RemoteCallbacks::new();
    callbacks.credentials(|_url, username, _allowed| {
        git2::Cred::ssh_key_from_agent(username.unwrap_or("git"))
    });

    let mut fetch_opts = FetchOptions::new();
    fetch_opts.remote_callbacks(callbacks);

    let mut builder = RepoBuilder::new();
    builder.fetch_options(fetch_opts);

    let repo = builder.clone(url, std::path::Path::new(dest_path))?;
    Ok(repo.path()
        .parent()
        .unwrap_or(repo.path())
        .to_string_lossy()
        .into_owned())
}
