# Reconciliation 002

Stage 0, second pass. Reads the owner's round-002 answers
(`ARCHITECTURE_INPUTS_002.md`, transcribed from `interview/answers-002.json`,
last updated 2026-10-03T10:54:49Z) against `RECONCILIATION_001.md`. 75
questions; 72 answered; 3 unanswered (G002, G003, V012); 6 notes.

Every contradiction R1–R15 of the first reconciliation is now settled, except
where a line below says OPEN. Nothing here is a decision record; these are
GIVENs from which the records of `RECONCILIATION_001.md` §6 can be drafted in
their stages.

## 1. Settled

| Was | Now GIVEN | Answers |
| --- | --- | --- |
| R1 Hart | Our text is translated from the Greek with stored lexica as evidence. Hart and the TLV are consulted on the owner's machine for which reading to prefer; a local check measures shared word sequences and flags a verse that tracks too closely. Restricted texts live outside the repository entirely. | E002 A, E007 A, V003 A |
| R2 Syriac | Build-time only. The book carries our English of the Peshitta; no Syriac text, face or column ships. Psalms first, then other books by how much their differences matter. | Y001 A, Y002 A |
| R3 Order | Hebrew order for the Hebrew Bible; the deuterocanon as its own section between the Writings and the Gospels, with 1 Enoch and Jubilees after it, marked as outside every Western canon; the Apostolic Fathers after Revelation. | O001 A, O002 A, O003 A |
| R4 Voice | A key-term entry has two parts: the evidence, neutrally; then a signed Editor's definition. The models draft those in the owner's voice from the owner's notes; the owner edits. “my feedback helps direct exploratory rendition, but i do not directly offer most definitions.” | G001 A, G004 B |
| R5 Tokens | Two layers: words are the logical unit for identity, search, anchors and shaping; subword pieces code the vocabulary and stream beneath. | J001 A |
| R6 Name rule, internal | The Name stands only in the naming passages (Exodus 3:14–15; 6:2–3; 34:5–7; Isaiah 42:8), written YHWH. Elsewhere in God's speech it follows the ordinary rule — with the new formula of §2 below. | D005 B, D006 B, D007 |
| R7 Name rule and fidelity | The rule applies to every English text shown, the BSB included. The title page says “BSB, with the divine names rendered by rule”; the slip shows the BSB's own word. | E005 A |
| R8 Faces | Latin: EB Garamond and Portsmouth. Portsmouth is the owner's to license and ships under the OFL with the source-font notices Plan Paint carries. | J005 A, J006 A |
| R9 Which side | Original on the left page, English on the right, for every language. | J004 A |
| R10 Alignment | STEPBible's Greek and Hebrew are displayed; the Berean alignment is carried across word by word, and the places where the two Greek texts differ are marked. | J007 A |
| R11 Sound | Silent. Hymn melodies are printed in a simple notation. | U009 B |
| R12 Licences | Uncertain sources may be read for research on this machine, outside the tracked tree; nothing ships or is cited from them until cleared. Models may read outside sources while arguing; what they write cites a stored, cleared source or is labelled as going beyond them. | V002 A, V004 A |
| R13 Review | Review state lives in the source files only; the slip carries no mark. Tie-breaker interviews are batched, one per book. | V006 B, V005 A |
| R15 Scope | Rabbinic and patristic commentary, and Worship and Practice, arrive after the first public version, section by section. | O008 B, U011 B |

Also GIVEN
- **The rendition** begins as a revision of the BSB — its wording stands where
  adequate and changes where it conceals, each change recorded with evidence —
  and becomes a fresh translation later if that proves too timid (E001 C). Its
  name is **A Subjective Interpretation** (E004). The BSB is the page; the
  rendition is a second text, turned to book by book as each is reviewed (E003
  A). The Old Testament follows later, Psalms and Prophets first (E006 B). The
  deuterocanon shows the WEB until our rendition of a book is reviewed (E009 C).
- **Names.** “the Blessed One” in small capitals (D001 B). Adonai with the Name:
  “Lord God” (D002 C). “of Hosts”: **“of Endless Might”** (D003 note). The
  “… is His name” formula only in its literal form (D004 A). New Testament
  quotations of a verse that has the Name: “the Blessed One”; otherwise “Lord”
  (D008 A). “the Anointed One” always; “Jesus” with a switch to **Yehoshua**
  (D012 A, D013 B). Gathered in `NAME_RULE.md`.
