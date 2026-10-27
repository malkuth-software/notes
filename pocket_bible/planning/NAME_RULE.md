# The name rule

Status: **GIVEN** by the owner (round 001 T018, T019; round 002 section D),
gathered here in one place. Lines marked OPEN are the working readings of
`RECONCILIATION_002.md` §3 and await confirmation. Implementation is Stage 6;
this document is its specification, not its design.

The owner's statement of principle: “The choice between Father, God, Lord, and
The Blessed One is a deterministic rule that must be applied consistently.”

## Scope

The rule applies to every English text the book shows, the BSB included (E005
A). The title page of an adapted text says so (“BSB, with the divine names
rendered by rule”), and the slip on any changed word shows the source's own
wording. The source files themselves are never altered; the rule is applied
when the container is built, from tables.

## Old Testament: by what the Hebrew has

| The Hebrew has | The page prints | Source |
| --- | --- | --- |
| The four-letter name, ordinary use | the Blessed One, set in small capitals | T018; D001 B |
| Adonai, in appeal | Lord | T018 |
| Adonai with the four-letter name (BSB “Lord GOD”) | Lord God | D002 C |
| The four-letter name with Sabaoth, ordinary use (BSB “the LORD of Hosts”) | the Blessed One of Endless Might | D003 note |
| The literal formula “… is His name” (for example Jeremiah 31:35; Amos 4:13; Exodus 15:3), with or without Sabaoth | Loving, Merciful, Faithful, and Just is His name | T018; D004 A |
| The naming passages: Exodus 3:14–15; Exodus 6:2–3; Exodus 34:5–7; Isaiah 42:8 | YHWH — “we record exactly what he says” | T018; D005 B; D007 |
| God's self-identification closing a command, “I am the LORD” | I am the Lord your God | D009 note — OPEN-1 for its reach |
| Elohim | God | T018 |
| El Shaddai, El Elyon | transliterated: El Shaddai, El Elyon | D009 B — OPEN-2 |
| Other titles (the Holy One of Israel, and the like) | as the BSB prints them | D009 A |

“the name of the LORD” in phrases such as “call on the name of the LORD” is
ordinary use: “the name of the Blessed One” (D004 A excludes it from the
formula).

In God's own speech outside the naming passages the ordinary rule applies
(D006 B), except for the self-identification row above.

## New Testament

| The Greek has | The page prints | Source |
| --- | --- | --- |
| κύριος, in a quotation of an Old Testament verse that has the four-letter name | the Blessed One | D008 A |
| κύριος elsewhere | Lord | D008 A |
| Χριστός | the Anointed One, always | T019; D012 A |
| Ἰησοῦς | Jesus; a switch prints Yehoshua | T019; D012 A; D013 B |
| πατήρ of God | Father, only where the text has it | OPEN-2 |

## Colour

- God's direct speech in the Old Testament, the voice from heaven in the New
  Testament, and the angel of the LORD speaking as God: antique gold that meets
  the body-text contrast floor, with a restrained specular (R010 note; D010 A;
  D011 A–C). First-person prophetic oracles are not gold.
- Words of Christ: red (R010 B).

## What the rule needs as data

1. Every occurrence of each divine name in the tagged Hebrew, with its class
   from the table above. Most classes follow mechanically from the tagging
   (the lemma and its neighbours); the formula, naming-passage and
   self-identification classes are short reviewed lists.
2. The English token or tokens each occurrence aligns to in each English text.
3. A table of New Testament quotations of Old Testament verses, word-aligned
   enough to say which κύριος stands for the Name.
4. A speaker table: for each verse or part-verse, whether God, the voice from
   heaven, or the angel of the LORD speaking as God is the speaker.

Items 3 and 4 have no known open-licence source and are authored under the
two-model protocol with the owner's review.

## What must be tested

- Determinism: the same sources and tables give the same page bytes.
- Coverage: no occurrence of the four-letter name in the Hebrew is left without
  a class; none of “LORD”, “GOD” in small capitals survives on an adapted page.
- Grammar at the join: capitalization at sentence start (“The Blessed One”),
  possessives (“the Blessed One's”), and the article after prepositions and
  vocatives (“O LORD” → OPEN: “O Blessed One”).
- Length: page breaks are computed after the rule is applied.
- Reversibility: the slip reproduces the source's wording exactly.
