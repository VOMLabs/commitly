use commitly_core::git::{commit as git_commit, log, status};
use std::path::Path;
use tempfile::TempDir;

fn init_repo(dir: &Path) {
    let repo = git2::Repository::init(dir).unwrap();
    let mut config = repo.config().unwrap();
    config.set_str("user.name", "Test User").unwrap();
    config.set_str("user.email", "test@example.com").unwrap();
}

fn make_file_and_stage(repo_path: &Path, filename: &str, content: &str) {
    std::fs::write(repo_path.join(filename), content).unwrap();
    git_commit::stage_paths(repo_path.to_str().unwrap(), &[filename][..]).unwrap();
}

#[test]
fn status_untracked() {
    let dir  = TempDir::new().unwrap();
    init_repo(dir.path());
    std::fs::write(dir.path().join("hello.txt"), "hi").unwrap();

    let entries = status::repo_status(dir.path().to_str().unwrap()).unwrap();
    assert!(!entries.is_empty(), "expected at least one status entry");
    let found = entries.iter().any(|e| e.path == "hello.txt");
    assert!(found, "hello.txt not in status");
}

#[test]
fn status_staged() {
    let dir = TempDir::new().unwrap();
    init_repo(dir.path());
    make_file_and_stage(dir.path(), "readme.md", "# Hello");

    let entries = status::repo_status(dir.path().to_str().unwrap()).unwrap();
    let entry = entries.iter().find(|e| e.path == "readme.md").expect("readme.md missing from status");
    assert_eq!(entry.index_status, commitly_core::models::FileStatus::New);
}

#[test]
fn log_after_commit() {
    let dir = TempDir::new().unwrap();
    init_repo(dir.path());
    make_file_and_stage(dir.path(), "a.txt", "content");
    git_commit::create_commit(dir.path().to_str().unwrap(), "initial commit", None).unwrap();

    let commits = log::repo_log(dir.path().to_str().unwrap(), 10).unwrap();
    assert_eq!(commits.len(), 1);
    assert_eq!(commits[0].summary, "initial commit");
}

#[test]
fn log_multiple_commits() {
    let dir = TempDir::new().unwrap();
    init_repo(dir.path());

    for i in 0..3u8 {
        make_file_and_stage(dir.path(), &format!("f{i}.txt"), &format!("content {i}"));
        git_commit::create_commit(
            dir.path().to_str().unwrap(),
            &format!("commit {i}"),
            None,
        ).unwrap();
    }

    let commits = log::repo_log(dir.path().to_str().unwrap(), 10).unwrap();
    assert_eq!(commits.len(), 3);
    // Most recent first
    assert_eq!(commits[0].summary, "commit 2");
}
