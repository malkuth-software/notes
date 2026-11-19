# Brief: `stx_filesign`, self-addressed signed files in STXlib

Date: 2026-11-19. Written by the coordinator for the owner and for the
STXlib workers who will build it. Its home is STXlib's desk
(`projects/desks/stxlib/`); it is kept here until that checkout is free.
Status: the owner's rulings below are decided; the rest is a plan.

Source: Astra's (Codex) specification "STX Release Seal v1: Circular CMS
Endorsement with Ratcheted LMS Signing", on STXlib `main` at
`ff919162626f8358d28a0d61c25f43ed14613cb1`, `docs/release-seal/`: the TeX
source, the PDF whitepaper and the machine-readable vectors. Astra wrote it
as pure mathematics, without the house style, as with the `pow` and `sinc`
work. The companion note is "Circular Endorsement of Self-Hashing Files"
in the papers repository.

## 1. What it is, in plain words

Today a signed file carries a signature over a hash of the file *without*
the signature, and the signature sits in a special region the hash skips
(Authenticode's certificate table, Apple's code-signature blob).

A sealed STX file is different. Its last 64 bytes, the trailer `t`, are
the file's own address: the STX hash of the **entire** file, signature and
certificates included, equals `t`. The publisher signs those 64 bytes. So
one short number both names the file exactly and is what the publisher
vouched for. Making a file hash to a value written inside itself used to
need brute force; this construction gets it in one pass, with no search,
because the hash is built around the signature equation:
- the hash folds in everything except the signature's secret-dependent
  part, takes the value `a` that a correct file must have, and signs it;
- for a correct file, the signature check and the hash's closing condition
  are the same equation, so the hash of the finished file comes out as `a`;
- any other file cannot be a fixed point, and two different files with the
  same address would give a collision in SHA3-512 or SHA-256.

Two signature suites:
- **Ed25519**, classical, small (a 547-byte CMS in the fixture);
- **LMS** (RFC 8554 hash-based signatures, type 6 with LM-OTS type 3),
  resistant to quantum computers, 2,512-byte signatures, 1,024 signatures a
  key, with a private-key ratchet so a stolen signer cannot sign in the past.

Two carriers:
- an **envelope**: one file holding an optional front record, the payload
  byte for byte, and the end record;
- a **sidecar**: the records in a companion `.stxsig` file beside an
  unchanged native file, so an already signed `.exe` or `.app` still passes
  Windows and macOS checks. Native signing, timestamping, notarisation and
  stapling come first; STX sealing comes last.

The signature is ordinary CMS (RFC 5652) with X.509 certificates, so
standard tools can check it: OpenJDK verified both published fixtures, and
OpenSSL the Ed25519 one.

## 2. Owner rulings, 2026-11-19

