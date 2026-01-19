# Local Git workflow

Goal

1. Keep claiming fast and local.
2. Keep agents isolated.

Concepts

1. A local bare repo acts as the shared origin for all agents.
2. A master clone is used to sync with public GitHub and to create worktrees.
3. Each agent works in its own worktree and branch.
4. Agents push to the local origin main to publish claims and completed work locally.
5. A separate orchestrator step pushes local main to public GitHub.

One time setup

1. Create a local bare origin

```bash
git clone --bare https://github.com/haeminjung12/Droplet-Microfluidics-Analysis-Suite.git C:\repos\droplet-suite.origin
```

2. Create a master clone that uses the local origin

```bash
git clone C:\repos\droplet-suite.origin C:\repos\droplet-suite.master
```

3. Add the public remote in the master clone

```bash
cd C:\repos\droplet-suite.master
git remote add public https://github.com/haeminjung12/Droplet-Microfluidics-Analysis-Suite.git
```

Agent worktree creation

Run in the master clone

1. Sync master from public and publish to local origin

```bash
cd C:\repos\droplet-suite.master
git fetch public
git checkout main
git pull --rebase public main
git push origin main
```

2. Create a new worktree and branch

```bash
git worktree add C:\work\agent01 -b agent01_taskNN origin/main
```

Agent inner loop

All commands run inside the agent worktree.

1. Sync your branch to local main

```bash
git fetch origin
git rebase origin/main
```

2. Claim a task

1. Read Docs/TODO.md top to bottom.
2. Pick the first unchecked task with no ACTIVE marker.
3. Add a marker line under that task

SIGN <your branch name> ACTIVE task NN

4. Commit and push the claim to local origin main

```bash
git add Docs/TODO.md
git commit -m "Docs claim task NN"
git push origin HEAD:main
```

If push fails

1. Another agent pushed first.
2. Re sync and try again.

```bash
git fetch origin
git rebase origin/main
```

If the rebase conflicts in Docs/TODO.md

1. Abort.
2. Re read Docs/TODO.md and pick a different task.

```bash
git rebase --abort
```

Task completion

1. Commit your code changes on your branch.
2. Update Docs/TODO.md with checkbox and DONE marker.
3. Re sync then push the completed work to local origin main.

```bash
git fetch origin
git rebase origin/main

git add -A
git commit -m "Task NN done"

git push origin HEAD:main
```

Push to public GitHub

Run in the master clone.

```bash
cd C:\repos\droplet-suite.master
git checkout main
git pull origin main
git push public main
```

Cleanup

Run in the master clone.

```bash
git worktree remove C:\work\agent01
```
