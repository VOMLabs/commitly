use crate::{error::Result, models::Commit};
use git2::{Repository, Sort};

/// Return up to `max` commits starting from HEAD.
pub fn repo_log(path: &str, max: usize) -> Result<Vec<Commit>> {
    let repo = Repository::open(path)?;
    let mut revwalk = repo.revwalk()?;
    revwalk.set_sorting(Sort::TIME | Sort::TOPOLOGICAL)?;
    revwalk.push_head()?;

    // Collect tag OIDs for annotation
    let mut tag_map: std::collections::HashMap<git2::Oid, Vec<String>> =
        std::collections::HashMap::new();
    repo.tag_foreach(|oid, name| {
        let n = String::from_utf8_lossy(name)
            .trim_start_matches("refs/tags/")
            .to_owned();
        // Resolve annotated tags to their target commit
        let target_oid = repo
            .find_tag(oid)
            .map(|t| t.target_id())
            .unwrap_or(oid);
        tag_map.entry(target_oid).or_default().push(n);
        true
    })?;

    let commits = revwalk
        .take(max)
        .filter_map(|r| r.ok())
        .filter_map(|oid| repo.find_commit(oid).ok())
        .map(|c| {
            let sha       = c.id().to_string();
            let short_sha = sha[..7].to_owned();
            let summary   = c.summary().unwrap_or("").to_owned();
            let body      = c.body().unwrap_or("").to_owned();
            let author    = crate::models::CommitIdentity::from_git2(&c.author());
            let committer = crate::models::CommitIdentity::from_git2(&c.committer());
            let parent_shas = c.parent_ids().map(|id| id.to_string()).collect();
            let tags = tag_map.get(&c.id()).cloned().unwrap_or_default();
            Commit { sha, short_sha, summary, body, author, committer, parent_shas, tags }
        })
        .collect();

    Ok(commits)
}
