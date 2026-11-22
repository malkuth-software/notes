# Study: building for Apple without a Mac

Date: 2026-11-22. Status: study for roadmap step 8 ("Everything else"),
building on the week plan's section 3.3, "Building for Apple without Apple"
(`planning/WEEK_2026-11-23_PUSH_TO_1.0.md`). This study changes no code and
builds nothing. It quotes Apple's licence terms and sets them against each
route, but **it draws no legal conclusion**. As section 3.3 says, code that
compiles against or derives from Apple's SDK off Apple hardware waits for the
EFF's answer to the owner. Signing and notarisation off a Mac (section 7)
raise no SDK question and could go ahead before that answer.

The repositories were read at gui_forms `875590404` (2026-11-21), STXlib's
`claude/ic-full` branch, and the M4 scripts in `~/Developer/work/bin`. Web
facts were checked on 2026-11-22 and are cited inline and in section 12.

## 1. The question, and the short answer

How could STXlib, GUI.Forms and the programs be built, signed and packaged
for macOS (arm64, and possibly x86_64) from Linux or Windows, including in CI
without macOS runners?

The process has four steps. Each one depends on Apple to a different degree:

| Step | Tools that run off a Mac | What still ties it to Apple |
|---|---|---|
| Compile | clang 22 with `-target arm64-apple-macos14` | the **SDK headers** for the AppKit host (licence) |
| Link | `ld64.lld` from the same LLVM | the SDK's **.tbd stubs** (licence) |
| Package (.app, zip, pkg, dmg) | plistlib, `zip`, xar with bomutils, libdmg-hfsplus | none (formats only) |
| Sign and notarise | rcodesign (apple-codesign) and Apple's Notary REST API | a **Developer ID certificate** and an **App Store Connect API key** (owner, paid membership) |
| Run and test | nothing that runs AppKit | **Apple hardware** (the M4, or a hosted Mac) |

The signing and packaging steps can leave the Mac today with no licence
question. The compile and link steps can leave it technically, as Mozilla and
Chromium have shown, but only with an SDK. Apple's SDK comes under an
agreement whose text is quoted in section 3. The alternative is a clean-room
header and stub set, which nobody has published for AppKit. Running and
testing the AppKit host stays on Apple hardware whichever route is taken.

One fact changes the cost side. gui_forms and stxlib are **public**
repositories (`gh repo view`, 2026-11-22). GitHub's standard macOS runners
cost nothing for public repositories, and CI already uses them
(section 2.3). "CI without macOS runners" therefore saves no money now. Its
value is independence from GitHub and Apple, plus one toolchain on every
host.

## 2. How macOS is built today

### 2.1 GUI.Forms

`CMakeLists.txt`, on a Darwin host:
- Sets `CMAKE_OSX_DEPLOYMENT_TARGET 14.0`.
- Fixes `CMAKE_OSX_ARCHITECTURES` at `arm64`. Any other value stops the
  configure with `FATAL_ERROR "GUI.Forms supports Apple silicon arm64 only"`.
- Requires LLVM 22.1.x (`cmake/llvm22.cmake`).
- Turns on `-Werror=unguarded-availability`.
- Links Release modules with `-dead_strip`, and adds
  `-no_warn_duplicate_libraries`, a flag of Apple's ld-prime.
- Turns LTO off on macOS. The comment at line 222 gives the reason: the macOS
  linker "reads bitcode through Xcode's libLTO, not LLVM 22's".

`cmake/llvm22.cmake` gets the SDK from `xcrun --show-sdk-path`, which sets
`CMAKE_OSX_SYSROOT`. It links libc++, libc++abi and libunwind statically from
`GUI_FORMS_LLVM_RUNTIME`. That is the LLVM 22.1.8 runtime for macOS 14, built
by template_harness `harness/build_macos_runtimes.py`. **The C++ runtime is
therefore already ours. Apple supplies only the system libraries and the
frameworks.**