- **Gold** is a darker antique gold whose contrast passes, with a restrained
  specular (D010 A), for God's direct speech in the Old Testament, the voice
  from heaven in the New, and the angel of the LORD speaking as God (D011 A–C).
  Prophetic oracles in the first person are not gold.
- **Apostolic Fathers kept:** 1 Clement, the Didache, Ignatius's seven letters,
  Polycarp to the Philippians. Excluded: 2 Clement, Barnabas, Hermas,
  Diognetus, Papias, the Martyrdom of Polycarp (O004).
- **Thumb-index sections** as proposed (O005 A): Torah · Former Prophets ·
  Latter Prophets · The Twelve · Psalms and Wisdom · Scrolls and Histories ·
  Deuterocanon and Enoch · Gospels and Acts · Paul · Letters and Revelation ·
  Apostolic Fathers · Worship and Practice · Maps and Indexes.
- **Rabbinic material:** a curated set of commentary on the Gemara, as with the
  Vilna Gaon (O006 A); Mishnah where it explains practice; Pirkei Avot whole
  (O007 A, B).
- **Worship and Practice:** a division at the back (U001 A). Traditions:
  rabbinic Judaism, Anglican, Roman Catholic in the older rite, Messianic
  Jewish (U002). Rites: Sabbath, Passover, the other appointed times with
  Hanukkah and Purim, daily Jewish prayer, the Supper, baptism, morning and
  evening prayer, marriage, burial and mourning, the Christian year, anointing
  of the sick — “not confession or absolution. but the anointing is attested”
  (U003 note). Each entry gives the liturgy's own words and then how and why
  (U004 C). About fifty hymns (U006 A); creeds, classic prayers, a handful of
  homilies, pastoral letters and rules (U007 A–D); Stage 1 proposes the lists
  and the owner strikes and adds (U008 A). Shipped subjects: the weekly Torah
  portions with their haftarot, the Prayer Book lectionary, authored story
  threads (U010 A–C).
- **Pages:** three type-size steps, each with its own page breaks (J003 B);
  build-time against first-open pagination is measured in Stage 4 (J002 C).
- **Witnesses:** where Hebrew, Greek, Latin or Syriac differ in substance a
  verse is marked and the slip sets the renderings side by side in English;
  the Peshitta Psalm can also be the facing text (Y003 C). Both Latin Psalters
  are Psalms witnesses, shown in English (Y004 B). The proving psalm is Psalm
  87 (Y005 A, Y006 A) — the Syriac is still to be read when stored.
- **Material:** the fibre renderer is copied into this repository as a
  build-time tool and extended for paper and wood grain; baked tiles ship (J009
  A). The alternative-visibility pairs are Phosphor and Bordeaux (J010 A).
- **Ceilings** are ceilings; Stage 2 measures the floors (J008 A).
- **Repository:** create `falseywinchnet/pocket-bible`, private (V001 A). The
  public repository will start with a fresh history from a reviewed tree; the
  working history stays private (V013 A).
- **Toolkit requests** are kept in `docs/TOOLKIT_REQUESTS.md`; the owner or the
  Codex threads pick them up (V009 A).
- **Dedication and credits** (V007, V008): the dedication opens the About
  section — “a dedication to god that is sincere does not come after or in the
  middle of commentary or about sections. it is at the beginning … because it
  establishes foundation.” Text in the root `README.md`. Credits: “Authors:
  Opus, Fable, Astra, Rainstar”. No sponsor line.
- **Conduct of Stages 1 and 2:** two visible sessions, one on sources, one on
  the skeleton and build (V011 C). Prompts in `planning/handoff/`.
- **Golden queries so far** (S015, V010), verbatim: “devoted” (must find chesed
  and the other kinds); “hardened; fear of the lord; overzealous, samuel's
  grandson, the hundred,”.

## 2. New from the notes

