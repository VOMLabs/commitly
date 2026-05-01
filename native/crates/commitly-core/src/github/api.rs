use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GhRepo {
    pub id:          u64,
    pub full_name:   String,
    pub clone_url:   String,
    pub html_url:    String,
    pub description: Option<String>,
    pub private:     bool,
    pub default_branch: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GhPullRequest {
    pub number:    u64,
    pub title:     String,
    pub body:      Option<String>,
    pub state:     String,
    pub html_url:  String,
    pub head_ref:  String,
    pub base_ref:  String,
    pub draft:     bool,
}

const GITHUB_API: &str = "https://api.github.com";

fn client(token: &str) -> reqwest::blocking::Client {
    reqwest::blocking::Client::builder()
        .user_agent("commitly/0.1.0")
        .default_headers({
            use reqwest::header::{AUTHORIZATION, HeaderMap, HeaderValue};
            let mut map = HeaderMap::new();
            map.insert(
                AUTHORIZATION,
                HeaderValue::from_str(&format!("Bearer {token}")).unwrap(),
            );
            map
        })
        .build()
        .expect("failed to create HTTP client")
}

pub fn list_repos(token: &str) -> crate::error::Result<Vec<GhRepo>> {
    let url  = format!("{GITHUB_API}/user/repos?per_page=100&affiliation=owner,collaborator");
    let resp = client(token).get(&url).send()?.error_for_status()?;
    let repos: Vec<GhRepo> = resp.json()?;
    Ok(repos)
}

pub fn list_pull_requests(token: &str, owner: &str, repo: &str) -> crate::error::Result<Vec<GhPullRequest>> {
    let url  = format!("{GITHUB_API}/repos/{owner}/{repo}/pulls?state=open&per_page=100");
    let resp = client(token).get(&url).send()?.error_for_status()?;
    let prs: Vec<GhPullRequest> = resp.json()?;
    Ok(prs)
}

pub fn verify_token(token: &str) -> crate::error::Result<String> {
    #[derive(Deserialize)]
    struct User { login: String }
    let resp = client(token)
        .get(format!("{GITHUB_API}/user"))
        .send()?
        .error_for_status()?;
    let user: User = resp.json()?;
    Ok(user.login)
}