The macOS host, `gui_forms_host_macos`, has 22 Objective-C++ files under
`src/surface/macos` (about 11,200 lines including headers). It refers to 359
distinct Apple identifiers with the prefixes `NS`, `CG`, `CT`, `CA`,
`IOSurface`, `UT` and `CF`. Linked frameworks:
- `cmake/ApplicationPackage.cmake:90`: AppKit, QuartzCore, IOSurface,
  CoreGraphics, CoreText, UniformTypeIdentifiers and CoreFoundation.
- `cmake/Audio.cmake:86`: CoreAudio, AudioToolbox and CoreFoundation.

The sources also import CoreVideo (`services/appkit_host_services.mm`) and
CoreServices. `check_renderer_boundaries` forbids Metal, MetalKit, OpenGL and
WebKit, so the surface stays small: one window system, 2D drawing, text,
shared surfaces and audio.

### 2.2 STXlib

`APPLE` selects `STXLIB_HOST_FAMILY macos`. The only Apple header in STXlib
is `<mach/...>`, and it links no framework. **STXlib needs only libSystem.**
That is the easiest part of any off-Mac build.

### 2.3 Packaging, signing and CI

`tools/package_programs.py` builds the .app and copies in its dylib closure
using `otool` and `install_name_tool`. It edits `Info.plist`, then runs:
1. `codesign --force --sign -` on each Mach-O, then `--deep` on the app;
2. `codesign --verify --deep --strict`;
3. `ditto -c -k --sequesterRsrc --keepParent` for the zip;
4. `pkgbuild` for the installer.

`required_tools("macos")` is `otool, install_name_tool, codesign, ditto,
pkgbuild`, and every one of them is an Apple tool.
`tools/publish_tested_release.py` says: "macOS bundles are ad hoc signed
(`codesign --sign -`), not Developer ID signed or notarized". Since macOS 15
the Control-click way past Gatekeeper is gone. Users must go to System
Settings, Privacy & Security, Open Anyway, for each app [MacRumors 2024].
Today's downloads therefore meet the strictest first-launch path there is.

CI runs `.github/workflows/release.yml` (platform `macos-arm64`) and
`native.yml` (native and programs jobs) on GitHub's `macos-15` runners. Local
work goes to the M4 through `m4ws`, `m4run` and `m4sb`, which rsync the
workspace to `m4mini` and run with `env.sh` (LLVM 22, ccache).

## 3. What Apple's licences say

The quotes below come from the Xcode and Apple SDKs Agreement, version
"EA2002 06/08/2026", fetched from apple.com on 2026-11-22 [Apple SLA]. They
are quoted exactly, with omissions marked by "…". Interpretation is for the
EFF.

**Header (all capitals in the original):**
> "USE OF APPLE SOFTWARE IS GOVERNED BY THIS AGREEMENT AND IS AUTHORIZED ONLY
> FOR EXECUTION ON AN APPLE-BRANDED PRODUCT RUNNING MACOS. ANY OTHER DOWNLOAD
> OR USE OF APPLE SOFTWARE … BREACH OF THIS AGREEMENT."

**Definition:** the "macOS SDK" means Apple-proprietary SDKs "including but
not limited to header files, APIs, libraries … labeled as part of the macOS
SDK".

**§2.2.A, the grant:**
> "Install a reasonable number of copies of the Apple Software on
> Apple-branded computers that are owned or controlled by You to be used
> internally by You or Your Authorized Developers only as follows: (i) …
> Xcode Developer Tools to test and develop application and other software;
> (ii) You may use the macOS SDKs to test and develop application and other
> software; …"

**§2.5, copies:**
> "You may copy only the entire package … You are expressly prohibited from
> separately using the Apple SDKs or attempting to run any part of the Apple
> Software on non-Apple-branded hardware."

**§2.7, restrictions:**
> "… do not permit You to … install, use or run the Apple Software or Apple
> Services on any non-Apple-branded computer or device, or to enable others to
> do so. This Agreement does not allow the Apple Software … to be made
> available over a network where they could be run or used by multiple
> computers at the same time … you agree not to rent, lease, lend, upload to
> or host on any website or server, sell, redistribute, or sublicense …"

§2.7 also forbids decompiling and reverse engineering "except as and only to
the extent any foregoing restriction is prohibited by applicable law, or …
open-sourced components".

