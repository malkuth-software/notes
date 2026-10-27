# Source register

Every text, lexicon, dataset, map source and font the product might carry.
Nothing has been downloaded. On 2026-10-03 only the rows marked ✔ had their
licence statement checked against a live page; the rest are from general
knowledge and are **CANDIDATE, unverified**. Stage 1 turns each row into a
record: URL, retrieval date, SHA-256, licence text copy, attribution line,
importer, or a REJECTED entry with the reason.

Status key: CAND = candidate; BLOCKED = cannot ship; ASK = owner must clarify.

> **Amended 2026-10-03 by Stage 1 (sources and licences).** The opening
> paragraph above is superseded: 118 source files are now pinned in
> `sources/lock.json` and 29 licence statements saved under
> `sources/licences/`. The table that follows is the register's current state.
> The older tables below it are kept as written; where they disagree with this
> one, this one prevails. The record for each row — upstream, statement,
> format, counts, caveats — is `sources/README.md`; the measurements are
> `sources/MEASUREMENTS.md`. “Proposed cleared” becomes “cleared” only with the
> owner's signature at gate G1 (`planning/reports/S1-2026-10-03.md`).

## State after Stage 1

Key: **PC** proposed cleared (statement saved, file pinned, format and counts
recorded, importer specified); **PC−** proposed cleared with a named part
excluded; **CAND** pinned or located, a question open; **RESEARCH** read on
this machine only; **REJ** rejected, reason kept; **BLOCKED** as before;
**—** not audited in this stage (not in the first wave).

### Scripture and witnesses

| Work | State | Lock id | Note |
| --- | --- | --- | --- |
| Berean Standard Bible: text, headings, footnotes, cross-references | PC | `berean` | 31,086 verses; the 16 absent verses are in its footnotes. Carries its own words-of-Jesus markup |
| Berean translation tables (interlinear, alignment) | PC | `berean` | The dedication is read as covering them; no sentence names them |
| Berean Literal Bible; Majority Standard Bible | PC | `berean` | `msb.txt` is not UTF-8 |
| STEPBible TAGNT, TAHOT, TVTMS, morphology codes | PC | `stepbible` | CC BY 4.0. Edges recorded: Nestle-Aland spellings; ETCBC morphology |
| World English Bible with deuterocanon; words-of-Jesus markup | PC | `ebible-web` | Trademark on the name |
| Brenton's English Septuagint | PC | `ebible-brenton` | |
| Clementine Vulgate (its Psalms are the Gallican Psalter); Douay–Rheims 1899 | PC | `ebible-vulgate`, `ebible-douay` | |
| 1 Enoch, Charles 1917 | PC | `charles-enoch` | Critical signs handled as structure |
| Jubilees, Charles 1917 | PC | `charles-jubilees` | Via Sefaria |
| Apostolic Fathers, Lightfoot, **English** | PC | `lightfoot-english` | All four kept works present |
| Apostolic Fathers, Lightfoot, **Greek** | CAND | `scans-ocr` | Public domain; only uncorrected OCR exists |
| King James Version with Apocrypha | CAND | `ebible-kjv` | Crown letters patent in the United Kingdom |
| Septuagint, Swete | CAND | `swete-lxx` | CC BY-SA 4.0; share-alike question |
| Septuagint, Rahlfs 1935 (digital) | **REJ** | — | CC BY-NC-SA and the CCAT terms; not fetched |
| Peshitta OT, ETCBC/peshitta | **RESEARCH** | `etcbc-peshitta` | **Correction:** the text is CC BY-NC 4.0; MIT covers the program only. Edition: Leiden (Brill), with Codex Ambrosianus for unpublished books |
| Peshitta printed editions (Lee, Urmia, Mosul) | — | — | No text located |
| Oliver 1861, Psalms from the Syriac | CAND | — | Located (`Oliverpeshittopsalms`); archive.org returned errors on download |
| Psalter *iuxta Hebraeos* | CAND | `scans-ocr` | Lagarde 1874 and Harden 1922 as OCR only |
| SBLGNT; Nestle 1904; Robinson–Pierpont; Westminster Leningrad Codex; Etheridge; Murdock; Targums; Greek Enoch fragments; Qumran Aramaic fragments | — | — | Not first wave. TAGNT and TAHOT are the chosen bases (T010 D, T012 B) |
| Hart; Tree of Life Version; Lamsa; Hagee's lists | BLOCKED | — | Unchanged |
| “The Prophecy Bible”; a sacred-name Bible | void | — | T002; OPEN-6 |

