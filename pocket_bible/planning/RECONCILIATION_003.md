# Reconciliation 003

Reads the owner's round-003 answers (`ARCHITECTURE_INPUTS_003.md`, from
`interview/answers-003.json`, last updated 2026-10-04T01:10:26Z; 71 questions,
62 answered) and the owner's direction of the same day
(`OWNER_DIRECTION_2026-10-04.md`). Not a decision record.

## 1. Gate G1

**Passed.** B301 A: “Signed: all seventeen rows”, with no row struck (B302
empty). Stage 1 is merged to `main`.

## 2. Settled by the answers

Sources
- The BSB is shown exactly as published. The name rule applies only in A
  Subjective Interpretation (B303 C). This reverses round 002 E005 A and
  invariant I37; Berean's request about its name no longer arises.
- STEPBible's licence statement is relied on, and the clean-provenance fallback
  (SBL's Greek or Nestle 1904, Open Scriptures morphology) is audited and
  pinned now so a switch is a rebuild (B304 C). The displayed Greek is the
  Nestle-Aland-side words (B305 A). Only the 1,553 substantive differences from
  the Berean base are marked (B306 A).
- Words of Christ follow the BSB's own markup; the WEB only lists disagreements
  (B307 A).
- A note to STEPBible is drafted when the first public version is near (B308
  A). Its brief Hebrew definitions are left out (B309 A).
- The pinned bytes are archived as a private release asset and on a disk of the
  owner's (B310 D).
- The Peshitta: the ETCBC text is the base (P301 A), read by two models, with
  the entries the Psalms need transcribed from Payne Smith (P304 B, L303 B).
  Psalm 87 is the owner's psalm: “all my springs are in you” is itself the
  declaration of refuge (P303 note). It joins the proving slice with Hebrew
  now; the English of the Syriac follows (P305 A).

Faces
- Portsmouth is for English only (F301 note) and needs nothing added: “anything
  it doesnt have should be dynamically created by the engine” (F302 note). The
  compositor supplies the en dash, the dagger and small capitals for it (F303
  A). It is the owner's own face and ships with no licence (F304).
- Hebrew: Ezra SIL, and Stage 4 re-tests Noto Serif Hebrew with the shaper
  (F305 B). Ezra's MIT layout tables are within the rule (F306 A). Greek: EB
  Garamond's own beside EB Garamond, Gentium Book beside Portsmouth (F307 A).
  The compositor draws Charles's critical signs (F308 A). The toolkit's seven
  faces ship meanwhile (F309 A).

Lexica
- The first public version waits for Trench (L301 B), which is transcribed
  first, by two models against the page images (L302 A). Payne Smith and
  Moulton–Milligan: only the entries the work needs (L303 B). The Apostolic
  Fathers are English only at first (L304 A). Semantic groupings are our own,
  built from what is cleared (L305 C).

The window
- The Windows image cost goes to the GUI.Forms threads as the first priority
  while Stage 4 measures every other route (W301 A). Requests M1, M2, M3, M5 and
  M6 go now (W302 A–D). “Instant” is 50 ms from key to page (W303 C). Pages are
  composed at device resolution; on Windows only the visible spread is kept
  until the doubling is fixed (W304 A). The launch ceiling stays 400 ms (W305
  A). Fable continues to conduct (W306 A).

Lists
- Later writers: none struck; the owner has not read the list and asks what
  each gives (H305 note). Kierkegaard is translated here from the Danish (H307
  A). Each writer: the part named, none longer than Mark (H308 A). They sit in
  one section after the Apostolic Fathers, with links from subjects and key
  terms (H309 C). Messianic practice is noted within the Jewish rites (H310 A).
  Readings from the Prophets: Ashkenazi base (H313 A).
- Hymns and liturgical texts are printed as their authors wrote them; the name
  rule does not reach them (H304 A). No hymn is struck. Whether “not Luther”
  reaches hymns is unanswered: the owner used H301 for the objection taken up
  in §4.
- The Talmud's base text (H311 note) falls under §4: an ancient text is free
  whatever its transcription's label; the convenient transcription is used.

Names (these now apply to A Subjective Interpretation only)
- “I am the Lord” for the short form, “I am the Lord your God” for the long
  (Q301 B), everywhere God so declares Himself except Ezekiel's recognition
  formula, which stays “the Blessed One” (Q302 C). The vocative is “O Lord”
  (Q303 B). El Shaddai and El Elyon are transliterated; other titles as the BSB
  (Q304 A). “Father” only where the text has it (Q305 A). The key-term seed
  list stands (Q307 A). The short label is ASI (Q308 A). No sacred-name edition
  is sought (Q309 A). The Eastern liturgy and homily are included as texts
  (Q310 A).

Conduct
- Stage 1 merged on signature (C301 A). Next: two visible sessions, importers
  with the container study, and the compositor's proving slice (C302 A). The
  Trench and Syriac transcriptions follow Stage 4's gate (C303 B) — but see
  L301 B: Trench now gates the first public version, so it cannot wait long.
- A report with its interview after every stage (C304 A), with “better
  explanation of my choices”, and with every open question of every session in
  it: the owner will not look in a second place (W307 note).

## 3. Two search engines (GIVEN, 2026-10-04)

1. **The reference field.** Inline, near the top. Finds books, chapters and
   verses only. Matches from the first letter. Tolerates common misspellings,
   transpositions and off-by-one errors.
2. **The knowledge engine.** Keyword-based; not an AI. A system of facts:
   labelled people and concepts, concepts in relation to other concepts, trees.
   The number of assertions, the vocabulary and the types of fact are all
   bounded, and are to be minimized — “battle with kolmogrov at its finest”.

This replaces the single search of round 001 section S, and answers Q312. It
is a new stage with its own interview (the fact types are the owner's to
bound). Working shape, HYPOTHESIS:

- Entities: people, peoples, places, things, events, concepts, words.
- A fact is (subject, relation, object) with the verses that establish it. The
  relation types are a closed list of a few dozen: kinship, office, belonging,
  place, event and its participants, speech, naming, kinds and contrasts of a
  concept, the original words behind a concept.
- Whatever a rule can derive is not stored: parents are stored, grandsons are
  derived. That is where the size is won.
- A query is keywords resolved to entities and relation words. Every answer
  shows its chain of facts with their verses; the chain is the explanation.
- The owner's golden queries are its first tests: “samuel's grandson” is two
  kinship facts composed; “devoted” is one concept with its kinds, positive and
  negative, each tied to its Hebrew and Greek words.

## 4. Source use: what the owner directed, and where the coordinator stands

The owner's principle: use as much as possible of what someone else *knows*
without including their work verbatim, and credit them; distribution,
commercial exploitation and re-licensing of others' works are not acceptable;
whatever is refused is retranslated by us.

Adopted in full
- **Anything may be read to inform our own text.** No licence term restricts
  learning from a source. (Already I41, I42.)
- **Ancient texts are free whatever label a digitizer attached.** The Peshitta,
  the Talmud, the Septuagint: a transcription's non-commercial or share-alike
  label does not reach our translation of the ancient text. Stage 1's caution
  here is withdrawn. Round 003 P301 A and B313 A no longer conflict.
- **Facts are free.** Morphology, alignment, versification, coordinates, who
  begat whom: used from any source, with credit.
- **Public domain in the United States is enough.** Stage 1 also required a
  work to be free everywhere; that second test is dropped. This unblocks, among
  others, *Great Is Thy Faithfulness* and the Dominican Aquinas.
- **Scanned public-domain books are usable now as research input.** “No clean
  text” stops only their verbatim display.
- **Authored entries may draw on the models' own knowledge and outside
  research**, disciplined by evidence a reader can check: the stored lexica and
  the usage of the word in the stored texts themselves. (Answers L301 note.)
- **Openly licensed components ship under their own terms**, each credited on
  one sources page. The owner is not asked licence questions again.
- **Portsmouth** ships as the owner's, unlicensed.

Declined, narrowly
- The verbatim wording of works whose expression is still someone's — Hart, the
  NKJV, the Tree of Life Version, Hine's English of *How Great Thou Art*,
  Lewis and the like — is not placed in the repository or the installer:
  whole, in standalone sections, or divided into verse-keyed excerpts. By the
  owner's own line this is distribution of their work, and dividing it does not
  change what is shipped. A translation stored as rows is still the
  translation. Quotation at the scale of a sentence, credited, where the exact
  words are the point, is used.
- No tool removes copy protection.

By the owner's rule each declined item is re-expressed by us. Case by case:

| Wanted | What is declined | What the book carries instead | What the owner would recognize |
| --- | --- | --- | --- |
| Hart's New Testament | his wording | A Subjective Interpretation, from the Greek, with his readings consulted | his choices of meaning, not his sentences |
| NKJV (B311 note) | its wording | the KJV itself, which is free; NKJV readings consulted for ASI | the KJV's cadence |
| Tree of Life Version | its wording | readings consulted for ASI | — |
| How Great Thou Art | Hine's English | our English from Boberg's Swedish original, which is free | the tune and the thought; not Hine's lines |
| Great Is Thy Faithfulness | nothing | the hymn as written | all of it |
| Lewis, Bonhoeffer, Weil, Barth, Heschel | their published English | the argument in our words, credited; Bonhoeffer and Weil translated by us where the original is free | the ideas; a sentence here and there |
| Melito, *On the Pascha*; the Peace Prayer; later English of the mystics | a modern translator's English | ours from the original | the content |
| Louw–Nida domain numbers | the classification as data | our own groupings, informed by it and credited | — |
| Online Bible's Hebrew glosses | their text | our merged entries | — |

## 5. Stage 2: closed by the coordinator

Its report exists (`reports/S2-2026-10-03.md`) and is merged. Its open
questions, which the owner asked the coordinator to settle (W307 note):

1. No tag. The workflow artifacts stand until there is something to read.
2. Linux does not idle at zero paints because of the toolkit (M6). The request
   goes now (W302 C); the miss stands in `BUDGETS.md` until a new pin.
3. The 150 MiB ceiling is read as resident memory, the stricter figure; the
   footprint is reported beside it.
4. The toolkit, its faces and the compiler runtimes get rows in the register.
5. The root licence text stands.
6. The opening size is a placeholder. An application icon is the owner's to
   choose: next round.
7. The Debian package name stands.

## 6. Still open

- H301: whether “not Luther” reaches hymns. H302, H303: hymns to strike or
  add. H312: Pirkei Avot's English. H314: how the remaining lists are brought.
  Q306: the editor's signature. Q311: what each golden query must return.
- The owner has asked what each later writer gives (H305 note): next report.
- The fact types of the knowledge engine: its own interview.
- An application icon.