**The macOS licence itself** allows up to two extra copies of macOS in
virtual machines. Older texts tie them to "each Mac Computer you own or
control that is already running the Apple Software" [Apple macOS SLA, older
texts]. A forum post quotes a newer clause that adds "except as otherwise
provided in writing, signed, or issued by an authorized representative of
Apple" [Apple forums 845455]. This study did not fetch the current macOS 26
licence. Whatever its exact wording, a macOS virtual machine on non-Apple
hardware is not one of the paths this study proposes.

**Questions for the EFF.** Each item names the route it decides (section 4).
1. Route B: does compiling or linking on Linux or Windows against headers and
   .tbd stubs copied from Xcode's SDK fall under "install, use or run … on any
   non-Apple-branded computer"? The SDK is data that the compiler reads; it is
   not code that runs.
2. Route B: does keeping that SDK in a private CI image or cache count as
   "upload to or host on any … server"?
3. Route C: may a clean-room header set be written from Apple's public
   documentation? The set would be names, signatures, constants and struct
   layouts written by us, with no SDK file copied. The precedent is
   section 3.3's Windows route (headers generated from openly licensed
   metadata).
4. Route C, in two variants:
   - May the clean-room set be checked against the real SDK on the M4? Here
     "checked" means compile probes such as `static_assert` on sizes and
     offsets, with nothing copied.
   - May the .tbd symbol lists be produced on the M4 by reading Apple's
     shipped stubs, which §2.7's reverse-engineering clause may touch?
5. Routes B and C: does Apple's open-source exception ("open-sourced
   components") cover the parts Apple publishes under APSL or similar
   licences (libc headers, dyld and cctools headers), so that those parts may
   be used freely?

## 4. The three routes for compiling and linking

### Route A: compile on Apple hardware, everything after it off the Mac

Today's build stays as it is: the M4 locally, and GitHub `macos-15` in CI.
Packaging, signing and notarisation move to Linux (sections 6 and 7). This
raises no SDK question. It removes Apple's `codesign`, `ditto` and `pkgbuild`
from the release path, and it is the route that gives Developer ID
notarised releases soonest.

### Route B: Apple's SDK on Linux or Windows

This is the osxcross route. `gen_sdk_package_pbzx.sh` unpacks the SDK from
an Xcode `.xip` on Linux. It needs up to 45 GB of free disk space. A sister
script packages the SDK on a Mac from the Command Line Tools
[osxcross README.SDK]. Mozilla and Chromium both build macOS releases off
Apple's linker (section 5). Technically the route is proven. **The open
question is the licence (section 3, questions 1 and 2).** This study does not
recommend it until the EFF answers.

### Route C: a clean-room SDK subset

This is the analogue of the Windows no-MinGW precedent that section 3.3
names. The build would need four things:
1. **Headers for the subset in use.** That is the 359 identifiers above plus
   their transitive types. Most are Objective-C class and protocol
   interfaces, where only the methods called need declaring. The rest are C
   functions (CoreGraphics, CoreText, IOSurface, CoreFoundation, CoreAudio
   and AudioToolbox), constants and enum values, and a few struct layouts
   (`CGRect`, `CGAffineTransform`, `NSEdgeInsets`, AudioStreamBasicDescription
   and similar). Apple's public documentation gives the names, signatures,
   availability and most constant values. It does not always give struct
   layouts or numeric enum values. Those would have to be inferred, and in
   the variant of question 4 checked on the M4.
2. **The libSystem and libobjc surface.** STXlib needs only mach and libc.
   The Objective-C runtime ABI (`objc_msgSend`, class references
   `_OBJC_CLASS_$_NSWindow`, selector references) is documented in Apple's
   published objc4 sources and implemented in clang itself. Darwin's libc
   headers are partly published by Apple as open source (question 5).
