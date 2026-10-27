# Architecture hypotheses

Status: **HYPOTHESIS throughout.** This is the component map I would test, with
the reasoning behind it, written so that Stage 0 has something concrete to hold
the interview answers against. None of it is decided. Where an interview
question governs a choice, it is named.

## 1. Shape of the program

```
 sources (hash-pinned)                    repository (authored)
        │                                        │
        ▼                                        ▼
  importers ──► normalized works ◄── expansions, notes, translations, maps
                       │
                       ▼
                 corpus builder ──► one container file (read-only, checksummed)
                                              │
            ┌──────────────┬──────────────────┼───────────────┬─────────────┐
            ▼              ▼                  ▼               ▼             ▼
        text reader     search           lexicon/hapax     map reader   layout data
            └──────────────┴────────┬─────────┴───────────────┴─────────────┘
                                    ▼
                             page compositor ──► complete page images ──► GUI.Forms window
                                    ▲
                             user marks (separate small files)
```

Three libraries and one executable, following Plan Paint's split:

- `bible_core` — toolkit-free: container reader, token streams, references,
  versification, search, lexicon access, marks model, storage. Fully testable
  without a window. Builds on every platform with `-DBIBLE_BUILD_APPLICATION=OFF`.
- `bible_page` — toolkit-free: the typesetter and rasterizer. Input: tokens,
  layout stream, faces, page geometry. Output: a page image and a hit map.
- `bible_forms` — the GUI.Forms controls: book view, slips, search slip, map
  view, input, accessibility exposure.
- `bible_build` — command-line corpus builder and study tools (not shipped).

## 2. The container (interview K; `TEXT_CONTAINER_STUDY.md`)

One read-only file, memory-mapped. A section table of fixed-width records, each
section with offset, length, count and digest — File Manager's storage-spine
pattern. HYPOTHESIS for the sections:

- **Dictionaries**: per script, sorted; entry = normalized form + class flags.
  For Hebrew, the consonant skeleton is the entry; vowels and accents are
  separate sparse layers keyed by token position (K011).
- **Token streams**: per work, per chapter block (K009). Tokens are words and
  learned multi-word phrases (K002). No space, case or verse-marker tokens:
  spacing is implied by token class with flagged exceptions; capitalization is
  rule-derived with flagged exceptions (K006).
- **Structure index**: book → chapter → verse offsets into the token stream (K007).
- **Layout stream**: paragraph, poetry level, heading, footnote anchor, speaker
  runs; parallel to the token stream (K008).
