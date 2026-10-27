# Expansion protocol

> **Amended 2026-10-03 after round 001** (see `RECONCILIATION_001.md` §1, §2):
> the owner corrects that the exemplar is not personal conviction but the
> remedy of concealment; an expansion is a meaning expansion, never a
> translation; books may ship audited before the owner has read all of them
> (the Mark pilot still calibrates voice first); two models argue and may read
> outside sources, and the owner breaks ties by interview. The same protocol
> now governs every authored work: merged lexicon entries (L002 A), key terms,
> the rendition, translations, rite explanations. Open points are round 002
> V004–V006 and G001.

Status: HYPOTHESIS. The voice gate (owner review of the Mark pilot) must pass
before any of this scales. Interview section X governs every choice marked.

## What an expansion is

Where the English of a verse does not carry the full meaning of an
original-language word or idiom, an expansion is a concise authored statement of
what is lost, anchored to that word, shown in a slip. It is commentary. It is
never presented as scripture and never replaces the translation on the page.

The owner's exemplar (Mark 7:21–22) is in `OWNER_BRIEF_2026-10-03.md`. The owner
calls it “a selective interpretation of the expansion of the Greek words and
their actual meanings”. Two things follow:

- The exemplar's voice — terse, concrete, moral and psychological — is the
  target voice (X008) until the owner says otherwise.
- Some of the exemplar's lines restate a sense the lexica give (ὀφθαλμὸς
  πονηρός as envy and begrudging); others draw the meaning out further than a
  lexicon entry states (κλοπαί as “warring against another's portion”). The
  owner's position is that both undo concealment. The protocol keeps both and
  labels which evidence each rests on (X005 A), so a reader can always see how
  far the stored sources carry a line.

## The absolute rule

A model writing or auditing expansions reads only what is stored: the texts,
morphology, lexicon entries and witnesses in the repository's normalized works.
It does not supply lexical facts, usage citations, etymologies or quotations
from its own recall. If the stored sources do not support a claim, the claim is
not made, or it is labelled as the author's interpretation with no evidence
line.

## Source format

One file per book under `authored/expansions/`, line-oriented and diffable
(X011). Proposed record, to be fixed in Stage 10 (identifiers and values are
illustrative):

```
@ MRK 7:22 #9            anchor: book chapter:verse, original-token ordinal
lemma    G3788+G4190     identity of the anchored word(s)
shows    envy            the BSB word(s) it explains (for review only)
phrase   malicious begrudging of the prosperity of others and belief in the
         insufficiency of one's own portion
label    idiom
evidence abbott-smith:ὀφθαλμός §2 ; bdb:עַיִן ; verse:DEU 15:9 ; verse:PRO 28:22 ; verse:MAT 20:15
more     Literally "an evil eye": a Hebrew idiom for envy and stinginess.
author   <model id> rev 1 2026-..-..
audit    <model id> supported 2026-..-..
```

- `label` is one of: `attested` (a sense the cited entry gives), `etymology`
  (derivation the cited entry gives; never offered as meaning by itself),
  `idiom`, `usage` (established by cited passages), `interprets` (goes beyond
  the cited entries; the author's reading), `disputed` (readings listed with
  their support).
- `evidence` items resolve to stored records. The build fails on an expansion
  whose label requires evidence and has none, on an evidence item that does not
  resolve, on a `phrase` over the length ceiling (X021), and on an anchor that
  does not exist in the base text.
- Etymology is a known trap: a word's derivation is not its meaning in use. An
  expansion may report derivation under `etymology`; it may not present
  derivation as what the speaker meant unless usage evidence supports it.

## Reading pass (authoring)

Per verse, the sibling model is given (X007): the BSB verse and its neighbours;
the original with morphology; the stored lexicon entries for each lemma;
Septuagint, Peshitta and Targum renderings where stored; a literal rendering;
parallel passages; the voice guide and exemplar.

It answers one question per content word: does the BSB wording lose something a
careful reader would want? Most words: no — and no expansion is written (X001
A). The absence of a mark means the English is adequate.

## Audit pass

A second, independent model (X006) receives only the expansion record and the
stored evidence it cites, and answers: does the evidence support this wording?
Outcomes: `supported`, `overreaches` (with the unsupported part quoted),
`unsupported`, `wrong anchor`. Anything but `supported` goes to the owner's
queue or back to authoring; it does not ship.

## Owner review

- **Voice gate (G4)**: the whole of Mark, authored and audited, reviewed by the
  owner in the client or as a printed list. The owner's edits become the voice
  guide. Nothing scales before this.
- Thereafter a fixed sample per book plus every audit disagreement (X010), and
  an in-client “flag this expansion” that writes to a local errata file.

## Order of the campaign

Mark → the other Gospels → the rest of the New Testament → Torah → Psalms →
Prophets → the rest (X022), each book a separate reviewed change.

## Related authored works under the same protocol

- **Context notes** (X020): one short note per passage; evidence from stored
  sources only.
- **Model-made translations** (T022): Syriac Psalms, rabbinic excerpts, the
  Vilna Gaon. Two independent translations; agreement in sense is shown;
  disagreement is queued. Each carries “translated for this edition by <model>,
  reviewed: <state>”. Oliver 1861 is an independent control for the Psalms.
- **Topic tags** (S006 F), if admitted.

## What this protocol does not establish

- That a model can hold the exemplar's voice across 31,000 verses. The pilot
  tests it.
- That audit by a second model catches subtle overreach. The owner's sample
  measures the miss rate; the rate is reported.
- That the lexica admitted by licence are sufficient evidence for Old Testament
  idiom. Where they are thin, the honest output is fewer expansions.