3. **Text-based stubs (.tbd).** A .tbd is a YAML (v1 to v4) or JSON (v5)
   description of a dylib: install name, current and compatibility version,
   targets, and exported symbols. LLVM's TextAPI reads and writes every
   version. `llvm-readtapi` gained `--stubify` in the LLVM 18 cycle
   [LLVM readtapi commit], and the LLVM 19 cycle added stubifying whole
   directories. lld reads .tbd files directly. Hand-written stubs need, for
   each framework, its install name (for example
   `/System/Library/Frameworks/AppKit.framework/Versions/C/AppKit`) and the
   symbols the program uses. The linker checks only that the symbols exist
   in the stub. dyld checks them again at launch.
4. **Our own C++ runtime.** This is already done (section 2.1).

Effort: about 360 identifiers is a bounded job, perhaps one or two weeks of
focused work for the host plus audio, and a generator could keep it in step
with the sources. Once the set exists, a host change that uses a new AppKit
call has to add its declaration. A clean-room check could catch that, the way
`check_renderer_boundaries` works. **The risk is silent ABI error:** a wrong
struct layout, enum value or variadic signature compiles and links cleanly
and fails only at run time on the Mac. The M4 test runs (section 8) are what
catch it, which is one more reason they stay.

## 5. The toolchain off the Mac

- **Compiler:** clang 22 is a cross-compiler by default. Set
  `-target arm64-apple-macos14`, or `x86_64-apple-macos14`, and point
  `--sysroot` at the SDK of route B or C. The Darwin branch of
  `cmake/llvm22.cmake` would need a toolchain file that sets
  `CMAKE_SYSTEM_NAME Darwin`, `CMAKE_OSX_SYSROOT` and the target, and drops
  the `xcrun` call.
- **Linker:** `ld64.lld`. LLVM's new Mach-O port became the default
  `ld64.lld` around LLVM 12 [LLVM discourse 57346]. Chromium "uses LLD for all
  macOS and iOS builds", including release builds and ThinLTO, and documents
  that ld-prime cannot be used when cross-compiling [Chromium linkers].
  lld uses LLVM 22's own LTO, so moving to lld would also **remove the reason
  LTO is off on macOS** (line 222). Not checked: whether lld accepts
  `-no_warn_duplicate_libraries`. If it does not, the option has to be
  guarded by linker.
- **Mach-O tools:** `llvm-otool`, `llvm-install-name-tool`, `llvm-lipo`,
  `llvm-objdump --macho`, `llvm-nm` and `llvm-strip` replace Apple's tools in
  `package_programs.py` one for one.
- **Runtime:** `build_macos_runtimes.py` builds libc++, libc++abi and
  libunwind on the Mac today. Under route C it could cross-build them against
  the clean-room libc subset.
- **Can be done now, on the M4, with no licence question:** switch the macOS
  link to `ld64.lld` with Xcode's SDK. That proves the linker half of the
  route on Apple hardware, and LTO comes with it.

**x86_64.** GUI.Forms refuses anything but arm64 today. Supporting x86_64
would mean:
- universal binaries (`llvm-lipo`);
- an x86_64 build of the runtime;
- a deployment-target audit (`tools/audit_macos_minimum.py`);
- tests under Rosetta 2 on the M4.

The target is fading. macOS 26 is the last release for Intel Macs, and
Apple's developer news lists macOS 27 as the "final release to support
Rosetta" for Intel-only apps on Apple silicon [Apple developer news w5ngl9k2;
MacRumors 2026-06-10]. Testing x86_64 on the M4 would therefore stop working
after macOS 27.

## 6. Packaging off the Mac

- **.app:** a directory layout plus `Info.plist`. Python's `plistlib` writes
  plists anywhere. The dylib closure and the `@rpath` rewrite use
  `llvm-otool` and `llvm-install-name-tool`. Any edit to a Mach-O invalidates
  its signature, so signing comes last (section 7).
