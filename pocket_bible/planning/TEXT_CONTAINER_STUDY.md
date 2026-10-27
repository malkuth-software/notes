# Text container study — protocol

Status: protocol only. No corpus has been fetched and nothing has been measured.
Interview section K sets the priorities this study serves.

## Question

How small, and at what decoder cost, can the admitted corpus be stored when the
coder knows it is this corpus: these languages, these structures, these aligned
witnesses?

## What the owner asked for (GIVEN)

A content-specific compression table, built by studying the text, in the manner
of tokenizer work; deterministic decoding that knows the formatting; a tiny
database engine; a sparse container; text compressed primarily with other text;
most formatting characters absent from the representation.

## Oracle

Byte-exact round trip from container to the normalized form of every work
(interview K012, K013). The importer that produces the normalized form, and the
list of upstream markup deliberately not carried, are part of the record. A
candidate that fails the round trip on any verse is not measured further.

## Controls

On the same normalized bytes, per work and for the whole corpus: raw UTF-8;
zstd at its highest level, plain and with a dictionary trained on the corpus;
xz; PPMd; one context-mixing compressor as a reference bound that will never
ship (K016). Controls are run both on whole works and on chapter-sized blocks,
because the product needs random access (K009) and whole-file numbers flatter
the controls.

## Candidate axes

Each axis is varied with the others held at a stated default. Every cell of the
table is kept, including the losers.

1. **Token unit** (K002): surface words; lemma + morphology with an exception
   list; learned subwords; words + learned phrases; combinations.
2. **Phrase learning rule**: greedy by total bits saved net of dictionary cost;
   deterministic tie-breaking; the rule and its output are recorded so the same
   corpus always yields the same table.
3. **Dictionary sharing** (K014): per work; per language; shared base plus
   per-work additions.
4. **Formatting model** (K006): implied spaces by token class; rule-derived
   capitalization; count and kind of flagged exceptions per work.
5. **Structure** (K007, K008): index streams against inline markers; layout
   stream run-length coding.
6. **Hebrew layering** (K011): skeleton + vowel + accent layers against pointed
   words; the same question for Greek accents and breathing and for Syriac points.
7. **Cross-text prediction** (K003): English given the aligned original token;
   Septuagint given Hebrew; Peshitta given Hebrew; Greek and English of the
   Apostolic Fathers.
8. **Entropy coder** (K004): variable-length integers; canonical Huffman; static
   rANS/tANS.
9. **Context** (K005): order 0; previous token; previous token's class.
10. **Block size** (K009): verse; passage; chapter; fixed token count.

## What is recorded for each candidate

- Bytes: streams, dictionaries, tables, indexes — separately and in total. A
  number that omits the dictionary is not reported.
- Bits per character of the normalized form.
- Decoder: lines of code, tables in memory, working memory to open one chapter,
  time to open one chapter (p50, p95, max) on the named machine.
- Whether search can run on the coded form without decoding text (K018).
- What the candidate does not establish.

## Study of the text itself

Before any coder: vocabulary size and frequency distribution per work; share of
tokens covered by the top N words; repeated phrases by length and count;
formulaic lines (genealogies, refrains, doubled Psalms, synoptic parallels,
Kings/Chronicles) and how much of the corpus they are; how regular the BSB's
rendering of each lemma is (the entropy of English given the original), which
bounds axis 7 before it is built; how many spaces and capitals are predictable
by rule. These tables are useful to the product in their own right: they are the
concordance and the rendering-variety view of interview L014.

## Decision

The study ends in a decision record choosing a format, with the table of
candidates, the reasons the others lost, and a reversal path (the normalized
works and importers remain in the repository, so any later format can be built
from them). The format is frozen only after the proving slice has rendered pages
from it (gate G3 in `STAGE_PLAN.md`).

## Guard rails

- No target size is promised in advance (K022 D is the suggested answer).
- The decoder must stay small enough to verify by reading. A candidate that
  wins on bytes and loses on that is reported as such, not quietly chosen.
- Reproducible container bytes on every platform (K019); the hash is part of
  the release.
- Mixed-licence sources are not merged into one dictionary until the licence
  question is settled (`SOURCE_REGISTER.md`).