### Lexica and study data

| Work | State | Lock id | Note |
| --- | --- | --- | --- |
| Strong's, Greek and Hebrew | PC | `strongs` | |
| Abbott-Smith | PC | `abbott-smith` | |
| Brown–Driver–Briggs (Open Scriptures) with lexical index | PC | `oshb-hebrew-lexicon` | CC BY 4.0 |
| STEPBible TBESG; TFLSJ (Liddell–Scott–Jones for the Bible's vocabulary) | PC | `stepbible` | TFLSJ answers L001 E |
| STEPBible TBESH | **PC−** | `stepbible` | Numbering and lemmas carried; **definitions REJ** (Online Bible's permission required, by the file's own header) |
| MACULA Greek and Hebrew | **PC−** | `macula-greek`, `macula-hebrew` | United Bible Societies' domain fields not carried |
| Liddell–Scott–Jones, full (Perseus) | CAND, not fetched | — | CC BY-SA, about 270 MB |
| Intermediate Liddell | — | — | No open text located |
| Payne Smith; Trench; Moulton–Milligan | CAND | `scans-ocr` | Public domain; only uncorrected OCR exists |
| Thayer; Dodson; Gesenius; Jastrow; Vincent; Treasury of Scripture Knowledge; OpenBible.info; Nave; Hitchcock; Webster | — | — | Not chosen in L001 |
| BDAG, HALOT, TDNT, NIDNTTE, Louw–Nida text | BLOCKED | — | Unchanged |

### Fonts

| Face | State | Lock id | Note |
| --- | --- | --- | --- |
| EB Garamond (four styles) | PC | `font-ebgaramond` | Passes the BSB and WEB; also covers TAGNT's Greek |
| Gentium and Gentium Book 7.000 | PC | `font-gentium` | Passes TAGNT on the NFC form |
| Ezra SIL and Ezra SIL SR 2.51 | PC | `font-ezra` | Passes all 6,536 Hebrew sequences (static). Layout tables under MIT inside an OFL font |
| Portsmouth | **REJ as built** | `font-portsmouth` | No U+2013 (2,244 uses in the BSB); no small capitals, old-style figures or kerning |
| Frank Ruhl Libre; David Libre | **REJ** | | No cantillation glyphs |
| Cardo | **REJ** for Hebrew | | No anchors for cantillation |
| Noto Serif Hebrew | **REJ** | | Three sequences, 41 occurrences, without an anchor |
| Literata, Source Serif, Cormorant, Libre Caslon, Crimson Pro, GFS Didot | not sought | — | J005 A and F002 A name the faces |
| Noto Sans Syriac; Meltho | not needed | — | Y001 A |
| SBL Hebrew, SBL Greek | BLOCKED | — | Unchanged |

### Maps, and everything in Worship and Practice

Not audited. Maps are Stage 12. The worship, rabbinic and later-Christian
bodies are proposals for the owner to strike (`planning/proposals/`); nothing
was fetched.

### Rejections, in the form of the negative-results ledger

`DECISION_PROTOCOL.md` §4 asks for `planning/NEGATIVE_RESULTS.md` from the
first rejection. That file is a shared planning document outside this session's
ownership, so the entries are kept here and are to be copied there by whoever
next owns it.

| What was tried | Strongest version | What it offered | What failed | What would reopen it |
| --- | --- | --- | --- | --- |
| Portsmouth as a Latin text face (J005 A) | The four styles shipped by Plan Paint 1.1.3 | The owner's own face | U+2013 absent: the BSB cannot be set. No `smcp`, `onum`, `kern` | A build with the en dash; for parity, the dagger and the features |
| Frank Ruhl Libre, David Libre for Hebrew | Regular | Familiar Hebrew book faces | 32 and 30 code points of the Hebrew text absent | A decision that cantillation is never shown in them |
| Cardo for Hebrew | Regular | One face for three scripts | No mark anchors for 685 base-mark pairs | A shaped test showing acceptable placement |
| Noto Serif Hebrew | 2.004, unhinted | Complete code-point coverage | Three real sequences unanchored | A shaped test, or a release with the anchors |
| Rahlfs 1935 as the Septuagint | `eliranwong/LXX-Rahlfs-1935` | The standard hand edition | CC BY-NC-SA; CCAT restrictions | A text of Rahlfs with a plainly open licence |
| TBESH brief Hebrew definitions (L001 B) | STEPBible's file | Short glosses keyed to extended Strong | The file says Online Bible's permission is needed | That permission, in writing |
| ETCBC Peshitta as a shipped or translated source | Leiden text, plain | The standard edition, clean | CC BY-NC: a translation inherits it | The owner accepting a non-commercial condition; ETCBC's or Brill's permission; or a public-domain printed text transcribed |
| MACULA's semantic domains for search (S006 B) | Louw–Nida and SDBH fields | Ready-made domains | “Used with permission”; no licence to pass on | A licence statement from the United Bible Societies |

> **Amended 2026-10-03 after round 002.** Syriac is a build-time source only
> (Y001 A): no Syriac face is needed. Portsmouth is cleared by the owner as
> their own, OFL with Plan Paint's source-font notices (J006 A). No separate
> sacred-name edition is sought for now (E008; OPEN-6). Apostolic Fathers are
> 1 Clement, Didache, Ignatius, Polycarp only (O004). Latin faces are EB
> Garamond and Portsmouth (J005 A). New candidate bodies, each to be proposed
> as a list for the owner in Stage 1: later Christian philosophical writers
> (O004 note; not Luther); hymns, prayers, homilies and pastoral rules; rite
> sources for rabbinic, Anglican, older Roman and Messianic practice.
>
> **Amended 2026-10-03 after round 001.** “The Prophecy Bible” row is void: the
> owner wants a sacred-name Bible still to be identified (round 002 E008). New
> rows are at the end under “Added by round 001”. Syriac may be build-time only
> (Y001). The Greek and Hebrew chosen for display are STEPBible's (T010 D,
> T012 B); the Berean tables remain the alignment source (J007).