- **zip:** `ditto --sequesterRsrc` matters only for resource forks and
  extended attributes, and our bundles have none. A plain zip that keeps
  symlinks (`zip -y`, or Python's `zipfile` with the mode bits set) is enough.
  Apple's notary service accepts zip, pkg and dmg.
- **pkg:** a flat package is a xar archive holding `Distribution`,
  `PackageInfo`, a `Bom` and a `Payload`. The open-source `mkbom` from
  bomutils makes the Bom, and xar wraps the whole [bomutils tutorial].
  Upstream bomutils is archived and marked deprecated, so a maintained fork
  is needed. One report says xar 1.5 is too old to sign with, so use 1.6.1 or
  later. Another says a package built with xar alone failed with an installer
  page-controller error until the Distribution XML was right. `apepkg`, a
  Linux port of munki-pkg, does the job with bomutils and xar and notarises
  with rcodesign [apepkg].
- **dmg:** libdmg-hfsplus (`hfsplus`, `dmg`) plus `mkfs.hfsplus` or
  `genisoimage -apple`. Firefox's Linux cross builds make release DMGs this
  way [Firefox build-dmg-hfsplus.sh]. The fork matters: one Mozilla report
  found the original made DMGs that failed with "Invalid checksum" until it
  switched forks [Tor ticket 10207]. A pure-Go HFS+ and UDIF writer also
  exists [leaanthony/dmg]. rcodesign signs a DMG but cannot create one, and
  cannot sign the files inside it (section 7). The order is therefore:
  1. sign the .app;
  2. build the dmg;
  3. sign the dmg;
  4. notarise it;
  5. staple it.

The zip and pkg we ship today are enough. A dmg is optional (decision 5).

## 7. Signing and notarisation off the Mac

**The tool.** rcodesign is the CLI of the apple-codesign Rust crate
(documentation for 0.29) [rcodesign docs]:
- It signs Mach-O files, bundles, flat packages (xar) and DMGs, and
  notarises and staples them.
- It runs on Linux, Windows and the BSDs.
- Keys can live in PEM or PKCS#12 files, on a YubiKey, behind PKCS#11, or
  with a remote signer.
- Since 0.17 the notarisation client is native Rust over Apple's Notary API,
  and Apple's Transporter is no longer needed [apple-codesign changelog;
  Szorc 2022].
- Mozilla moved Firefox's notarisation from `altool` to rcodesign
  [Mozilla bug 1774273].

**Known limits** [rcodesign quirks]:
- It does not create DMGs and does not sign their contents.
- It cannot modify the contents of a pkg (no BOM writer), but it can sign
  one that bomutils built.
- It has a long tail of small differences from Apple's `codesign`.
- The Linux musl builds lack PKCS#11. The gnu builds need glibc 2.35.

**The release flow, all on Linux:**
1. `rcodesign sign` each nested Mach-O, then the bundle, using the Developer
   ID Application certificate with the hardened runtime and a secure
   timestamp. Notarisation requires all three: Developer ID, hardened
   runtime and a timestamp, and no `get-task-allow` entitlement
   [Apple notarizing doc].
2. Build the zip, pkg or dmg. Sign the pkg with the Developer ID Installer
   certificate, or the dmg with the Application certificate.
3. Run `rcodesign notary-submit --api-key-file key.json --wait --staple
   <artifact>`. `--staple` waits for acceptance and attaches the ticket.
   `rcodesign staple` works on its own, and `notary-log` fetches Apple's
   report [rcodesign notarizing].

Underneath, the Notary API (`https://appstoreconnect.apple.com/notary/v2/submissions`)
takes a JWT signed with ES256 from the API key's key ID and issuer ID
[Apple Notary API; Apple JWT]. Forum reports show submissions sometimes stuck
"In Progress" for hours, so the release job needs a timeout and a resume step.

**What the owner must provide.** Only the owner can do these steps.
1. **Apple Developer Program membership:** 99 USD per membership year, or
   the local equivalent [Apple membership]. An organisation enrolment shows
   the organisation's name and needs a D-U-N-S number. An individual
   enrolment shows the owner's personal name in Gatekeeper and on the
   certificate.
2. **Developer ID certificates.** Only the Account Holder can create them
   [Apple Developer ID help; Apple roles]. There is a limit of five Developer
   ID Application and five Developer ID Installer certificates. The private
   key need not touch a Mac:
   - `rcodesign generate-certificate-signing-request` (or openssl with
     RSA 2048) makes the CSR;
   - the owner uploads it in the developer portal as "Developer ID
     Application", G2 Sub-CA;
   - the owner downloads the .cer. Repeat for "Developer ID Installer" if
     the pkg is kept.