- **Alignment**: English token ↔ original token links.
- **Versification map**: native numbering of each witness ↔ master numbering (T017).
- **Lexicon**: lemma table (extended Strong's as key, L003), entries per source.
- **Authored**: expansions and notes keyed by original-language token identity
  (X012), with evidence references into the lexicon section.
- **Search**: vocabulary → posting lists of token positions; folded-form maps
  for Greek, Hebrew and Syriac; the single-edit candidate structure.
- **Maps**: vector features, gazetteer with confidence, period layers.
- **Manifest**: every source's identity, licence, hash; format version.

The property that matters most: **token identity is the shared key.** The same
number addresses a word for decoding, search postings, alignment, expansion
anchors, highlight anchors (H003, H013) and the shaping cache (§3).

“Compressed with other text” (K003) has two readings to measure: a shared
dictionary across works, and conditional coding of a translation given the
aligned original. The second is elegant and couples every English read to an
original read; the study reports its real saving before anyone commits.

## 3. The page compositor (interview F010, R, P)

OBSERVED constraint: the toolkit cannot typeset this page (see
`TOOLKIT_NOTES.md` §4). HYPOTHESIS: the application owns a compositor that
produces complete page images off the UI thread and publishes only finished
pages — the toolkit's complete-frame law applied by the application.

Pipeline for one page:

1. **Gather**: tokens and layout runs for the page's span, per column.
2. **Shape**: each distinct (token, face, size) is shaped once with HarfBuzz and
   cached as a positioned glyph run. The token dictionary bounds the cache: a
   Bible has tens of thousands of distinct words, not millions. Hebrew shapes
   skeleton plus active mark layers as one cluster sequence. Bidi is resolved per
   paragraph with SheenBidi; a Hebrew or Syriac verse is a single-direction
   paragraph, which keeps the hard cases rare (numerals, embedded Latin).
3. **Break**: paragraph-level line breaking (total-fit in the Knuth–Plass manner)
   over word boxes with stretchable glue; hyphenation points from a pattern
   dictionary for English (R006). Verse numbers, drop numerals and footnote marks
   are boxes with their own rules. Aligned parallel mode (R003 A) breaks each
   verse pair in two columns and equalizes row height.
4. **Paginate**: fill columns; keep headings with what follows; place footnotes
   (R011). Pagination is a pure function of (text span, geometry, type settings),
   cached per book as a list of page starts.
5. **Raster**: FreeType coverage, greyscale (F007), blended in linear light over
   the paper tile (F009); highlights under the ink; marks for expansions.
6. **Hit map**: per page, a sorted list of word rectangles with token identity,
   so pointer → token is a binary search, and selection, highlights and slips
   need no toolkit text geometry.
7. **Publish**: upload as a premultiplied BGRA image; the control paints it with
   `draw_image`. Hover and selection change small rectangles through
   `patch_bgra32_premultiplied`.

Budget HYPOTHESIS to measure in Stage 4: two page images resident (the spread),
one glyph-coverage cache, one shaping cache; neighbours composed on demand.

Alternatives kept as CANDIDATES: text-mask service per run (F010 B); build-time
shaping stored in the container (F010 C); a toolkit prepared-layout interface
(F010 D), filed in `docs/TOOLKIT_REQUESTS.md` regardless.

Accessibility: the page image is opaque to a screen reader. The control must
publish the page's verses to the toolkit's semantic graph (I011 B).

## 4. Book furniture (interview P, N)

All static pixels, computed once and cached: the paper tile (P013), gutter and
curvature shading as a multiply layer baked into the page image, the edge stack
drawn from the current position (P004), thumb notches and ribbons as
application-rasterized sprites with alpha. The toolkit painter has no path fill,
so curves are rasterized by the application (a small anti-aliased scanline
filler; PlaySuite's `presentation.*` has one for polygons).

Nothing animates at rest. A page turn (R018) is a bounded sequence of frames
with a `Timer` that stops; reduced motion makes it instant.

## 5. Search (interview S)

Law taken from File Manager: approximate channels propose, the exact scan
decides, the result states its coverage.

- **Oracle**: exhaustive scan of token streams for the query's token sequence.
  At this corpus size it is interactive, and every accelerator must return
  exactly its results.
- **Exact index**: vocabulary → positions; phrase by position intersection.
- **Folded forms**: accent-stripped Greek, final-sigma folding, unpointed
  Hebrew and Syriac, each a declared versioned mapping (S004 C).
- **Spelling candidates**: single-edit neighbourhood over the vocabulary (tens of
  thousands of words; a plain deletion dictionary may beat the hashed tuple at
  this size — measure), then exact verification, then postings (S004 A).
- **Word pairs**: adjacency in either order, skipping function words (S005 B).
- **Original-language link**: a query word → the lemmas it translates anywhere →
  every verse with those lemmas, grouped by English rendering (S006 A). This is
  the one “semantic” channel whose every result can explain itself.
- **Other channels** (lexicon domains, cross-references, co-occurrence, tags,
  vectors) are admitted only by winning on the owner's golden queries (S015,
  S016).

## 6. Word study and hapax (interview L)

Hapax sets are computed at build time from the stored texts: lemma frequency per
Testament and corpus (L009). Each proximal word is produced by a named channel
with its evidence row (L010); the client shows reasons, not a blended score
(L012 A), unless the owner chooses otherwise. Septuagint translation-equivalent
tables are derived from aligned Hebrew and Greek where an open alignment exists;
where none exists the channel is absent rather than guessed.

## 7. Authored layer (interview X; `EXPANSION_PROTOCOL.md`)

Source files in the repository, compiled into the container; anchored to
original-language tokens; every claim cites a stored entry; a build check
rejects uncited or unlabelled expansions; a second model audits; the owner
reviews samples and disagreements. No model in the client.

## 8. Marks (interview H, N)

Highlights, notes, subjects and ribbon positions in small human-readable files
in the per-user state directory; PlaySuite's envelope pattern (magic, version,
checksum, temporary-then-rename) with rolling backups (H010). Anchors are token
identities with a verse fallback (H013).

## 9. What would falsify this shape

- Stage 4 shows the compositor cannot hold a page turn under a stated time on
  the reference machine at 2× density with justified Hebrew → reconsider F010 C
  (build-time shaping) or a lower-cost layout.
- Stage 3 shows the content-specific coder beats a trained-dictionary zstd by
  too little to justify its own decoder → use the general compressor per chapter
  block and keep only the token streams for search.
- The token-identity key proves unstable across source revisions → marks and
  expansions need a second, text-based anchor.
