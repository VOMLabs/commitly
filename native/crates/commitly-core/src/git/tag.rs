use crate::error::Result;
use git2::Repository;

#[derive(Debug, Clone)]
pub struct Tag {
    pub name:    String,
    pub sha:     String,
    pub message: Option<String>,
}

pub fn list_tags(path: &str) -> Result<Vec<Tag>> {
    let repo = Repository::open(path)?;
    let tag_names = repo.tag_names(None)?;
    let mut tags = Vec::new();

    for name in tag_names.iter().flatten() {
        let obj = repo.revparse_single(&format!("refs/tags/{name}"))?;
        let (sha, message) = if let Ok(tag) = obj.into_tag() {
            (tag.target_id().to_string(), tag.message().map(String::from))
        } else {
            (repo.revparse_single(&format!("refs/tags/{name}"))?.id().to_string(), None)
        };
        tags.push(Tag { name: name.to_owned(), sha, message });
    }

    Ok(tags)
}

pub fn create_tag(path: &str, name: &str, message: Option<&str>) -> Result<Tag> {
    let repo   = Repository::open(path)?;
    let head   = repo.head()?.peel_to_commit()?;
    let cfg    = repo.config()?;
    let uname  = cfg.get_string("user.name").unwrap_or_else(|_| "Unknown".into());
    let email  = cfg.get_string("user.email").unwrap_or_else(|_| "unknown@example.com".into());
    let sig    = git2::Signature::now(&uname, &email)?;

    let oid = if let Some(msg) = message {
        repo.tag(name, head.as_object(), &sig, msg, false)?
    } else {
        repo.tag_lightweight(name, head.as_object(), false)?
    };

    Ok(Tag { name: name.to_owned(), sha: oid.to_string(), message: message.map(String::from) })
}

pub fn delete_tag(path: &str, name: &str) -> Result<()> {
    let repo = Repository::open(path)?;
    repo.tag_delete(name)?;
    Ok(())
}