3. **An App Store Connect API key** (Users and Access, Integrations, Team
   key). The Developer role is enough. The owner gets the issuer ID, the key
   ID and a `.p8` file that can be downloaded only once.
   `rcodesign encode-app-store-connect-api-key` folds them into one JSON
   file [rcodesign getting started].
4. **Key custody** (decision 2). The Developer ID private key signs as the
   owner, and Apple revokes compromised certificates, which breaks every
   release signed after the compromise date. The API key can notarise but
   cannot sign.

## 8. Testing: what needs no Mac and what still runs on the M4

**Off the Mac (Linux CI):**
- The host-free and renderer-neutral GUI.Forms suites and all of STXlib
  already run on Linux. Building them for Darwin proves only that they
  compile.
- **Mach-O structure:**
  - `llvm-objdump --macho --private-headers` shows `LC_BUILD_VERSION`
    (platform macOS, minos 14.0, sdk), the load commands and the rpaths.
    `tools/audit_macos_minimum.py` could switch to llvm tools.
  - `llvm-nm -u` lists undefined symbols, to check against the .tbd stubs.
    This proves the link, not the run.
  - The architecture set (`llvm-lipo -info`).
- **Bundle checks:** `Info.plist` keys, the dylib closure (no absolute paths
  outside `/System` and `/usr/lib`), and every Mach-O signed.
- **Signature checks:** `rcodesign verify` and `print-signature-info` show the
  CodeDirectory hashes, team ID, flags and timestamp.
- **Notarisation acceptance:** Apple's service scans the artifact and returns
  a log. That is a real off-Mac check of signing, hardened runtime and
  entitlements, done by Apple's own server.

**Still on the M4** (or on GitHub `macos-15`, which is Apple hardware):
- Anything that runs a Mach-O. No emulator runs AppKit programs. Darling's
  own documentation says "GUI applications will not work in Darling at this
  point in time, with very few exceptions" [Darling docs], and a macOS VM off
  Apple hardware is outside the licence (section 3).
- **AppKit behaviour:** windows, input, menus, IME, accessibility, IOSurface
  presentation, CoreText layout, audio, and the visual-regression captures.
- **Route C's ABI risk:** the first launch of every program built from the
  clean-room set, and a smoke test over each host service.
- **Gatekeeper on a downloaded copy:** a quarantined download, then
  `spctl --assess --type execute`, the offline check of the stapled ticket,
  and first launch.
- **x86_64 under Rosetta,** if decision 4 adds it, and only until macOS 27.

So the M4 keeps the test half of every route. What leaves the Mac is the
build or release half. That splits the M4's slots across fewer jobs; it does
not retire the M4.

## 9. Costs

| Item | Cost | Notes |
|---|---|---|
| Apple Developer Program | 99 USD a year | needed for any Developer ID release, Mac or not |
| GitHub standard macOS runners | 0 for public repos; 0.062 USD a minute otherwise | both repos are public [GitHub pricing] |
| rcodesign, bomutils, xar, libdmg-hfsplus | 0 (open source) | forks to choose and pin |
| Route A work | about 2 to 4 days | `package_programs.py` on llvm tools, plistlib, zip and xar; rcodesign stage; secrets; release job |
| lld switch on the M4 | about 1 to 2 days | guard the ld-prime flags; turn LTO on and measure |
| Route C work | about 1 to 2 weeks, plus upkeep | headers, stubs, a check like `check_renderer_boundaries`, a Linux toolchain file |
| x86_64 | about 1 week, plus a test lane that ends with macOS 27 | runtime, lipo, audits |

These are estimates, not measurements.

## 10. Risks

1. **Licence (routes B and C).** This is the EFF question in section 3. No
   SDK-derived code before the answer.
2. **Signing key compromise.** The certificate is revoked and every release
   signed after the revocation date is broken. Mitigations: a hardware token
   or a remote signer, keeping the key out of CI logs, and a key used for
   nothing else.
3. **Notary delays.** Submissions sometimes sit for hours. The release job
   should poll with a timeout and be able to resume from a submission ID.
