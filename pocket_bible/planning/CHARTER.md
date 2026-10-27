# Charter

> **Amended 2026-10-03 after round 001.** New GIVENs, numbered to continue the
> list below; details and open points in `RECONCILIATION_001.md` §2.
>
> - G27 The book is also a spiritual reference for when no other can be found:
>   classic prayers, hymns and homilies, and how the rites of Jewish and
>   Christian practice are performed and what they mean. It cannot be every
>   book. (A007 note.)
> - G28 An English rendition of our own, more forceful than the BSB, made by
>   arguing models with the owner breaking ties; it learns from texts that
>   cannot be licensed and copies none. (A002 note, T003 D, T022.)
> - G29 The divine names follow one deterministic rule. (T018, T019.)
> - G30 The purpose of the authored layer is to undo concealment: “meanings
>   must not be hidden, text must not be paraphrased haphazardly.” (A006 note.)
> - G31 The words of God are set in gold and the words of Christ in red.
>   (R010.)
> - G32 A dictionary of key terms that are poorly defined, beginning with
>   faith. (L005 note.)
> - G33 The wider corpus of C001–C011.
>
> G06 stands (“mainly the BSB”) with G28 beside it. G07's “Prophecy Bible” is
> void (T002); a sacred-name Bible is to be named. The risk list gains R7: the
> volume of authored and translated text now exceeds the scripture it serves,
> and its review is the owner's time.

## Objective

One pocket Bible: the text, its original languages, a dictionary, authored
expansions where English falls short, verse-anchored commentary, and maps — in
one offline book that is beautiful to read and costs almost nothing to keep
open.

## Governing question

What is the smallest set of texts, mechanisms and interface elements that puts
context within one gesture of every verse, without the product becoming a
library, a search engine or a chat window?

## GIVEN (from the brief of 2026-10-03)

Product
- G01 A highly efficient, fully offline Bible client.
- G02 Built on GUI.Forms, specifically the Plan Paint enhancements on it, under
  the laws that distinguish GUI.Forms.
- G03 Its own GitHub repository, delivering a cross-platform installer and
  client, as the other projects do.
- G04 Passive: not resource-consuming, light on memory and storage.
- G05 What is left out defines the product.

Texts
- G06 Mainly the BSB. Not all translations.
- G07 Also wanted: David Bentley Hart's latest New Testament revision; “the
  Prophecy Bible's” Isaiah and other prophets; the Syriac Peshitta, with English
  for the Psalms that may have to be translated for this project.
- G08 Added apocrypha, mainly Enoch and Judith; possibly Talmud (and “gematria”)
  where direct commentary; well-attested early Christian writings such as
  Clement's letter.
- G09 The Vilna Gaon's commentary is the model of insight that should be
  reachable.
- G10 All text is captured correctly and completely.

Page
- G11 Side-by-side two-column English, Greek, Hebrew; footnotes.
- G12 Photorealistic textured paper; cloth bookmarks; thumb-index indents that
  can be clicked to jump.
- G13 Coloured tab-like bookmarks only as part of subject-matter work.
- G14 Antialiased text in a quality font, with several options.
- G15 Saved text highlights.

Study
- G16 A dictionary.
- G17 Search that goes beyond words and fragments: bigrams, errors,
  semantic-distance candidates, borrowing from the File Manager search.
- G18 For hapax legomena in both Testaments, a set of proximal words built from
  Strong's and other dictionaries.
- G19 A sibling model reads every line of every book in multiple translations
  and writes a concise expansion and commentary, shown in a popup, wherever the
  English does not convey the full meaning. The expansions are present in the
  source material.
- G20 Accurate maps of Judah from the periods involved.

Container
- G21 The text is studied and a content-specific compression table is built, in
  the manner of tokenizer work.
- G22 Deterministic, knowing the formatting: text is uncompressed from a tiny
  database engine into a sparse container format. Text is compressed primarily
  with other text. Most formatting characters are absent from the stored form.

Process
- G23 Opus 5.5 conducts the work in many stages from stored instructions.
- G24 Demos are built for the owner to look at.
- G25 An exhaustive interview precedes architecture; the owner will answer as
  many technical questions as necessary.
- G26 The programming house style governs.

## Workstreams and their exits

0. **Negative definition.** Exit: `PRODUCT_NEGATIVE.md` confirmed by the owner.
1. **Invariants and budgets.** Exit: `INVARIANTS.md`, `BUDGETS.md` with measured
   floors, not wished numbers.
2. **Sources.** Exit: every admitted source has a licence record, a pinned hash
   and a deterministic importer; every excluded source has a reason.
3. **Platform reality.** Exit: a window, a composed page and a package on each
   target platform, with idle measured at zero.
4. **Candidate dossiers.** Exit: for the page compositor, the container, search
   and paper, the candidates are written with their failure modes.
5. **Spikes and falsification.** Exit: each dossier has measurements, including
   the losing candidates.
6. **Decision records.** Exit: numbered records with owner approval.
7. **The staged build.** `STAGE_PLAN.md`.

## What this charter refuses to decide

- Who sets the type (interview F010). It names the question; it does not answer it.
- The container's token unit, coder and context model. These are measured.
- Whether any “semantic” channel beyond the original-language link is admitted.
- The expansion voice. The owner's exemplar is the only evidence so far.
- Any size, memory or speed target. Floors are measured first.
- The name.

## Risks named now

- R1 The compositor is a typesetter. Justification, hyphenation, bidi, mark
  positioning and hit-testing are each a known source of long work. A proving
  slice (Stage 4) bounds it before anything depends on it.
- R2 Authored expansions at the scale of the whole Bible are the largest body of
  new text in the product and the place where fabrication would do the most
  harm. `EXPANSION_PROTOCOL.md` makes uncited claims a build failure.
- R3 Wanted texts under copyright cannot ship. The design must not depend on them.
- R4 Model-made translations of Syriac and rabbinic Hebrew need a review method
  the owner trusts before they are shown.
- R5 “Photorealistic” and “passive” pull against each other. Everything
  realistic is precomputed or computed once and cached; nothing animates at rest.
- R6 GUI.Forms is pre-1.0. The toolkit pin is a commit; upgrades are deliberate.