1. The STXlib library is named **`stx_filesign`** (owner's suggestion).
2. Astra's work is ported to the house style. It is linted like all
   first-party code.
3. Its documentation includes a very careful exploration of the hashing and
   signing security: **nothing up our sleeves**.
4. The result is a persistent facility, used with a new set of small shim
   programs that perform signing-equivalent tasks.
5. We dogfood it by signing our own files.
6. A set of demonstration mock-up dialogs shows how it would look in
   practice, compared with signed Windows and Macintosh files.
7. The documentation wiki presents it as the future of file signing; it
   also makes finding files easier.
8. Released files come with a small metadata stub: **name, version, public
   key, self-hash**.
9. The catalogue model: a user downloads a catalogue, pins a specific
   public certificate, retrieves all the metadata, selects the version they
   want, then asks the server for the file with that self-hash.

## 3. What STXlib has and lacks

Has (STXlib `main`):
- `stxlib::precedence` (from stx_auxlib b6e330a): BLAKE2s, WOTS+ with 67
  chains, blocks of 256 epochs. Its copyable `ChainState` and its
  save/restore allow an old snapshot back, which an LMS release key must
  never allow. It stays as it is, with its vectors; a precedence record can
  later list published addresses under a separate key;
- `stxlib::fill_entropy` and `random::entropy_seed`, which fail closed;
- BLAKE2s in `stxlib/hash.hpp`.

Lacks, all needed:
- SHA-256, SHA-512, SHA3-512, HMAC-SHA256 and HMAC-SHA512;
- Ed25519 (RFC 8032) with canonical-encoding checks and the two-phase
  nonce workflow (fresh `R` before the trailer, `s` after);
- HSS/LMS (RFC 8554) verify and sign, with the two-phase preparation (leaf,
  randomiser `C`, `K_q` and the ten siblings before the trailer);
- a DER reader and writer strict enough for the profile (set order, minimal
  lengths, no trailing objects);
- CMS SignedData in the profile's one shape;
- X.509 parsing and RFC 5280 path validation, with key usage, code-signing
  EKU, name constraints, validity, revocation and algorithm policy;
- the protected signing provider (section 5.4).

## 4. House-style points for the port

- **Failures as data.** STXlib's rules: no exceptions in release builds,
  no `std::` in public headers. Every operation returns a named result.
  Verification keeps distinct outcomes, as the specification requires:
  malformed carrier, address mismatch, invalid signature, untrusted path,
  expired, revoked, below the rollback floor, policy rejection.
- **Checked arithmetic** on every length and offset (house style 1); the
  carrier is parsed from the 88-byte tail at the end, never by scanning the
  payload for a marker.
- **Explicit records**: the descriptor, the tail, the CMS template and the
  public preparation are named records with stated field widths, not byte
  arithmetic spread through the code.
- **Secrets have one owner and an end.** Secret material lives in named
  storage that is erased on every exit path; nothing secret is copyable; the
  volatile signing handle is linear (finish once or cancel, then gone).
- **Constant time.** Every operation on secret data (scalar arithmetic,
  HMAC keys, chain starts) runs without secret-dependent branches or memory
  addresses. The specification does not say this; the port must, and the
  security document must show how it was checked.
- **State machines with a fallback.** The signer's consume, prepare, finish
  and cancel are one state machine whose default reaches one fallback: the
  leaf is burned and nothing is emitted.
- **Streaming.** Payloads can be larger than memory: readers and the raw hash
  stream; seekable output or a spool where the final signature sits near the
  front.
- **No third-party code.** Reference software (OpenSSL, OpenJDK, the RFC
  reference implementations) is a test oracle only, never linked.

## 5. Stages

Astra's milestones, in order, with the house-style port in each.

| Stage | Contents | Done when |
|---|---|---|
| F0 | The primitives: SHA-256, SHA-512, SHA3-512, HMAC; then Ed25519; then LMS verify | NIST CAVP vectors for the hashes; RFC 8032 and Wycheproof vectors for Ed25519; RFC 8554 test vectors for LMS; constant-time checks |
| F1 | The carrier reader and the raw hash (spec sections 1-3): the tail, the front-copy equality, envelopes and sidecars (two handles), the total raw hash with both probes, the correction and the malformed-input rules; the `M_0` templates (Ed25519: zero only `s`; LMS: zero only the 67 chain words) | **first acceptance**: from the JSON vectors, rebuild each object, match its length and ordinary SHA-256, then the full STX hash against `trailer`; also `anchor_input_sha256`, `empty_raw_hash`, the LMS `candidate_k`, and the two LMS mutations (a chain word changed in both CMS copies; only the front magic changed) |
| F2 | Publisher authorisation: detached CMS verification of a trailer, and full release verification on a carrier (address, signature, path to an independently configured anchor, usage, validity, revocation, publisher, product, channel, rollback floor, PQ-only policy) | a valid detached signature whose changed carrier fails full verification; an unauthorised root, an expired certificate, a classical issuer under PQ-only policy, a release below the floor, each with its own outcome |
| F3 | The signer, built against the verifier: fixed templates, two-phase Ed25519 and LMS, self-verification before publication; LMS enrolment with the HMAC ratchet exactly as specified (`public_test_ratchet` first), the 98,272-byte public cache checked against the root; separate tree identities for tests, releases and certification | emitted signatures verified by an independent verifier (OpenSSL for Ed25519, an independent HSS verifier for LMS) |
| F4 | One-time signing across crashes: the provider contract (enrol, public status, consume-and-prepare, finish once, cancel-and-burn); durable consumption and erasure before any signature; `unsupported` on a backend without nonexportable volatile secrets and rollback-resistant atomic replacement | the failure table of the specification exercised in a process-level simulation; hardware claims only on hardware (question 1) |
| F5 | The shim programs, the metadata stub and the catalogue (section 6) | each shim round-trips the fixtures; a catalogue fetch by self-hash verified end to end |
| F6 | Dogfood and demonstrations (section 7) | our own release files sealed and verified; the acceptance matrix across both suites, both carriers and both front settings; a natively signed PE on Windows and thin, universal and bundled deliverables on macOS verified before and after the round trip |

The security document (section 8) is written alongside F0-F4, not after.

## 6. The persistent facility, the shims and the catalogue

**The facility** is `stx_filesign` itself plus a small key service: the
provider of F4, its public tree caches and its journal of public receipts.

**The shims** are small programs over it, one task each (names are
proposals):

| Shim | Task |
|---|---|
| `stx-seal` | seal a payload as an envelope or a sidecar |
| `stx-verify` | verify a file or a native file with its sidecar against a policy |
| `stx-address` | print a file's address (raw hash), without trust checks |
| `stx-extract` | take the native payload out of a verified envelope |
| `stx-keys` | enrol a key, show its public state and remaining leaves, burn |
| `stx-stub` | write a file's metadata stub |
| `stx-fetch` | fetch from a catalogue by self-hash and verify |

**The metadata stub** (owner): name, version, public key, self-hash. As a
proposal: the stub is a small text record (the key as its certificate
fingerprint and the self-hash as the 64-byte trailer in hex), and the stub
file is itself sealed, so the stub too is self-addressed.

**The catalogue** (owner): a sealed list of stubs. A client pins a
publisher certificate, downloads the catalogue, verifies it, picks a version,
and asks the server for the file with that self-hash; what arrives must hash
to the self-hash and verify under the pinned key. Because the address is the
file's exact identity, any server or mirror can serve it and no file name
or URL has to be trusted: this is the "finding files easier" part.

## 7. Dogfood and demonstrations

- **Sign our own files:** the SDK, Plan Paint, PlaySuite and the other
  products' release files get a `.stxsig` sidecar after native signing, and
  our install scripts verify it before use. Development keys and test trees
  are separate from release keys.
- **Mock-up dialogs:** GUI.Forms dialogs showing what a user would see:
  verified publisher, product, version, channel and address; a changed file;
  an unknown publisher; a revoked certificate; a rollback. Shown beside
  descriptions of what Windows (SmartScreen and the publisher prompt) and
  macOS (Gatekeeper) show for the same cases. Our dialogs are drawn in our
  own design language; the comparison describes the platform dialogs rather
  than copying their branding.
- **The wiki page** presents it as the future of file signing, with claims
  held to what the security document shows.

## 8. The security document: nothing up our sleeves

It explains, for a careful reader:
- **every constant and where it comes from**: the SHA-2 constants (from
  square and cube roots of primes), the SHA-3 round constants (from a linear
  feedback shift register), Ed25519's curve and base point and why they were
  chosen, the LMS type numbers, and our own domain strings (`STXSEAL1`,
  `STX-RATCHET-1`, `STX-LMOTS-1`), which are plain ASCII with no tuning;
- **what each primitive is trusted for**, and what breaks if it falls:
  SHA3-512 and SHA-256 collisions break the file binding; SHA-256 one-wayness
  of chain words protects past LMS leaves; HMAC as a pseudorandom function
  protects the ratchet;
- **the circular construction** and its two theorems (fixed point and
  physical binding; single use and persistence), in plain words beside the
  proofs;
- **the honest limits**, as the specification states them:
  - the LMS suite's file binding inherits SHA-256's collision bound; a
    64-byte trailer does not raise it;
  - no concrete quantum security level is claimed for SHA-256;
  - LMS signatures reveal their leaf index, so observers can see order and
    gaps;
  - the ratcheted private-key derivation is allowed by RFC 8554 but is not
    the single-seed method NIST SP 800-208 prescribes, so it claims no SP
    800-208 conformance;
  - stealing the current signer state lets the thief sign *future* leaves;
    revocation, independent time evidence and version floors handle that;
  - Ed25519 is classical only;
  - the protected provider's guarantees hold only on hardware qualified for
    them;
- **side channels**: how constant-time behaviour was achieved and checked;
- **what was verified independently** (OpenJDK, OpenSSL) and what was not
  (Windows admission, notarisation, enclave rollback resistance).

## 9. Placement

It is STXlib work and runs beside the GUI.Forms push to 1.0, through
STXlib's coordinator and integrator. It does not gate GUI.Forms 1.0.
GUI.Forms may later use it (Plan Paint verifying what it downloads, the
products' installers), through the stx pin.

## 10. Open questions

1. **Hardware for the protected LMS signer.** The specification needs an
   isolated module with rollback-resistant atomic state and nonexportable
   volatile secrets; Apple's public Secure Enclave API does not run this LMS
   transition. Which device do we qualify (an HSM, a TPM-backed signer, a
   dedicated offline machine)? Until then: Ed25519 and LMS test trees only,
   and the provider answers `unsupported` for production LMS.
2. **The catalogue's format and hosting**: a sealed text list on
   malkuthsoft.com, or something else?
3. **The shim names** in section 6.