4. **rcodesign differences.** Its documented long tail of small differences
   from `codesign`. Notarisation acceptance plus the M4 Gatekeeper check
   (section 8) catch them. Pin the version.
5. **Abandoned packaging tools.** bomutils upstream is archived, and
   libdmg-hfsplus has had bad forks. Pin a fork and test the result on the
   M4 at each pin change.
6. **Silent ABI errors (route C)**, caught only at run time on the Mac.
7. **Apple policy changes:** Notary API versions, Gatekeeper rules (macOS 15
   already tightened the bypass), and the deployment floor. Apple controls
   all three.
8. **Single point of failure.** Only the Account Holder can create Developer
   ID certificates. If the owner is unavailable, a lost or expired
   certificate cannot be replaced.

## 11. Recommendation and decisions for the owner

**Recommendation.** Take route A now, try the lld switch on the M4 next, and
leave routes B and C waiting for the EFF.
1. **Now (no licence question):**
   - Enrol, create the Developer ID certificates through a CSR so the key
     never touches a Mac, and create an App Store Connect API key.
   - Move signing, packaging and notarisation to a Linux release job using
     rcodesign and the Notary API. Replace Apple's `otool`,
     `install_name_tool`, `codesign`, `ditto` and `pkgbuild` with LLVM tools,
     plistlib, zip, xar and bomutils.
   - Keep compiling on the M4 and on the free `macos-15` runners.
   - Result: Developer ID notarised releases, an end to the Open Anyway trip
     in System Settings, and a release path that needs no Mac after the
     compile.
2. **Next:** switch the macOS link to `ld64.lld` on the M4 with Xcode's SDK.
   This removes the libLTO limit, so LTO comes on, and it proves the
   cross-link half of routes B and C on Apple hardware.
3. **After the EFF answers:** if a clean-room set is cleared, prototype
   route C for STXlib first (libSystem only), then the AppKit host. If the
   SDK on Linux is cleared, route B is the cheaper path. If neither is
   cleared, route A is the final state, and it still delivers everything
   except a Mac-free compile.
4. Stay **arm64 only**. Keep the zip and pkg, and add a dmg only if users ask
   for one.

**Decisions:**

1. **Enrolment.**
   - (a) Organisation enrolment (recommended). Gatekeeper shows the
     organisation's name. It needs a D-U-N-S number, and Apple's verification
     takes days to weeks.
   - (b) Individual enrolment. Fastest, but the owner's personal name appears
     on every signature.
   - (c) No enrolment. Releases stay ad hoc and every user goes through Open
     Anyway. Routes B and C would still deliver unsigned builds only.
2. **Where the Developer ID key lives.**
   - (a) A hardware token (YubiKey) on a trusted machine, signing as a manual
     or semi-manual release step. Safest, but needs the owner or a trusted
     machine at each release.
   - (b) An encrypted PKCS#12 file in GitHub Actions secrets. Fully automatic
     releases, but a CI compromise exposes the key.
   - (c) rcodesign's remote signing, with the key on the owner's machine and
     CI asking for each signature. Automatic with consent, but more moving
     parts.

   The recommendation is (a) for 1.0 and (c) later. The API key can live in
   CI secrets under any option.
3. **Where macOS compiles.**
   - (a) Apple hardware only: route A (recommended now). No licence question,
     and the M4 plus free runners stay.
   - (b) Linux with Apple's SDK: route B. Proven technically, but waits on
     the EFF's answer to questions 1 and 2.
   - (c) Linux with a clean-room subset: route C. One to two weeks plus
     upkeep, waits on questions 3 to 5, and carries ABI risk caught only on
     the M4.
4. **Architectures.**
   - (a) arm64 only (recommended). No change. Intel Macs stop at macOS 26.
   - (b) Add x86_64 as a universal binary. Reaches Intel Macs on macOS 14 to
     26, costs about a week, and its Rosetta test lane on the M4 ends with
     macOS 27.
5. **Formats.**
   - (a) zip and pkg as today (recommended). The pkg needs the Installer
     certificate.
   - (b) Add a dmg. That means libdmg-hfsplus in the release job and a
     sign-the-app, build, sign, notarise order.
   - (c) zip only. Drops the Installer certificate and the bomutils
     dependency.
