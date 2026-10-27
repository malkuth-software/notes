# Product negative

> **Amended 2026-10-03 after round 001 (A007, A008 and notes).** Confirmed
> absent by the owner: reading plans and streaks; verse of the day and
> notifications; accounts, cloud sync and social sharing; a chat assistant in
> the client; links that open a browser; telemetry; donation and ratings
> prompts; a translation library; automatic update checks; any network code in
> the client (A008 A); an amplified reading mode (X019 B); a gematria display
> (L017 A); saved searches (S014 A); printing (I013 A); a handwriting face
> (F015 A). **Not** confirmed absent, and now wanted in bounded form: homilies,
> pastorals, prayers, hymns and rite explanations (A007 note; round 002 section
> U). Audio is undecided (U009). Lines marked PROPOSED below that the list
> above covers are now GIVEN; the rest stand as written.

What the product is not. Each line is an exclusion with its reason. Lines marked
GIVEN come from the brief; lines marked PROPOSED await interview A007 and become
GIVEN or are struck in Stage 0.

## Interface trajectories

- GIVEN No translation library. Mainly the BSB; a short, fixed list of other
  witnesses. Reason: “it is what we leave out”.
- GIVEN No decorative bookmarks. Coloured tabs exist only as part of subject
  work.
- OBSERVED (toolkit law) No persistent side panels. Secondary tools are owned
  popups or bounded overlays. `file_manager/AGENTS.md`, “Persistent left/right
  panels are reserved to File Manager”.
- OBSERVED (toolkit law) No idle animation, no perpetual redraw.
- PROPOSED No chat assistant in the client. Expansions are authored ahead of
  time and shipped as text; no model runs on the reader's machine.
- PROPOSED No reading plans, streaks, verse of the day, notifications.
- PROPOSED No devotionals or sermon material.

## Runtime exclusions

- GIVEN No network dependency. Fully offline.
- PROPOSED No network code linked at all (A008).
- PROPOSED No telemetry, crash upload, analytics, accounts, cloud sync.
- PROPOSED No automatic update check.
- OBSERVED (toolkit law) No GPU requirement; CPU rendering only.
- OBSERVED (toolkit law) No web engine, no .NET, no Java.
- PROPOSED No audio subsystem (P015).
- PROPOSED No silently used host fonts (toolkit direction; F012).

## Content exclusions

- GIVEN No text that is not captured completely and correctly from a source.
- Absolute No scripture, lexicon or commentary text supplied from a model's
  recall. Models read stored sources and write labelled authored text.
- Absolute No copyrighted text in the repository, CI or installer. BDAG, HALOT,
  TDNT, NA28/UBS5 apparatus, modern copyrighted translations are out.
- PROPOSED No doctrinal adjudication in authored text (A006, X009).
- PROPOSED No numeric “semantic distance” presented as a measurement (L012).
- PROPOSED No opaque embedding vectors unless they win a judged comparison
  (S016).

## Anti-model work still required

For each excluded thing a reader will look for, the product needs an honest
answer that is not the excluded thing. Stage 0 fills this table from the
interview.

| A reader wants | The product offers instead |
| --- | --- |
| Another translation to compare | The original word, its lexicon entry and its expansion |
| To ask a question | The expansion, the context note, the verse-anchored commentary |
| A reading plan | Ribbons |
| Sync between machines | Human-readable mark files to copy |