## Scripture and witnesses

| Work | Role | Licence as understood | Status | Interview |
| --- | --- | --- | --- | --- |
| Berean Standard Bible, text, headings, footnotes, cross-references | Primary English | Public domain since 2023-04-30 ✔ ([berean.bible terms](https://berean.bible/terms.htm), [eBible](https://ebible.org/details.php?id=engbsb)) | CAND | T004 |
| Berean interlinear / Greek and Hebrew tables | Alignment; base originals | Public domain with the BSB (verify the tables are covered) | CAND | T010, T012, K023 |
| Berean Literal Bible | Input to expansions | Public domain (verify) | CAND | T020 |
| Majority Standard Bible | NT variant notes | Public domain ✔ (same dedication) | CAND | T004 |
| D. B. Hart, *The New Testament: A Translation*, 2nd ed., Yale UP 2023 | Wanted English NT | Copyright Yale University Press | **BLOCKED** for the repository and installer; user-supplied import or licence only | T001 |
| “The Prophecy Bible” Isaiah and prophets | Wanted English | Unidentified | **ASK** | T002 |
| SBL Greek New Testament | Greek NT | CC BY 4.0 | CAND | T010 |
| Nestle 1904 | Greek NT | Public domain | CAND | T010 |
| Robinson–Pierpont Byzantine | Greek NT | Public domain | CAND | T010 |
| STEPBible TAGNT, TAHOT, TBESG, TBESH, TVTMS | Tagged texts, brief lexica, versification map | CC BY 4.0 ✔ ([STEPBible-Data](https://github.com/STEPBible/STEPBible-Data)) | CAND | T010, T012, T017, L001 |
| Westminster Leningrad Codex; Open Scriptures morphology | Hebrew | Text public domain; morphology CC BY 4.0 | CAND | T012 |
| Unicode/XML Leningrad Codex | Hebrew | Public domain | CAND | T012 |
| Septuagint, Swete | Greek OT | Public domain | CAND | T015 |
| Septuagint, Rahlfs 1935 | Greek OT | Text out of copyright in most jurisdictions; digital tagging has its own terms | CAND, needs legal reading | T015 |
| Brenton, English Septuagint 1851 | English for Greek-only books | Public domain | CAND | C004, T016 |
| World English Bible, deuterocanon | English for Judith | Public domain | CAND | C004 |
| Peshitta OT, ETCBC/peshitta | Syriac | Repository shows an MIT licence ✔ ([ETCBC/peshitta](https://github.com/ETCBC/peshitta)); the underlying edition's terms are not yet confirmed | CAND, needs edition check | T006 |
| Peshitta printed editions: Lee 1823, Urmia 1852, Mosul 1887–91 | Syriac | Public domain | CAND; transcription cost | T006 |
| A. Oliver, *Translation of the Syriac Peshito Version of the Psalms*, 1861 | English control for Syriac Psalms | Public domain; scan exists ✔ ([archive.org](https://archive.org/details/Oliverpeshittopsalms)) | CAND | T007 |
| Etheridge 1849; Murdock 1851 | English of Peshitta NT | Public domain | CAND | T005 |
| Lamsa | English Peshitta | Unclear | treat as BLOCKED until proven otherwise | T007 |
| Targum Onkelos, Jonathan | Aramaic witness | Texts public domain; digital editions vary | CAND | C009 |
| 1 Enoch, R. H. Charles 1917 | English | Public domain | CAND | C002 |
| 1 Enoch Greek fragments | Greek | Older editions public domain | CAND | C003 |
| Jubilees, Charles | English | Public domain | CAND | C002 |
| Apostolic Fathers, Lightfoot | Greek and English | Public domain | CAND | C005, C006 |
| Babylonian Talmud, Midrash, Rashi, Ibn Ezra, Ramban, Malbim, Vilna Gaon | Hebrew/Aramaic commentary | Originals public domain; English translations mostly not, or mixed community licences | CAND originals; English is model-made and labelled | C008–C010 |
| Latin Vulgate | Hapax witness | Clementine text public domain | CAND | L013 |

## Lexica and study data

| Work | Licence as understood | Status |
| --- | --- | --- |
| Strong's | Public domain | CAND |
| Abbott-Smith, Manual Greek Lexicon | Public domain | CAND |
| Thayer | Public domain | CAND |
| Liddell–Scott–Jones; Intermediate Liddell | Open digital editions (Perseus, CC BY-SA) — share-alike terms must be read against the product licence | CAND |
| Dodson | Public domain | CAND |
| Brown–Driver–Briggs; Open Scriptures Hebrew Lexicon | Public domain; CC BY 4.0 | CAND |
| Gesenius (Tregelles) | Public domain | CAND |
| Jastrow | Public domain | CAND |
| Payne Smith, Compendious Syriac Dictionary 1903 | Public domain | CAND |
| MACULA Greek and Hebrew (Clear Bible) | CC BY 4.0 | CAND |
| Trench, Synonyms | Public domain | CAND |
| Moulton–Milligan | Public domain in the US (verify dates) | CAND |
| Vincent, Word Studies | Public domain | CAND |
| Treasury of Scripture Knowledge | Public domain | CAND |
| OpenBible.info cross-references and geodata | CC BY | CAND |
| Nave's Topical Bible; Hitchcock's names | Public domain | CAND |
| Webster 1913 / 1828 | Public domain | CAND |
| BDAG, HALOT, TDNT, NIDNTTE, Louw–Nida text | Copyright | **BLOCKED** |

## Maps

| Source | Licence as understood | Status |
| --- | --- | --- |
| Natural Earth | Public domain | CAND |
| Open elevation data (SRTM / Copernicus DEM) | Open; attribution terms differ | CAND |
| OpenBible.info Bible geocoding | CC BY | CAND |
| G. A. Smith, *Atlas of the Historical Geography of the Holy Land*, 1915 | Public domain | CAND |
| Palestine Exploration Fund Survey of Western Palestine | Public domain | CAND |

## Fonts

| Face | Script | Licence as understood | Status |
| --- | --- | --- | --- |
| EB Garamond, Gentium, Literata, Source Serif, Cormorant, Libre Caslon, Crimson Pro, Cardo | Latin (some Greek, Hebrew) | SIL OFL 1.1 | CAND |
| GFS Didot | Greek | SIL OFL | CAND |
| Ezra SIL | Hebrew with cantillation | SIL OFL | CAND |
| Frank Ruhl Libre, David Libre, Noto Serif Hebrew | Hebrew | SIL OFL; cantillation positioning unverified | CAND |
| SBL Hebrew, SBL Greek | Hebrew, Greek | Free of charge; redistribution by permission | BLOCKED unless permission (F013) |
| Noto Sans Syriac (three hands) | Syriac | SIL OFL | CAND |
| Meltho (Beth Mardutho) | Syriac | Own licence; redistribution to be read | CAND, needs reading |
| Portsmouth Rapids (toolkit) | Latin | Owner evaluation build; redistribution unresolved in toolkit | not for this product |

## Attribution obligations

CC BY and CC BY-SA sources require credit in the product. Stage 1 produces the
exact attribution lines and the product's “Sources” page; the packaging carries
licence texts as PlaySuite does (`licenses/` and a per-file manifest).

## Added by round 001

| Work | Role | Licence as understood | Status |
| --- | --- | --- | --- |
| Tree of Life Version | Wanted for some books (T020 note) | Copyright (Messianic Jewish Family Bible Society) | **BLOCKED**; consult-only like Hart (E007) |
| A sacred-name Bible, unidentified | Wanted for the prophets (T002) | Unknown | **ASK** (E008) |
| World English Bible, words-of-Jesus markup | Source for red letters | Public domain | CAND |
| A speaker table for God's speech in the Old Testament | Source for gold (R010 note) | None known with a clear licence | to be authored and reviewed, or found (D011) |
| Full deuterocanon: WEB, KJV Apocrypha, Brenton, Septuagint text | C001 C, C004 D | Public domain | CAND |
| Jubilees, R. H. Charles 1902/1913 | C002 C | Public domain | CAND |
| Qumran Aramaic Enoch fragments | C003 D note: own translation | The ancient text is free; published transcriptions and reconstructions may carry rights | CAND, needs a rights reading per edition |
| Mishnah, Midrash Rabbah, halakhic midrash, Pirkei Avot | C008 B, C009 | Originals public domain | CAND; English is ours |
| Rashi and Tosafot on the Talmud | C007 note | Originals public domain | CAND; English is ours |
| Patristic excerpts: ANF / NPNF series | C011 B | Public domain | CAND |
| Latin Psalters: Gallican and iuxta Hebraeos; Douay–Rheims as control | T005 note; Y004 | Public domain | CAND |
| Book of Common Prayer 1549; American 1928 | Worship and Practice | Public domain (1662 is under Crown privilege in the UK) | CAND |
| Singer, Authorised Daily Prayer Book 1890 | Worship and Practice | Public domain | CAND |
| Traditional Haggadah; Roman Missal and Liturgy of St John Chrysostom in older English | Worship and Practice | Texts public domain; each translation to be checked | CAND |
| Hymn texts, pre-1929 | Worship and Practice | Public domain | CAND; list by U006, U008 |
| Weekly Torah portions and haftarot; Prayer Book lectionary | Shipped subjects (N015 note) | Traditional / public domain | CAND |
| Hagee study-Bible subject lists | N015 note | Copyright | **BLOCKED**; may suggest topics only |
| Portsmouth (owner's face) | Latin option (F001 note) | Plan Paint ships it with OFL and MIT source-font notices; GUI.Forms records redistribution as unresolved | **ASK** (J006) |

## Rules for this register

- A row moves to “cleared” only with a saved copy of the licence statement, the
  URL, the date and the hash of what was fetched.
- A row whose licence cannot be established is REJECTED, with the reason kept.
- Share-alike sources are not mixed into a single compressed dictionary with
  other sources until the product licence question (A010) is settled.
- A source that is cleared for redistribution may still be excluded by
  `PRODUCT_NEGATIVE.md`.