6. **Linker.**
   - (a) Try `ld64.lld` on the M4 now (recommended). It turns LTO on for
     macOS and readies routes B and C. Measure size and speed and keep it
     only if the tests pass.
   - (b) Stay on ld-prime. No work, LTO stays off, and a later cross-build
     still needs the switch.

## 12. Sources

- [Apple SLA] Xcode and Apple SDKs Agreement, EA2002 06/08/2026:
  https://www.apple.com/legal/sla/docs/xcode.pdf
- [Apple macOS SLA, older texts] for example
  https://www.apple.com/legal/sla/docs/macosx1072.pdf and
  https://apple.com.cn/legal/sla/docs/macOS1012.pdf
- [Apple forums 845455] macOS 27 EULA thread:
  https://developer.apple.com/forums/thread/845455
- [Apple membership] https://developer.apple.com/support/compare-memberships
  and https://developer.apple.com/programs/enroll/
- [Apple Developer ID help]
  https://developer.apple.com/help/account/certificates/create-developer-id-certificates/
- [Apple roles] https://developer.apple.com/support/roles
- [Apple Notary API] https://developer.apple.com/documentation/notaryapi
- [Apple JWT]
  https://developer.apple.com/documentation/appstoreconnectapi/generating-tokens-for-api-requests
- [Apple notarizing doc]
  https://developer.apple.com/documentation/security/notarizing-macos-software-before-distribution
- [Apple developer news w5ngl9k2] https://developer.apple.com/news/?id=w5ngl9k2
- [MacRumors 2026-06-10]
  https://www.macrumors.com/2026/06/10/macos-golden-gate-last-to-support-intel-apps/
- [MacRumors 2024]
  https://www.macrumors.com/2024/08/06/macos-sequoia-gatekeeper-security-change/
- [rcodesign docs] https://gregoryszorc.com/docs/apple-codesign/main/
- [rcodesign notarizing]
  https://gregoryszorc.com/docs/apple-codesign/main/apple_codesign_rcodesign_notarizing.html
- [rcodesign getting started]
  https://gregoryszorc.com/docs/apple-codesign/main/apple_codesign_getting_started.html
- [rcodesign quirks]
  https://gregoryszorc.com/docs/apple-codesign/main/apple_codesign_quirks.html
- [apple-codesign changelog]
  https://docs.rs/crate/apple-codesign/0.18.0/source/CHANGELOG.rst
- [Szorc 2022]
  https://gregoryszorc.com/blog/2022/08/08/achieving-a-completely-open-source-implementation-of-apple-code-signing-and-notarization/
- [Mozilla bug 1774273] https://bugzilla.mozilla.org/show_bug.cgi?id=1774273
- [osxcross README.SDK]
  https://raw.githubusercontent.com/tpoechtrager/osxcross/master/README.SDK.md
- [LLVM discourse 57346]
  https://discourse.llvm.org/t/making-ld64-lld-default-to-the-new-mach-o-port/57346
- [Chromium linkers]
  https://chromium.googlesource.com/chromium/src.git/+/main/docs/apple_platform_linkers.md
- [LLD Mach-O] https://lld.llvm.org/MachO/
- [LLVM readtapi commit]
  https://lists.llvm.org/pipermail/all-commits/Week-of-Mon-20231218/142121.html
- [bomutils tutorial] https://hogliux.github.io/bomutils/tutorial.html
- [apepkg] https://github.com/peetinc/ape-pkg
- [Firefox build-dmg-hfsplus.sh]
  https://sources.debian.org/src/firefox/147.0.4-1/taskcluster/scripts/misc/build-dmg-hfsplus.sh
- [Tor ticket 10207] https://gitlab.torproject.org/legacy/trac/-/issues/10207
- [leaanthony/dmg] https://pkg.go.dev/github.com/leaanthony/dmg
- [Darling docs] https://docs.darlinghq.org/known-nonfunctional-software.html
- [GitHub pricing] https://docs.github.com/en/billing/reference/actions-runner-pricing
