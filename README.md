# GitIdentityManagerCpp

Are you accidentally comitting with the wrong git identity? If so, this tool is for you! Always make sure your git identity is set in the local repository config. Use a global git hook to make sure you never mess up with a new repository again!

Cross platform command line port of [GitIdentityManager](https://github.com/mrexodia/GitIdentityManager) (probably should have been written in Go).

```bash
$ git-identity
(1) John Doe <john.doe@personal.space>
    commit.gpgsign      true
    tag.gpgsign         true
    user.signingkey     12345566
(2) John Doe <jdoe@office.corp>
    commit.gpgsign      false
    tag.gpgsign         false
    user.signingkey     <empty>

Your choice:
```

## Installation

Compile the project with CMake and copy/symlink the `git-identity` executable in your path.

### git-identity.json

You should create a file name `~/git-identity.json` that contains your identities (or any other git config). The only required fields are `user.name` and `user.email`.

```
[
    {
        "user.name": "John Doe",
        "user.email": "john.doe@personal.space",
        "user.signingkey": "12345566",
        "credential.username": "johnpersonal",
        "commit.gpgsign": "true",
        "tag.gpgsign": "true"
    },
    {
        "user.name": "John Doe",
        "user.email": "jdoe@office.corp",
        "credential.username": "johnoffice",
        "user.signingkey": "",
        "commit.gpgsign": "false",
        "tag.gpgsign": "false"
    }
]
```

You can now run `git identity` to change your identity in a local repository. You can use `git identity check` in scripts to see if a local identity is set.

### Global configuration

The following configuration is recommended to prevent accidents:

```bash
git config --global user.useConfigOnly true
git config --global --unset user.name
git config --global --unset user.email
```

An alternative is to set up a global `pre-commit` hook template. A sample hook is provided (`.git-templates` directory in this repository) that checks your identity before committing. See [this post](https://santexgroup.com/blog/create-a-global-git-hook-to-check-flake8-before-each-commit/) for more details.