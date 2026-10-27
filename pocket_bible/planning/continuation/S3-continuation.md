# Stage 3 continuation note (written when the session's usage ran out)

Branch work/s3-container, not pushed. Read the git log first.

Done and committed: items 1-3 of the handoff; the container encoder
(src/corpus/container_*.cpp), the reader (src/core/container_reader.cpp),
tests container_coder_tests and container_roundtrip_tests, the study commands
(bible_build study-text | candidates | dump-blocks DIR | time-container FILE).

Results on disk:
- experiments/001-fallback-originals/ (complete, with README)
- experiments/002-text-study/results.tsv (complete; README not written)
- experiments/003-container-candidates/timing.tsv (complete)
- experiments/003-container-candidates/candidates.tsv (a background run was
  writing it when the session stopped; rerun `bible_build candidates` if it is
  short of 7 variant rows plus the computed rows)
- .build/s3/controls/whole and /blocks: every work whole and every block
  alone (7,294 files), ready for the controls. NOT yet run:
    zstd --ultra -22 on whole/*.txt and on blocks (-r --output-dir-mirror)
    zstd --train on blocks, then -D dict on blocks (count the dictionary)
    xz -9e on whole and on blocks (--files=list, -k)
  Sum compressed sizes per work and in all.

Not done: the controls; READMEs for experiments 002 and 003 (candidate
table with losers); decisions/ADR-001 (proposed, not accepted);
planning/reports/S3-<date>.md with the owner questions; dated notes at the
top of planning/README.md, STAGE_PLAN.md, SOURCE_REGISTER.md and
sources/IMPORTERS.md; the house-style review; the push.
Also not done: the private release sources-2026-10-03 (refused by the
permission layer; see sources/README.md).

Later: candidates.tsv finished (51 lines, all seven variants round-trip) and
is committed. Two results to carry into the candidate table and the ADR:
- Hebrew as whole pointed words is 403,139 bytes SMALLER than the three
  layers of K011 A (38,246,101 against 38,649,240). The GIVEN stands; the
  cost must be reported to the owner as a question.
- The computed Huffman size is slightly below the built rANS size
  (33,345,455 against 33,408,272 symbol bytes): the integer frequency
  scaling in build_model loses more than Huffman's whole-bit rounding.
  Worth one experiment before the ADR.