- **N16 “I am the LORD” as “I am the Lord your God”.** D009 note, on Leviticus
  19:18: “this type of place is awkward. the invocation repeatedly makes it
  clear he is reminding them who he is. who is he (in relation to them? he is
  the Lord their God. so ‘the Lord Your God’ is another form that must be
  considered — and works best here.” A fifth output of the name rule, for the
  self-identification that closes commandments. OPEN-1 below.
- **N17 Later Christian philosophical writings** (O004 note): “just as we do
  later rabbis … especially for those which delve into important or difficult
  spiritual subjects. but not luther!” A new body of text. Stage 1 proposes a
  list for the owner to strike; it arrives after the first public version with
  the other commentary. OPEN-4.
- **N18 A notation for melodies** (U009 B) is something the page must set. A
  toolkit-independent, text-based notation rendered by the page compositor;
  designed in the Worship and Practice stage.

## 3. Still open — small, none blocks Stages 1 or 2

Each has a working reading that stands unless the owner says otherwise at the
next gate. They are collected for a short round 003 when the first gate (G1)
comes.

- **OPEN-1** The self-identification formula. Working reading: where God closes
  a command with “I am the LORD” (אֲנִי יְהוָה), print “I am the Lord your God”.
  Where the Hebrew itself says “I am the LORD your God” (אֲנִי יְהוָה
  אֱלֹהֵיכֶם) the same English results, so two different Hebrew formulas become
  one; the slip shows which stood. Confirm that the merging is acceptable, and
  whether it reaches “I am the LORD” outside the law codes (Exodus 6, Isaiah
  40–55, Ezekiel's “then they will know that I am the LORD”).
- **OPEN-2** D009 ticked both “as the BSB” and “El Shaddai, El Elyon
  transliterated”. Working reading: those two are transliterated; every other
  title is as the BSB prints it. “Father” only where the text has it (unticked,
  so not confirmed). Our own notes say “God”, not “HaShem” (unticked).
- **OPEN-3** G002 and G003 unanswered: the signature under an Editor's
  definition (working reading: “— the editor”), and the list of key terms
  (working reading: the seed list in G003 stands as candidates).
- **OPEN-4** The later Christian writers (N17): which, and how much of each.
- **OPEN-5** U002 left Eastern Orthodox unticked, while U005 E admits the
  Liturgy of St John Chrysostom and U007 C's example is his Paschal homily.
  Working reading: the liturgy and homily are included as texts; an Orthodox
  set of rites is not written.
- **OPEN-6** E008: “it would seem sacred name bibles are mainly.. kjv?” Working
  reading: the wish for a sacred-name text is met by the name rule itself; no
  separate sacred-name edition is sought. The King James (public domain) stays
  available as a comparison input for the prophets.
- **OPEN-7** Golden queries need, for each, what a right answer must contain.
  Stage 8 brings the owner candidate judgments built from the stored texts
  rather than guessing now. Two of the six (“samuel's grandson”, “the hundred”)
  ask for a person and a thing rather than a word, which is a kind of search
  round 001 did not describe.
- **OPEN-8** The rendition is named A Subjective Interpretation; its short
  citation label for Copy is not set (working reading: “ASI”).
- **OPEN-9** The vocative. “O LORD” under the rule: working reading “O Blessed
  One”, without the article.

## 4. Consequences (HYPOTHESIS)

- **C8** No Syriac shaping and one Latin face family of two: the compositor
  handles Latin, polytonic Greek and pointed Hebrew only. Portsmouth is
  Latin-only, so Greek and Hebrew beside it need their own faces in every
  setting; with EB Garamond, Greek may come from Gentium (F002 A).
- **C9** Page-break tables: two Latin faces × three size steps = six sets per
  layout (reading and study), if build-time pagination wins (J002).
- **C10** The name rule needs, per occurrence of each divine name in the
  Hebrew, a class (ordinary; with Adonai; with Sabaoth; name formula; naming
  passage; self-identification; quoted in the New Testament). That is a table
  built from the tagged Hebrew plus a small reviewed exception list — and the
  BSB's English alignment tells where each lands on the page.
- **C11** “the Blessed One of Endless Might” is longer than “the LORD of
  Hosts” by about fifteen characters and occurs several hundred times; page
  breaks are computed after the rule is applied, never before.
- **C12** The speaker table (gold) and the quotation table (D008) are authored
  data under the same two-model protocol as the expansions.

## 5. Acted on in this pass

- Created the private repository and pushed (V001 A) — see the Stage 0 report
  addendum for the exact commands and result.
- Wrote `planning/NAME_RULE.md`, `planning/handoff/S1-sources.md`,
  `planning/handoff/S2-skeleton.md`; amended `INVARIANTS.md`, `STAGE_PLAN.md`,
  `AGENTS.md`, `SOURCE_REGISTER.md`, `README.md`.
- Stages 1 and 2 are open.
