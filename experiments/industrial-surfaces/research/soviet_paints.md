# Soviet industrial paint colours (1950s–1980s): documented sources

Research date: 2026-10-31. All values below cite their source. Value classes:

- **(a)**: numeric value given by a standard (GOST) or official conversion table.
- **(b)**: numeric value from a secondary source (vendor, hobbyist digitisation, restorer).
- **(c)**: no numeric value found. Name and/or card-index number only.

Hex values marked "conv." are my own conversion from the xyY in the cited
standard. The method is in the section "Conversion method". No hex value in
this file was eyeballed or sampled from a photograph.

## 1. Summary

**Documented:**

1. **The colour system.** Soviet enamel GOSTs specify colour by a pair of
   swatches from the *«Картотека образцов (эталонов) цвета лакокрасочных
   материалов»* (the "Kartoteka"). ГОСТ 12.4.026-76 Appendix 2 states the rule:
   «норму цвета устанавливают двумя образцами (эталонами) цвета "Картотеки"».
   The pair bounds the permitted tolerance. It is not two different colours.
   ГОСТ 9825-73 lets a colour name carry the first number of its pair ("серая
   571"), which is why names like «голубой 423» or «голубовато-серый 564»
   exist. Hundreds digits group by hue: 0xx–1xx red/orange, 2xx yellow/cream,
   3xx green/blue-green, 4xx blue, 5xx grey (bluish/greenish), 6xx
   beige/brown, 7xx khaki/protective, 8xx greys/neutrals. This grouping is my
   inference from the GOST tables and the laz-legend list, not stated by a
   source.
2. **Machine-tool colours have official colorimetry.** ГОСТ 22133-86
   (lacquer-paint coatings of metal-cutting, forging, foundry and woodworking
   equipment) lists 10 "main" exterior colours and 5 auxiliary colours. It
   gives Kartoteka numbers and **CIE x, y and reflectance (%)**, measured on
   the «Радуга-2» instrument under illuminant C, specular component included.
   This is the single most valuable source. It covers exactly the pale mint
   (светлый серо-зелёный 319), grey-green (серо-зелёный 365) and dark teal
   (сине-зелёный 397) look of Soviet machine tools.
3. **Enamel GOSTs map named colours to card numbers.** ПФ-115 (ГОСТ 6465-76),
   ПФ-133 (ГОСТ 926-82), НЦ-132 (ГОСТ 6631-74), НЦ-256 (ГОСТ 25515-82, the
   machine-tool enamel), МЛ-12 (ГОСТ 9754-76), МЛ-152 (ГОСТ 18099-78) and
   ХВ-124 (ГОСТ 10144-89) all have their colour-to-card tables.
4. **Safety colours.** ГОСТ 12.4.026-76 gives Kartoteka numbers and CIE
   chromaticity regions for its signal colours. ГОСТ Р 12.4.026-2001 Table В.1
   cross-lists the Kartoteka against RAL, Munsell and Pantone for the signal
   colours only.
5. **Control panels.** ОСТ 36.13-90 (switchboards and consoles for process
   automation) names enamels and colours: grey, light grey, and grey-blue for
   tropical versions. It gives no card numbers.

**Not documented (gaps):**

- No official conversion table from the Kartoteka to RAL, Munsell or Lab
  exists for the general palette. A 2016 machinist forum thread (chipmaker.ru)
  searching for exactly this came up empty. Colorimetry exists only where a
  GOST prints it (ГОСТ 22133-86, ГОСТ 12.4.026), and per the
  chem21/«Справочник химика» snippet on the backs of older cards.
- **No numeric values found for the turquoise / sea-wave / grey-blue swatches**
  (330, 387, 377, 379, 320, 491, 495, 498, 463, 564) beyond one hobbyist
  RGB digitisation of unknown method (class b, laz-legend.ru).
- **Power-plant control rooms (ChNPP, LNPP, Kursk, hydro): no source names
  the paint.** No Minenergo or ОСТ 108 rule on panel colour was found.
  Statements about "Chernobyl green/turquoise" are photo impressions only.
- No GOST colour for KRU/KSO switchgear cabinets was found. Modern makers use
  RAL 7035, which is not Soviet evidence.
- The 1950s–60s predecessors (ГОСТ 22133-76, earlier ЭНИМС norms, and pre-1985
  Kartoteka editions, ТУ 6-10-1449-85 and earlier) were not found in text.

## 2. Colour table

"GOST xyY" means CIE 1931 x, y under illuminant C with Y = reflectance %
(ГОСТ 22133-86, Table 7). "laz" means the hobbyist RGB list at laz-legend.ru,
for the first number of the pair (class b; method unstated, probably
scan/screen-sampled). It is cross-checked against GOST in section 3.4.

### 2.1 Machine-tool palette with official colorimetry (ГОСТ 22133-86)

| Russian name | English gloss | Card no. (T3 pair) | Enamels (GOSTs listing this name+pair) | Value | Class | Equipment use (source) | Sources |
|---|---|---|---|---|---|---|---|
| «Слоновая кость» | ivory | 224 (224, 229) | НЦ-256 | x .335 y .350 Y 56.8±1.0 → conv. #CCC7AC, Lab 80.1/−3.1/14.4 | a (conv.) | exterior of metal-cutting/forging/foundry/woodworking equipment (ГОСТ 22133 §9.1.2) | S1, S2 |
| Светло-бежевый | light beige | 661 (661, 662) | НЦ-256, МЛ-152, МЛ-12 | x .352 y .351 Y 34.2±0.5 → conv. #AE9B84; laz #a89e7b | a (conv.); b | same | S1, S2, S7, S8, S13 |
| Коричневый | brown | 619 (619, 622) | НЦ-256, ПФ-115 | x .357 y .339 Y 10.0±0.3 → conv. #68554C; laz #55452c | a (conv.); b | same | S1, S2, S3, S13 |
| **Светлый серо-зелёный** | **light grey-green (pale mint)** | **319 (319, 352)** | НЦ-256, НЦ-132 | x .311 y .339 Y 52.4±0.7 → conv. **#B4C4B2**, Lab 77.6/−9.0/7.2; laz 319 #b1ccb9, 352 #a6c5b3 | a (conv.); b | same. Forum restorer states 16К20 and 676 machines were factory-painted 352 or 365 (S10) | S1, S2, S5, S10, S13 |
| **Серо-зелёный** | **grey-green** | **365 (365, 396)** | НЦ-256, НЦ-132 | x .300 y .333 Y 27.5±0.7 → conv. **#80948A**, Lab 59.5/−9.3/3.0; laz 365 #729388, 396 #79998c | a (conv.); b | same (S10 as above) | S1, S2, S5, S10, S13 |
| **Сине-зелёный** | **blue-green ("industrial teal")** | **397 (397, 398)** | НЦ-256 | x .269 y .300 Y 12.4±0.3 → conv. **#4D676D**, Lab 41.9/−7.8/−6.7; not in laz list | a (conv.) | same | S1, S2 |
| «Белая ночь» | "white night" (pale grey-green-white) | 858 (858, 859) | НЦ-256, МЛ-152, МЛ-12 | x .319 y .333 Y 48.8±0.9 → conv. #B9BBAE; laz #b3c1b2 | a (conv.); b | same | S1, S2, S7, S8, S13 |
| Серый | grey | 840 (840, 841) | НЦ-256, МЛ-12 | x .317 y .324 Y 28.8±0.5 → conv. #94928D; laz #808c88 | a (conv.); b | same | S1, S2, S8, S13 |
| Серо-серебристый | grey-silver | 810 (810, 845) | НЦ-256 | x .302 y .318 Y 29.7±0.5 → conv. #8D9695 | a (conv.) | same | S1, S2 |
| Тёмно-серый | dark grey | 832 (T7) / 822 (T3); pair 832, 847 in НЦ-256 | НЦ-256 | x .292 y .305 Y 15.3±0.3 → conv. #656F74 | a (conv.) | same | S1, S2 (**conflict**, §4) |
| Кремовый | cream | 270 (270, 271) | НЦ-256, НЦ-132 | x .374 y .376 Y 61.5±1 → conv. #E7CA96; laz #efda97 | a (conv.); b | internal surfaces not seen by operator: bed interiors, hydraulics/lubrication, **cabinets and niches with electrical equipment** (§9.1) | S1, S2, S5, S13 |
| Оранжевый | orange | 121 (121, 128) | МЛ-12, ПФ-133 | x .506 y .385 Y 29.5±0.5 → conv. #E0783A | a (conv.) | exterior elements of robotic complexes | S1, S6, S8 |
| Белый | white | control sample | — | reflectance ≥ 75 % | a (partial) | interiors of electrical cabinets/niches; scale backgrounds | S1 |
| Чёрный | black | control sample | — | reflectance ≤ 7 % | a (partial) | anvils, earthing busbars, inscription backgrounds | S1 |

ГОСТ 22133-86 Appendix 4, Table 11 recommends two-colour schemes (S1). The
numbers refer to the rows above:

- 4 светлый серо-зелёный goes with 5, 6.
- 5 серо-зелёный goes with 1, 4, 6, 7.
- 6 сине-зелёный goes with 4, 5, 7.
- 7 белая ночь goes with 3, 5, 6, 8, 9, 10.

Same-hue, different-lightness pairs are preferred, with at most three colours
per machine excluding signal colours. So the documented "Soviet machine" looks
are pale mint over grey-green, and grey-green or white-night over dark teal.

Appendix 5 lists the hammer-finish (молотковые) enamels МЛ-165 (ГОСТ 12034)
that go with these colours:

- серо-бежевый, with 1–3.
- серо-зелёный, with 1, 4, 5, 7.
- **серо-голубой**, with 6, 7, 9.
- **голубовато-серый**, with 6, 7, 9, 10.
- серебристый, with 7–10.

The hammer colours are set by control samples only, so they are class c
(S1, S12).

### 2.2 Blue-green, turquoise, grey-blue and mint colours from enamel GOSTs (no official colorimetry)

| Russian name | English gloss | Card pair | Enamel (GOST) | Value | Class | Equipment use | Sources |
|---|---|---|---|---|---|---|---|
| Серо-голубой | grey-blue | 491, 492 | ПФ-115 (6465-76), МЛ-12 (9754-76), МЛ-152 (18099-78) | laz 491 #437180, 492 #4c777e | b | ОСТ 36.13-90 Table 34 names серо-голубой (ХВ-16) for tropical (ТВ) switchboard/console panels, with no card number (S9) | S3, S7, S8, S9, S13 |
| Серо-голубой (НЦ-132) | grey-blue | 463, 464 | НЦ-132 (6631-74) | laz 463 #5a94a0, 464 #5f939e | b | general indoor | S5, S13 (**conflict** with 491) |
| Светлый серо-голубой | light grey-blue | 495, 496 | МЛ-12, МЛ-152 | laz 495 #8cb5b7, 496 #8cb4b3 | b | — | S7, S8, S13, S14 |
| Голубовато-серый 564 | bluish grey | 564, 565 | МЛ-12, МЛ-152 | laz 564 #69878f, 565 #67888d | b | — | S7, S8, S13 |
| Голубовато-серый 520 | bluish grey | 520, 521 | МЛ-152 | none found | c | — | S7 |
| Зелёно-голубой 498 | green-blue (deep teal-blue) | 498, 499 | МЛ-12, МЛ-152 | laz 498 #186575, 499 #1a667d | b | — | S7, S8, S13 |
| Зелёно-голубой 442 / светлый зелёно-голубой | green-blue | 442, 445 / 443, 444 | МЛ-12 | none found | c | — | S8 |
| Сине-зелёный (МЛ) | blue-green | 375, 376 | МЛ-12, МЛ-152 | none found | c | — | S7, S8 (**differs** from ГОСТ 22133 сине-зелёный 397) |
| «Морская волна» | sea-wave | 330, 387 | ПФ-133 (926-82), МЛ-12 | laz 330 #318084, 387 #17707e | b | ПФ-133 is for freight rolling stock and containers (S6) | S6, S8, S13 |
| «Морская волна» (МЛ-152) | sea-wave | 377, 378 | МЛ-152 | none found | c | — | S7 (**conflict** with 330, 387) |
| Бирюзовый | turquoise | 320, 326 | МЛ-12 | laz 320 #1e9e8f (326 not listed) | b | — | S8, S13 |
| Светло-бирюзовый | light turquoise | 379, 380 | МЛ-12, МЛ-152 | none found | c | — | S7, S8 |
| Фисташковый | pistachio | 388, 389 | ПФ-115, ПФ-133, НЦ-132 ("фисташковый 388") | laz 388 #6f9b77, 389 #6f927e | b | — | S3, S5, S6, S13 |
| Фисташковый (МЛ-12) | pistachio | 381, 382 | МЛ-12 | laz 381 #7aa27d, 382 #819f79 | b | — | S8, S13 (**conflict**) |
| Фисташковый 309 | pistachio | 309, 328 | НЦ-132 | laz 309 #7dac80, 328 #87b084 | b | — | S5, S13 |
| Бледно-зелёный | pale green | 340, 360 (НЦ-132) / 340, 368 (МЛ-12) | НЦ-132, МЛ-12 | laz 340 #a0cfad, 360 #95ccaf, 368 #a2c6ac | b | — | S5, S8, S13 |
| Светлый серо-зелёный (НЦ-132) | light grey-green | 319, 352 | НЦ-132 | see §2.1 (class a) | a | — | S5 |
| Серо-зелёный (НЦ-132) | grey-green | 365, 396 | НЦ-132 | see §2.1 (class a) | a | — | S5 |
| Тёмный сине-зелёный | dark blue-green | 356, 357 | НЦ-132 | laz 356 #294948, 357 #2b4846 | b | — | S5, S13 |
| Зелёный | green | 343, 344 (ПФ-115) / 300, 302 (ПФ-133) | ПФ-115, ПФ-133 | laz 343 #3a7944, 300 not listed, 302 #517e3d | b | — | S3, S6, S13 |
| Ярко-зелёный | bright green | 324, 329 | МЛ-12 | laz 324 #45a063, 329 #4a9d69 | b | 324, 325, 329, 385 are listed as signal-green samples (S11) | S8, S11, S13 |
| Голубой 423 | light blue | 423, 424 | ПФ-115, ПФ-133 | laz 423 #295c89, 424 #1a6396 | b | 423, 424 are listed as signal-blue samples (S11) | S3, S6, S11, S13 |
| Голубой 451 | light blue | 451, 452 | ПФ-115 | laz 451 #2b8cbf, 452 #388ec1 | b | — | S3, S13 |
| Светло-голубой | pale blue | 448, 467 (НЦ-132) / 404, 407 (МЛ-12) | НЦ-132, МЛ-12 | laz 448 #88c2d8, 467 #8dc7d5, 404 #78aac1, 407 #7ab1c5 | b | — | S5, S8, S13 |
| Светло-серый 505 | light grey | 505, 513 | НЦ-132, МЛ-12, МЛ-152 | laz 505 #9db4ae, 513 #99b7af | b | ОСТ 36.13-90 Table 33 lists светло-серый (ПФ-218ГС, НЦ-132П) for panels (S9) | S5, S7, S8, S9, S13 |
| Светло-серый 583 | light grey | 583, 584 | ПФ-115, НЦ-132 | laz 583 #adb1a2, 584 #b5baa6 | b | as above, without saying which light grey | S3, S5, S9, S13 |
| Серый (ПФ-115) | grey | 522, 536 | ПФ-115, МЛ-152 | laz 522 #929f8e, 536 #95a291 | b | ОСТ 36.13-90 Table 33 variant 1: ПФ-115 серый, glossy, for panels (S9) | S3, S7, S9, S13 |
| Серый (ХВ-124) | grey | 881, 882 | ХВ-124 (10144-89) | laz 881 #708479, 882 #728778 | b | ОСТ 36.13-90 Tables 33, 34: ХВ-124 серый, semi-matte, for panels (S9) | S4, S9, S13 |
| Тёмно-серый (ПФ-133/МЛ-152) | dark grey | 526, 527 | ПФ-133, МЛ-152 | laz 526 #546666, 527 #5b6d6d | b | — | S6, S7, S13 |
| Тёмно-серый 894 / 896 | dark grey | 894, 895 / 896, 897 | ПФ-115 | laz 894 #566868, 896 #47595b | b | — | S3, S13 |
| Защитный | protective (olive) | 732, 733 (МЛ) / 733 or 753, 754 (НЦ-132) / 753, 754 (ХВ-124) | МЛ-12, МЛ-152, НЦ-132, ХВ-124 | laz 732 #444b21, 753 #787734, 754 #717135 | b | ОСТ 36.13-90 lists защитный as a panel variant (S9) | S4, S5, S7, S8, S9, S13 |
| Салатный, светло-зелёный | lettuce green, light green | — | not found in any GOST fetched | — | c | — | — |

### 2.3 Safety colours (ГОСТ 12.4.026-76)

The Kartoteka samples that meet the requirement are listed in Table 1 (S11):

- red: 6, 7, 9, 10, 11, 19, 37, 43, 62.
- yellow: 216, 218, 220, 221, 254, 255, 285, 286, 287.
- blue: 408, 409, 423, 424, 449, 450, 474, 485, 486.
- green: 324, 325, 329, 385.

The chromaticity regions are class a, from Appendix 1 Table 1. The corner
points are listed below with decimal commas changed to points (S11b). The
page is mojibake-garbled, and the illuminant and observer were not legible.
The note cites ГОСТ 7721-76 and 45°/0° geometry.

| Signal colour | Corner 1 | Corner 2 | Corner 3 | Corner 4 | Luminance factor |
|---|---|---|---|---|---|
| Red | .569/.341 | .655/.345 | .690/.310 | .595/.315 | ≥ 0.07 |
| Yellow | .427/.483 | .465/.534 | .519/.480 | .468/.442 | ≥ 0.45 |
| Green | .007/.703 | .230/.754 | .291/.438 | .248/.409 | ≥ 0.12 |
| Blue | .078/.171 | .198/.252 | .240/.210 | .137/.038 | ≥ 0.05 |
| White | .290/.320 | .340/.370 | .350/.360 | .300/.310 | ≥ 0.75 |
| Black | .260/.310 | .345/.395 | .385/.355 | .300/.270 | as transcribed: "not less than 0.003" |

The black luminance-factor bound looks wrong: a black should take a maximum,
not a minimum. Verify it against a clean scan.

ГОСТ Р 12.4.026-2001 Table В.1 (S12b) lists standard samples whose
colorimetry meets the signal requirements. These are **not** equivalence
claims between the systems:

| Signal colour | Kartoteka | RAL | Munsell | Pantone |
|---|---|---|---|---|
| Red | 11* | RAL 3020 | 7.5R 4/14 | Warm Red C |
| Yellow | 220 | RAL 1023* | 5Y 8.5/14 | 109 C |
| Green | 385 | RAL 6024* | 5G 4/8 | 3415 C |
| Blue | — | RAL 5005 | 2.5PB 3/10 | 301 C |
| White | — | RAL 9003 | N 9.5 | — |
| Black | 800 | RAL 9004 | N 1 | — |

The asterisk marks samples inside the preferred small colour regions. These
pairs are the only official Kartoteka-to-RAL co-listings found.

## 3. The card index and its conversion tables

### 3.1 Identity and editions

- Title: «Картотека образцов (эталонов) цвета лакокрасочных материалов».
  Soviet GOSTs cite it simply as «Картотека». ГОСТ 9825-73 names it in its
  rule on colour designations (S15).
- Specifications: ТУ 6-10-1449-85, replaced by ТУ 6-10-1449-92 (in force
  01.12.1992) (S16). Later editions are ТУ 6-27-298-2002 (standard samples)
  and ТУ 6-27-91-2002 (working and demonstration samples). The sources
  disagree on which replaced which.
- Current maker: ЗАО «НПК ЯрЛИ» (Yaroslavl). The set is registered as a
  measuring instrument (Госреестр СИ) (S17).
- A 2008 article, «Лакокрасочные материалы и их применение» No. 6, p. 35,
  says (S18):
  - the current card has 240 colours, against 412 used earlier;
  - it comes in glossy and semi-matte versions;
  - it is now printed on paper rather than film;
  - reproduction keeps the colour of the old standards, using a control card
    to ТУ 6-10-1449-92.
- Pre-1985 editions, including the 1950s–70s cards that the 1974–82 GOSTs
  reference, were not identified by year in any source found.

### 3.2 Pair semantics and numbering

- ГОСТ 12.4.026-76 Appendix 2: the colour norm is set by two Kartoteka
  samples (S11c). The two numbers in every GOST table are therefore tolerance
  bounds of one colour.
- ГОСТ 9825-73: a name may be suffixed with the first number of its pair
  (S15).
- The numbering is sparse, and not every integer is a swatch. The
  laz-legend.ru admin could not explain the gaps (S13).

### 3.3 Published colorimetry

- **ГОСТ 22133-86, Appendix 2, Table 7** (S1): x, y and reflectance % for 12
  numbered samples (224, 661, 619, 319, 365, 397, 858, 840, 810, 832, 270,
  121). The measurement is in the XYZ system per ГОСТ 13088, illuminant C per
  ГОСТ 7721, specular component included, on «Радуга-2». This is the only
  published per-swatch colorimetry found for decorative (non-signal) Kartoteka
  samples.
- An excerpt in «Справочник химика 21» (chem21.info, cited via search; the
  page itself could not be fetched due to a TLS host mismatch) states that
  **older cards printed XYZ and x, y on the reverse**, measured on «Радуга-2»
  under illuminant C. A physical card of that era would therefore carry
  class-a values. None were found transcribed online.
- ГОСТ 12.4.026-76 Appendix 1: chromaticity regions for signal colours
  (§2.3).

### 3.4 Unofficial conversion: laz-legend.ru RGB list (class b)

laz-legend.ru, a LAZ bus history site, publishes about 260 Kartoteka numbers
with hex RGB "from the collection of the card index that reached us" (S13).
The method is unstated, so it is probably a scan. I converted these hex
values back (sRGB→XYZ D65→Bradford→C) and compared them with ГОСТ 22133-86:

| Card | GOST x, y, Y | laz-derived x, y, Y |
|---|---|---|
| 319 | .311, .339, 52.4 | .304, .340, 56.0 |
| 365 | .300, .333, 27.5 | .288, .335, 26.2 |
| 858 | .319, .333, 48.8 | .311, .335, 50.9 |
| 661 | .352, .351, 34.2 | .354, .369, 34.2 |
| 840 | .317, .324, 28.8 | .302, .323, 25.1 |
| 619 | .357, .339, 10.0 | .395, .388, 6.4 |
| 270 | .374, .376, 61.5 | .375, .390, 70.6 |

Light colours agree within roughly 0.01 in x and y. The laz values run
cooler/greener on greys and are badly off on the dark brown. The laz values
are usable as an ordering and hue guide for swatches that GOST does not
measure. They are not reference values.

### 3.5 RAL equivalence

- No official Kartoteka→RAL table exists for the general palette. A
  chipmaker.ru thread (2016; a restorer of 16К20 and 676 machine tools)
  confirms that tinting shops cannot match Kartoteka numbers and must go via
  coordinates or a spectrophotometer (S10). One poster chose Tikkurila
  Temacoat in RAL 6027, explicitly "a little different from the Soviet colour".
  That is an aesthetic substitute, not an equivalence.
- Vendor RAL tags on current ПФ-115 tins (e.g. "синяя RAL 5005", "серая
  RAL 7040") are modern product choices, not equivalences to the GOST card
  numbers. They are not used here.

## 4. Conflicts noted (not resolved)

1. **Тёмно-серый, ГОСТ 22133-86**: Table 3 gives pair 822, 847. Table 7 gives
   832. ГОСТ 25515-82 (НЦ-256) gives 832, 847, which supports 832 (S1, S2).
2. **Серо-голубой**: ПФ-115, МЛ-12 and МЛ-152 use 491, 492. НЦ-132 uses 463,
   464. The same name covers two different swatches (S3, S5, S7, S8).
3. **«Морская волна»**: ПФ-133 and МЛ-12 use 330, 387. The МЛ-152 transcription
   aligns 377, 378. МЛ-152 is OCR-garbled, but the meganorm and dnaop readings
   agree (S6, S7, S8).
4. **Фисташковый**: ПФ-115, ПФ-133 and НЦ-132 use 388, 389. МЛ-12 uses
   381, 382. НЦ-132 also has "фисташковый 309" (309, 328).
5. **Сине-зелёный**: the machine-tool сине-зелёный is 397, 398 (ГОСТ 22133,
   НЦ-256). In МЛ-12 and МЛ-152 it is 375, 376.
6. **Слоновая кость**: 224, 229 (НЦ-256 / ГОСТ 22133). In МЛ-12 it is 251,
   252, and a vendor lists 251, 252 too. МЛ-152 shows 252, 253 with uncertain
   alignment.
7. **Защитный, НЦ-132**: 733, 754 vs 753, 754 between grade columns; probably
   an OCR error (S5).
8. **ХВ-16** is cited as ТУ 6-10-1301 in ОСТ 36.13-90 Table 33 and as
   ТУ 16-10-1301 in Table 34 (S9).
9. **laz-legend RGB vs ГОСТ 22133 colorimetry**: see §3.4.

All GOST texts were read from web transcriptions (meganorm.ru, dnaop.com) that
contain OCR damage. The НЦ-256 and МЛ-152 pairs were aligned by order and need
checking against scans.

## 5. Equipment use: documented vs plausible

**Documented:**

- ГОСТ 22133-86 main colours, including pale mint 319, grey-green 365, teal
  397 and white-night 858, are for exterior surfaces of metal-cutting,
  forging, foundry and woodworking equipment, 1988 onward (S1).
- ГОСТ 25515-82: the НЦ-256 enamel in the same 11 colours is "for painting
  metal-cutting machine tools" (S2).
- Cream 270 is specified for the interiors of electrical cabinets and niches
  on machine tools (S1).
- ОСТ 36.13-90 (replaced ОСТ 36.13-76) covers process-automation switchboards
  and consoles (S9):
  - temperate versions: grey or light grey from ПФ-115, ПФ-218ГС, НЦ-132П or
    ХВ-124, with dark-grey, black or protective frames, or dark-brown/beige
    variants;
  - tropical versions: **серо-голубой** panels with dark-grey frames (ХВ-16).
  - It gives no card numbers.
- Hammer enamels МЛ-165 (серо-голубой, голубовато-серый, серо-зелёный and
  others) are for "instruments and other metal articles" (S12), and are
  recommended with the machine-tool colours (S1).

**Restorer claim (b):** the 16К20 lathe and 676 milling machine were
factory-painted светлый серо-зелёный 352 or серо-зелёный 365 (chipmaker.ru
poster, S10).

**Plausible but undocumented:**

- power-station control-room panels in grey-blue or grey-green;
- 1950s–60s machine tools in the same palette, since ГОСТ 22133-76 was not
  found;
- turquoise and sea-wave on industrial equipment. The only documented
  sea-wave use is ПФ-133 on freight wagons and containers.

## 6. Conversion method (for every "conv." hex)

Input: ГОСТ 22133-86 Table 7 x, y and reflectance R %, with Y = R/100.
Illuminant C white is x .31006, y .31616 (CIE 1931 2°; the GOST does not
state the observer, and 2° is assumed per ГОСТ 13088-era practice).

The steps:

1. Compute XYZ.
2. Apply Bradford chromatic adaptation from C to D65 (white x .31271,
   y .32902).
3. Apply the IEC 61966-2-1 sRGB matrix and transfer curve.
4. Compute CIELAB with the D65 white.

No value was out of gamut. Two caveats apply:

- The GOST values include the specular component, so the visual appearance of
  a glossy enamel (specular excluded) would be slightly darker and more
  saturated than the hex values.
- Each value is the first swatch of the tolerance pair, not the tolerance
  centre.

The script was run from scratch and is not kept in the repository. It is
reproducible from the formulas above.

## 7. Sources

- **S1** ГОСТ 22133-86 «Покрытия лакокрасочные металлорежущего,
  кузнечно-прессового, литейного и деревообрабатывающего оборудования. Общие
  требования» (введён 01.01.1988, взамен ГОСТ 22133-76).
  https://meganorm.ru/Data2/1/4294831/4294831647.htm
  - Table 3: names and pairs.
  - Appendix 2, Table 7: x, y and reflectance, Радуга-2, illuminant C.
  - Appendix 1: enamel list.
  - Appendix 4, Table 11: colour pairings.
  - Appendix 5: hammer enamels.
  - §9.1: surface use.
- **S2** ГОСТ 25515-82 «Эмали НЦ-256», for metal-cutting machine tools: 11
  colours, card pairs 224/229 … 858/859 (order-aligned).
  https://dnaop.com/html/76281/doc-%D0%93%D0%9E%D0%A1%D0%A2_25515-82
- **S3** ГОСТ 6465-76 «Эмали ПФ-115», Table 1: colours and card pairs (e.g.
  серо-голубой 491, 492; фисташковый 388, 389). Clause 1.4 sets colour
  against the «Картотека».
  https://dnaop.com/html/42611/doc-%D0%93%D0%9E%D0%A1%D0%A2_6465-76
- **S4** ГОСТ 10144-89 «Эмали ХВ-124»: защитный 753, 754; серый 881, 882.
  https://meganorm.ru/Data1/16/16220/index.htm
- **S5** ГОСТ 6631-74 «Эмали НЦ-132»: 22 colours with pairs.
  https://dnaop.com/html/68523/doc-%D0%93%D0%9E%D0%A1%D0%A2_6631-74
- **S6** ГОСТ 926-82 «Эмаль ПФ-133»: «морская волна» 330, 387; used for
  freight rolling stock and containers.
  https://meganorm.ru/Data2/1/4294845/4294845525.htm
- **S7** ГОСТ 18099-78 «Эмали МЛ-152»:
  https://meganorm.ru/Data1/21/21733/index.htm and
  https://dnaop.com/html/73031/doc-%D0%93%D0%9E%D0%A1%D0%A2_18099-78
- **S8** ГОСТ 9754-76 «Эмали МЛ-12», Table 1: 35 colours with pairs.
  https://dnaop.com/html/41853/doc-%D0%93%D0%9E%D0%A1%D0%A2_9754-76
- **S9** ОСТ 36.13-90 «Щиты и пульты систем автоматизации технологических
  процессов. ОТУ» (взамен ОСТ 36.13-76), Tables 33 and 34.
  https://meganorm.ru/Data2/1/4293836/4293836649.htm
- **S10** chipmaker.ru, «Конвертация цвета из картотеки ЯРЛИ в картотеку RAL»
  (2016): a restorer's account of 16К20/676 colours, citing ГОСТ 22133-86
  coordinates. https://www.chipmaker.ru/topic/179006/
- **S11** ГОСТ 12.4.026-76 «Цвета сигнальные и знаки безопасности».
  - Table 1 (card numbers):
    https://dnaop.com/html/42591/doc-%D0%93%D0%9E%D0%A1%D0%A2_12.4.026-76_
  - **S11b** Appendix 1 (chromaticity regions):
    https://base.garant.ru/5369561/53f89421bbdaf741eb2d1ecc4ddb4c33/
  - **S11c** Appendix 2 ("two samples" rule; enamel brands):
    https://zakonbase.ru/content/part/458742?print=1
- **S12** ГОСТ 12034-77 «Эмали МЛ-165, МЛ-165ПМ и МС-160»: hammer enamels,
  colours by control sample, for instruments and metal articles.
  https://dnaop.com/html/71373/doc-%D0%93%D0%9E%D0%A1%D0%A2_12034-77
- **S12b** ГОСТ Р 12.4.026-2001, Table В.1.
  https://www.consultant.ru/document/cons_doc_LAW_136313/6096b8c0b3585db9ec962c6c5c37e9af6a6c4890/
- **S13** laz-legend.ru, «Картотека образцов цвета лакокрасочных
  материалов»: hex RGB per card number, method unstated (class b).
  http://laz-legend.ru/kartoteka-obrazcov-cveta-lakokrasochnyh-materialov.html
- **S14** Vendor listings using Kartoteka numbers:
  - https://yar-tex.com/catalog/146/118088/ (МЛ-12 серо-голубая эт. 491, 492,
    and other МЛ-12 pairs);
  - https://krasku.ru/emal-ml-12-svetlo-sero-golubaya-et.-495-496
  - Neither gives a RAL or numeric equivalent.
- **S15** ГОСТ 9825-73, colour designation rule (via search excerpt; dnaop.com
  ГОСТ pages).
- **S16** ТУ 6-10-1449-92 catalogue entry:
  http://www.1bm.ru/techdocs/kgs/tu/932/info/167705/
- **S17** НПК ЯрЛИ: https://www.yarli.ru/about/wip/
- **S18** «Российская Картотека образцов цвета ЛКМ», ЛКМ и их применение,
  2008 No. 6: https://rucont.ru/efd/409126
- Searched without a documented result:
  - Chernobyl/ChNPP БЩУ paint (chnpp.fandom.com returned 402; English
    sources are photo impressions only);
  - Minenergo or ОСТ 108 control-room colour rules;
  - KRU/KSO cabinet colour (ГОСТ 14693-90 excerpt has none);
  - ГОСТ 23000-78 console colours (body text not retrievable);
  - ЭП-140 (ГОСТ 24709-81) card numbers.
