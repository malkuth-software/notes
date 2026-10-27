# Malkuth Software notes

Organisation-wide guidance lives on **main**. Each project's own material lives on a branch named after it, and is never shipped with the product.

| Branch | What |
|---|---|
| `main` | guidance for everyone: the house style, agent directions, and the contributing and new-app guides |
| `gui_forms` | GUI.Forms: planning and stage briefs, decisions, interviews, studies, experiments and results |

To read everything side by side, check each branch out as a worktree:

```bash
git clone https://github.com/malkuth-software/notes.git notes
cd notes
git worktree add ../notes-gui_forms gui_forms
```

## Status

- `HOUSE_STYLE.md` on this branch is the canonical programming house style (owner, 2026-10-27). Its sections are ordered by importance, most important first. Change it here; repositories that summarise it follow this text.
- The agent directions, and the contributing and new-app guides, arrive with `template_harness`.
