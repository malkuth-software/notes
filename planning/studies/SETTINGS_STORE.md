# Study: a settings store in STXlib, and its GUI.Forms layer

Date: 2026-11-22, revision 2. Status: a study for the owner, who approved
the work ("a settings store is also stxlib centered, because it involves
cross platform capability"). No code was written and nothing was built.
Section 9 ends with the decisions for the owner.

Revision history:
- 1 (`4bdfd12`): the survey, the design and eight decisions. The owner
  accepted it except for decision 1 and section 4.5. Decisions 2 to 8 are
  accepted as recommended.
- 2 (this one): section 4.5 and decision 1 rewritten against SwiftEdit's
  canon on files someone else changed (section 4.5.1). A note on
  section 4.6 and decision 2 records where the canon touches decisions
  already accepted.

Read against:
- STXlib `2ac73e4`;
- GUI.Forms `c1bbe4af3`;
- games (PlaySuite) `8fe12ac`;
- Plan Paint `3fc2b66`;
- SwiftEdit `claude/stxlib-syntax` at `09dacd6`;
- the data_workbench demo, `claude/devexpress-demos` at `3cd403b18`.

## 1. Summary

Every program that saves state builds its own folder path from environment
variables, writes its own file format and decides alone how carefully it
writes. The results differ in ways users would notice:
- A game's settings and Plan Paint's settings live in different kinds of
  folder on Linux.
- One crash can wipe a game's save, because there is no flush before the
  rename. Plan Paint's file survives the same crash.
- Plan Paint and the ribbon state cannot tell a damaged file from a good
  one.
- Nobody keeps a last good copy.
- Nothing guards two copies of one program writing at once.

STXlib already has the hard parts: the per-user folders, an atomic,
durable `write_file` and a hash. It lacks three things:
- an atomic replace on Windows that the library vouches for;
- a lock between processes;
- a settings format.

The proposal:
- `stxlib::settings`: one text file per program, or per program and part.
  It is named by the program's identity and holds typed, bounded,
  versioned, checksummed entries, with a last good copy and a rule for
  several processes.
- A thin GUI.Forms layer on top, so window placement, grid, ribbon and
  dock layouts and recent files share one API, reachable from C and C#.

## 2. What the programs do today

### 2.1 Games (PlaySuite)

- **Folder.** `src/runtime_paths.cpp` `state_directory()`, hand-rolled
  from `getenv`. `GAMES_STATE_DIR` overrides it. Otherwise it is:
  - Windows: `%APPDATA%\Rainstar\Games` (roaming);
  - macOS: `~/Library/Application Support/Rainstar/Games`;
  - Linux: `$XDG_STATE_HOME/rainstar/games`, else
    `~/.local/state/rainstar/games`.

  It throws if none can be named.
- **Per-game data.** `src/game_data.cpp` `save_game_data(game, name,
  bytes)`:
  - names are checked (`[a-z0-9_.-]`, at most 64, no leading dot);
  - the limit is 1 MiB;
  - it writes `<path>.tmp` with `std::ofstream` and flush, then calls
    `std::filesystem::rename`;
  - no fsync, so after a power loss the file can be empty.
- **Save envelope.** The new-game template (`new-games/template/src/save.cpp`)
  and catching thieves use the same shape:
  - a magic line that carries the version (`TEMPLATEGAME1`);
  - the body;
  - a last line `check=<FNV-1a 64, decimal>`;
  - a 64 KiB limit.

  `unseal` checks the magic, the size and the checksum. On any failure the
  game starts fresh and "never guesses". Catching thieves migrates step by
  step (`migrate`, format 1 to 2).
- **Suite settings.** `src/suite_settings.cpp`, file
  `playsuite-settings-v1.txt`:
  - header `PLAYSUITE_SETTINGS 1`, then `key=value` lines (flags 0 or 1,
    percentages);
  - each key is checked on its own, and a bad value keeps its default;
  - a migration renames a key: `card_back` became `card_backs`, and the old
    key is ignored so everyone gets the new default.
- **Cabinet.** `src/storage.cpp`, `cabinet-v1.txt`, header `RAINSTAR_GAMES 3
  <checksum>`, tmp file and rename.

### 2.2 Plan Paint

- **Folder.** `src/desktop.cpp` `preference_directory()`, hand-rolled:
  - macOS: `~/Library/Application Support/rainstar/RainstarPaint`;
  - Windows: `%APPDATA%\rainstar\RainstarPaint`;
  - Linux: `$XDG_DATA_HOME` or `~/.local/share/rainstar/RainstarPaint`,
    which is the data folder, not the configuration folder.

  The pre-1.0 names are kept on purpose.
- **Editor settings.** Format `RSPS8` in this checkout. The coordinator's
  brief says RSPS9 and `docs/THEMES_AND_RECOVERY.md` still says RSPS7: the
  documentation lags the code.
  - Values are positional and separated by whitespace, with no keys.
  - A value is read only when the version is at least N.
  - Ranges are clamped.
  - There is no checksum.
  - Reads stop at 4096 bytes.
  - Writes go through `write_file_atomic`.
- **Recent files.** `recent-files.bin`:
  - binary, with u32 length prefixes in host byte order, so the file does
    not move between hosts;
  - at most 12 entries of at most 65536 bytes each;
  - on Windows, comparisons fold case and separators.

  `custom-colors.bin` is the same kind of file.
- **Atomic write.** `src/safe_file.cpp` `write_file_atomic` is the best
  writer here:
  - Windows: `CreateFileW` with `CREATE_NEW` for a temporary file, then
    write, `FlushFileBuffers`, then `SetFileInformationByHandle` with
    `FileRenameInfoEx` and `FILE_RENAME_FLAG_POSIX_SEMANTICS`;
  - POSIX: fsync, then `rename` (or `link` when not replacing).
- **Recovery.** `src/recovery.cpp` keeps snapshots of documents, separately
  from settings.

### 2.3 SwiftEdit

SwiftEdit saves nothing. Its About text says "Settings are session-only".
Font role and size, wrapping and language last for the session.

### 2.4 GUI.Forms

- **Themes.** `src/runtime/user_themes/theme_files.{hpp,cpp}`:
  - `program_configuration_folder(program)` reads `HOME`, `APPDATA` and
    `XDG_CONFIG_HOME` itself. STXlib's `known_directory` was written to
    replace exactly this (files.hpp says so).
  - `write_new_file` only creates files and never replaces one ("a partly
    written file may remain").
  - Reads and listings are bounded.
- **Ribbon state.** `include/gui_forms/controls/ribbon_state.hpp`:
  - `RibbonState` holds the quick-access command IDs, the placement,
    `minimized`, and `unrecognized_entries`, which are kept for the round
    trip;
  - the text is `gui_forms.ribbon_state 1` plus `key=value` lines;
  - a parse never fails as a whole: diagnostics are per line and defaults
    are kept (the configuration contract, §7);
  - the program stores the text (RB-44).
- **Dock layout.**
  - Files: `docking/dock_layout/dock_layout.hpp` and
    `dock_layout_persistence.cpp`.
  - XML format 2, versions upgraded step by step, with a callback for the
    program's own version (`application_version`).
  - Unknown elements and attributes are kept (DK-48).
  - Depth and size are limited, with a fallback when a monitor is gone
    (DK-34).
  - `DockWorkspace::save_layout()` and `load_layout(xml)` run on request
    only; the workspace never saves on its own (DK-33).
  - Named layouts live in memory.
- **Help book.** Bookmarks come out through `set_bookmarks` and the
  `bookmarks_changed` event. The caller persists them.
- **Window placement.** There is no public type.

### 2.5 The data_workbench demo

`layout_text()` writes the grid layout as
`v1|name:width:display:visible|...|sort=none|col:a/d`. It is kept in
memory for the session. The demo's header says why: GUI.Forms has no
per-user settings store.

### 2.6 What repeats and what differs

| | Games | Plan Paint | Ribbon | Dock | Themes |
|---|---|---|---|---|---|
| Folder (Linux) | state | data | program's | program's | config |
| Folder source | getenv | getenv | n/a | n/a | getenv |
| Version in | magic line | magic line | header line | XML attribute | none |
| Keys | yes | no (positional) | yes | yes | n/a |
| Unknown entries kept | no | no | yes | yes | n/a |
| Checksum | FNV-1a 64 | no | no | no | no |
| Size bound | yes | yes | caller | yes | yes |
| Flush before rename | no | yes | caller | caller | create only |
| Last good copy | no | no | no | no | no |
| Several processes | no | no | no | no | no |

Every program repeats four things:
- the folder lookup;
- a format header that carries a version;
- per-entry validation that falls back to a default;
- a temporary file and a rename.

Nobody has a last good copy, a lock, or change notification.

## 3. What STXlib already gives

- **Folders.** `known_directory(KnownDirectory)` in `files.hpp` returns
  `home`, `user_configuration`, `user_data` or `user_cache`:
  - macOS: Application Support (Caches for the cache);
  - Linux: the XDG folders, following the specification (an empty or
    relative variable is ignored);
  - Windows: `SHGetKnownFolderPath` with RoamingAppData, RoamingAppData
    and LocalAppData. A UNC redirect is refused.

  The capability is `Capability::known_directories`. The folder need not
  exist yet. There is no `user_state` (XDG_STATE_HOME), which the games use,
  and no local configuration on Windows.
- **Writing.** `write_file(path, content, Durability)` writes a temporary
  file with a unique name in the same folder, flushes it as asked, then
  renames it over the target. On failure the old file is untouched and the
  temporary file is removed.
  - `Durability::durable` also makes the rename durable: the folder is
    flushed on POSIX, and Windows uses `MOVEFILE_WRITE_THROUGH`.
  - `Capability::atomic_replace` is true on POSIX and false on Windows.
    `file_windows.cpp` uses `MoveFileExW` and says it is not documented as
    atomic.
  - Plan Paint's `FileRenameInfoEx` with POSIX semantics is the call that
    would let STXlib say true on Windows 10 1709 and later on NTFS. On older
    systems, or on file systems without it, it has to fall back.
- **Reading.** `read_file(path, limit)` checks the limit before allocating
  and fails `limit_exceeded` past it.
- **Hashing.**
  - `hash.hpp` has BLAKE2s-256, plain and keyed.
  - `syntax.hpp` has an internal FNV-1a word hash, but no public FNV-1a 64
    over bytes.

  For detecting damage, either works. A public 64-bit FNV-1a costs a few
  lines and matches the games' envelope.
- **Paths and environment.** UTF-8 `Path` carries host names losslessly,
  and `environment.hpp` reads under a lock.
- **Missing.**
  - A lock between processes: there is no `LockFileEx`, `flock` or
    `F_OFD_SETLK` anywhere in STXlib.
  - A file watch for change notification.

## 4. `stxlib::settings`: the design

### 4.1 Identity and placement

A store is opened by the program's identity, the same one MCP uses:
`app.<vendor>.<appname>`, for example `app.malkuthsoft.planpaint`. The
identity is checked: lowercase ASCII letters, digits, `-` and `.`, three
or more parts, at most 128 bytes. It becomes one folder:
`<known_directory>/<identity>/`.

Placement is chosen per store, not per value:
- `Placement::roaming`: `user_configuration`, which roams on Windows.
  This is for what the user chose: themes, preferences, the quick-access
  toolbar.
- `Placement::local`: `user_data` on macOS and Linux. On Windows it is
  LocalAppData, which `known_directory` gains as `user_local_configuration`
  (decision 3). This is for what belongs to this machine: window placement,
  monitor-bound dock layouts, recent files with local paths.

A program has one roaming store and one local store at most, each a small
set of named files (`preferences`, `layout`, `recent`). Each file is
written whole.

Existing folders (`Rainstar/Games`, `rainstar/RainstarPaint`) are not
moved silently. A program that adopts the store reads its old file once,
on first open, through an import callback, writes the new file, and leaves
the old one in place for one release (decision 6).

### 4.2 Values and schema

The program declares a schema: a version and a list of entries. Each entry
has:
- a key: dotted lowercase, at most 64 bytes;
- a type: bool, signed 64-bit integer with a range, double with a range,
  text with a byte limit, an enumeration with its names, or a list of one of
  these with a count limit;
- a default.

Reads are typed: `get_bool(key)` and the like. A key the schema does not
declare is a programming error, which fails fast.

What happens on load:
- A stored value that fails its type or range keeps the default and adds a
  diagnostic. This is the configuration contract (§7) that RibbonState and
  the suite settings already follow.
- Unknown keys are kept and written back unchanged, as RibbonState and the
  dock layout do. An older copy of a program then does not erase what a
  newer one saved.
- Migrations are numbered steps from N to N+1, each a function over the
  parsed entries (rename, drop, convert), run in order on load. A file
  newer than the schema is read as far as its known keys go and is not
  written back, so the newer program's data survives (decision 5).

A program can store an opaque payload as one text value with a limit:
RibbonState text, dock XML or the grid layout. Those formats keep their own
versions and their own unknown-entry rules. The store only gives them a
safe home.

### 4.3 The file

The format is text, after the games' envelope:

```
stxlib.settings 1
schema=app.malkuthsoft.planpaint/preferences 4
brush.size=3.5
canvas.background=#ffffff
recent.path=/Users/x/a.rspaint
recent.path=/Users/x/b.rspaint
check=blake2s:3f9a...
```

- The file is UTF-8 with LF line endings.
- Each line holds one entry. Repeated keys make a list.
- Values are escaped: `\n`, `\\`, and `\xHH` for control bytes.
- The last line is a checksum over everything before it.

Text keeps the file readable and lets a person fix it by hand, as the games
allow. The checksum tells a damaged file from a good one, and a hand-edited
file whose checksum is wrong is still read (decision 4).

- **Bounds.** At most 1 MiB per file by default, and lower if the program
  says so. The limit is checked by `read_file` before allocating. Line
  length, key count and list counts are also bounded.
- **Secrets.** Never stored here. The documentation and the schema both
  say so. A text entry whose key ends in `password`, `token`, `secret` or
  `key` is refused when the schema is built. Secrets go to the host's
  keychain (Keychain, the Credential Manager, Secret Service), which is a
  separate study.

### 4.4 Writing, damage and the last good copy

- **Saving.** A save serialises, then calls `write_file(path, bytes,
  Durability::durable)`. Before replacing the file, the current file is
  copied to `<name>.last-good`, but only if it parsed with a correct
  checksum.
- **Loading.** If the checksum is wrong or the parse fails, the store reads
  `<name>.last-good`. If that fails too, it uses the defaults. In both
  cases the damaged file is renamed `<name>.damaged-<UTC time>`, keeping at
  most three such files, and a diagnostic says what happened. The store
  never guesses, as the games' rule says.
- **Windows.** STXlib's Windows `write_file` gains the rename that Plan
  Paint uses: `FileRenameInfoEx` with `FILE_RENAME_FLAG_POSIX_SEMANTICS |
  FILE_RENAME_FLAG_REPLACE_IF_EXISTS`. Where the system refuses it, it
  falls back to `MoveFileExW` with `MOVEFILE_REPLACE_EXISTING |
  MOVEFILE_WRITE_THROUGH`. `Capability::atomic_replace` then reports what
  this host actually has. `ReplaceFileW` is the other candidate (decision
  2): it keeps the old file's ACL and attributes and can make a backup in
  one call, but Microsoft does not document it as atomic either, and it
  fails when the target is missing.

  With a last good copy, a non-atomic rename on an old system costs at most
  the newest save, never the settings.

### 4.5 Several processes of one program

#### 4.5.1 The canon

SwiftEdit's objective ledger (`docs/SWIFTEDIT_OBJECTIVES.md`, from the
owner's interview of 2026-09-30) outranks its `DECISIONS.md`. It rules how
a program treats a file someone else changed:
- **External changes.** Views are frozen. A destructive save takes two
  stages: first overwrite or a new copy, then an editable file name and
  metadata, with an explicit warning. There is no compare and no merge.
- **Copies.** A dot version goes before the extension, and an existing dot
  number is incremented.
- **Lifetime.** Document windows are independent. New Window launches the
  same running executable as a separate process.

`DECISIONS.md` rows 40, 43 and 44 give the mechanics:
- On save, the program compares the file's identity, its timestamp and its
  exact original bytes.
- A changed or deleted target fails the save, and the edits stay in memory.
- There is no automatic reload and no watcher.
- The path is checked again immediately before the replacement is
  published.

Plan Paint's recovery leases (`docs/THEMES_AND_RECOVERY.md`, lines 43-47)
apply the same idea between running sessions:
- each live session holds a lease;
- one session never takes another's file;
- there is no guessing by PID or timeout.

#### 4.5.2 What carries over to settings unchanged

- **Detection.** When a store loads a file, it keeps:
  - the file's identity (device and inode on POSIX; volume serial and file
    index on Windows);
  - its modification time;
  - its exact bytes. They are bounded at 1 MiB, so the store keeps them
    and does not need a hash.

  Before it publishes, the store checks the file against what it keeps.
  This happens under the lock, right before the rename, as row 44 asks.
  A hand edit, another window, or a newer program writing the file is
  "someone else" in the same way.
- **Frozen view.** A process's values do not change under it. What it
  loaded and what it changed stay as they are until the program asks.
- **Never overwrite silently.** Whatever another writer put in the file
  survives a save, or the save fails visibly.
- **Copies.** Where something cannot be written to the file, it goes to a
  dot-numbered copy (`preferences.1.settings`), never nowhere.

#### 4.5.3 Where settings differ from documents

These differences are real, and they are why decision 1 is a choice:
1. **No dialog.** A document save is something the user does, so the
   program can ask them. A settings save happens on its own, often while a
   window closes. The two-stage dialog the canon prescribes has no moment
   to appear.
2. **Conflicts are routine.** Under the Lifetime rule every SwiftEdit
   window is its own process, so every window is a settings writer. With
   the strict document rule, the first window that saves makes every other
   open window's view stale. From then on only one window's settings ever
   reach the disk.
3. **No authored text.** A settings file is a set of independent values,
   not text the user wrote. Writing key A from one window and key B from
   another loses nothing either window did. The canon forbids comparing
   and merging an author's text, where a merge can silently produce
   something nobody wrote. A per-key write cannot produce a value nobody
   chose. Still, it is a merge of a kind, and the owner's ruling does not
   say it excludes only text.
4. **The cost of losing an update is small.** A lost settings change costs
   a column width or a toolbar entry. A lost document edit costs work.
   This argues for keeping the rule simple, not for overwriting.

#### 4.5.4 The mechanism common to every option

- **Lock.** A save takes an exclusive lock on `<identity>/.lock` for the
  check and the write, never longer. The lock is `LockFileEx` on Windows,
  `F_OFD_SETLK` on Linux, and `flock` on macOS and FreeBSD. It is STXlib's
  new `FileLock`, a scope object (house style §41), with a timeout of 2 s
  and a named failure, `busy`.

  The lock is the "stronger reservation" that row 44 lists as work still to
  do. It only binds processes that use the store. A hand edit is caught by
  the check instead.
- **Reads.** Reads take no lock. The rename guarantees that a reader sees
  the old file or the new one.
- **Pending changes.** A `busy` save, or one refused for a conflict, keeps
  its changes in memory, as row 43 does for documents.

The options differ in what a save does when the check finds the file
changed (decision 1).

- **a. Per key, refuse on a real conflict.** This is recommended. The
  canon is applied to each key as its own small target.
  - Under the lock, the store reads the file again. For each key this
    process changed, it compares the file's current value with the exact
    value it loaded.
  - If the value is the same, nobody else touched that key, so the store
    writes its change.
  - If the value differs, another writer changed it. The file's value
    stays, never overwritten. This process keeps its own value in memory
    and records a diagnostic, `conflict` with the key.
  - Keys this process did not change are taken from the file as they are.
  - On a later save, the conflicting change is still refused. GUI.Forms
    shows a non-modal notice ("settings changed in another window: keep
    theirs or use yours"). The user's choice is a new, explicit change.
  - At the last save of the process, any changes still in conflict go to a
    dot-numbered copy. The next start reports that copy once.

  Frozen view holds: `reload_if_changed()` (decision 7) adopts only keys
  this process has not changed.
- **b. Per key, the last save wins.** This was revision 1's rule. It works
  as in option a, but a key changed by both writers takes this process's
  value, with a diagnostic. It is simpler and needs no notice and no copy.
  It overwrites another writer's change to that key: what the canon
  forbids, limited to one key.
- **c. The whole file, refuse and copy.** This is the document rule
  without the dialog. If the file changed at all since this process loaded
  it, the save writes this process's whole settings to the next
  dot-numbered copy and leaves the file alone. No merge of any kind
  happens. With several windows, though, every window after the first
  writes a copy on every save, and the copies pile up until the user picks
  one. That needs a settings chooser that no program has.
- **d. The whole file, keyed by window.** Every process writes its own
  file (`preferences.<process>.settings`). On a clean exit the process
  publishes its file as the shared one only if the shared file is
  unchanged since it loaded. No merge and no conflicts happen. But
  settings scatter across processes, and a change made in one window
  reaches the next start only if that window closes last with nobody
  having saved meanwhile.

### 4.6 Change notification

A file watch (FSEvents, ReadDirectoryChangesW, inotify) is a large host
surface. Today it buys one thing: a second window sees a theme change at
once.

The proposal is no watch in revision 1. Instead:
- the store offers `reload_if_changed()`, which compares size, modification
  time and checksum;
- GUI.Forms calls it when a window is activated.

Every program gets most of the benefit, and no watcher thread is needed
(decision 7).

The canon (section 4.5.1) says documents have no automatic reload and no
watcher. Decision 7 is accepted and has no watcher. Its reload on window
activation is limited by the frozen view: it adopts only keys this process
has not changed, so a window never loses a change it made.

## 5. The STXlib interface, in outline

The interface follows STXlib's style: results instead of exceptions, owned
types, no `std::` in public headers.

```
namespace stxlib::settings {
enum class Placement : u8 { roaming = 0, local = 1 };
class Schema;        // built once: version, entries, migrations
class Store;         // one open file
struct Diagnostic;   // line, key, issue

OperationResult<Store> open(View<char> identity, View<char> file,
                            Placement placement, const Schema& schema);
// Store: get_* and set_* by key, list access, payload text,
// diagnostics(), save() -> OperationStatus, reload_if_changed().
}
```

New STXlib pieces this needs, each small and testable on its own:
1. `FileLock` (files.hpp).
2. Windows `write_file` with the POSIX-semantics rename, plus
   `atomic_replace` reported per host.
3. `KnownDirectory::user_local_configuration`, and possibly `user_state`.
4. A public FNV-1a 64, if decision 4 picks it over BLAKE2s.

## 6. The GUI.Forms layer

`gui_forms::Settings` wraps one roaming store and one local store per
application. Each GUI.Forms part gets a typed `save_to(Settings&)` and
`restore_from(Settings&)`, so a program writes two lines instead of a
format:

- **Window placement.** A new `WindowPlacement` value: the normal bounds
  in desktop units, the monitor's identity and work area, and the state
  (normal, maximised or full screen). It is stored locally. When restored,
  the window is fitted to the current monitors, with the same fallback the
  dock layout uses (DK-34).
- **Ribbon.** The RibbonState text, stored as a roaming payload. RB-44
  stays true: the program decides when to call it.
- **Dock layout.** `save_layout()` XML, stored as a local payload, with
  named layouts under `dock.layout.<name>`. DK-33 stays true: the workspace
  still never saves on its own.
- **Grid layout.** DataGridView gets a stable layout text: columns, order,
  widths, visibility and sort. This promotes data_workbench's `v1|...` text
  into the library with a version header. It is stored locally per grid
  name.
- **Recent files.** A `RecentFiles` list:
  - bounded (default 12), stored locally;
  - paths stored as UTF-8 `Path` text, compared as the host compares
    paths (Plan Paint's case and separator folding on Windows);
  - entries that no longer exist are dropped when shown, not when loaded.
- **Help book.** Bookmarks, stored roaming.
- **Themes.** `theme_files.cpp` moves onto `known_directory` and
  `write_file`, as files.hpp already plans. Theme files stay files of their
  own, because users copy and share them, but they move into the
  identity's folder.

**C ABI and C#.** These are value calls, not hooks:
- `gf_settings_open`, `gf_settings_get_*`, `gf_settings_set_*`,
  `gf_settings_save`, `gf_settings_close`;
- one `gf_<part>_save_settings` and one `gf_<part>_restore_settings` per
  part.

They go into the audited manifest, and `tools/abi_bindings.py` generates
the C# side. Class tables (`tools/abi_class_tables.py`) are needed only if
a program overrides how a control saves itself. The proposal is no
override in revision 1: a control's state is its own format.

## 7. Adoption order

1. STXlib: `FileLock`, the Windows rename, the new known directory, then
   `stxlib::settings` with its tests:
   - damage tests: a truncated file, a bad checksum, a damaged last good
     copy;
   - migration tests;
   - a test with two processes;
   - a size-limit test.
2. GUI.Forms: `theme_files.cpp` onto STXlib, then `gui_forms::Settings`,
   then `WindowPlacement` and the grid layout text.
3. SwiftEdit, which has nothing to migrate: font, wrapping, language,
   window placement and recent files.
4. Plan Paint: import RSPS and `recent-files.bin` once, then keep the old
   files for one release.
5. Games: suite settings and game data. The game saves keep their own
   envelope inside a payload, or move to the store once it has a checksum
   they trust.

## 8. Risks

- **Unconfirmed Windows atomicity.** The POSIX-semantics rename is
  documented for NTFS on Windows 10 1709 and later. Older systems or other
  file systems take the fallback. The last good copy limits the loss.
- **macOS `flock` across network homes.** A lock on an SMB or NFS home may
  not hold. The check against the file's identity and bytes still catches
  most races there. The window between the check and the rename stays open,
  as row 44 says it does for documents.
- **Windows replacement call.** SwiftEdit's `DECISIONS.md` row 40 publishes
  documents with `ReplaceFile`. Accepted decision 2 uses Plan Paint's
  `FileRenameInfoEx`. The two programs then replace files differently. That
  is acceptable, because documents need their ACL and attributes kept and
  settings do not, but the owner may want one call.
- **Moving existing files.** Users of Plan Paint and the games keep their
  settings only if the import runs. It has to be tested against real old
  files (RSPS1 to RSPS8, `playsuite-settings-v1.txt`).
- **Size.** 1 MiB is generous for preferences, but tight if a program puts
  many dock layouts in one file. Layouts go in their own file (`layout`)
  for that reason.

## 9. Decisions for the owner

1. **Several processes of one program** (held in revision 1; section 4.5).
   Every option detects a change by identity, timestamp and exact bytes
   under a lock, and keeps refused changes in memory. They differ in what a
   save does when the file changed.
   - a. Per key, refuse on a real conflict. Another writer's change to a
     key is never overwritten. This process's conflicting change stays in
     memory, is shown as a non-modal choice, and goes to a dot-numbered
     copy at exit. Windows that change different keys both save. The
     canon is applied per key: it departs from "no compare/merge" only in
     treating a settings file as independent values (§4.5.3, point 3).
     Costs a notice in GUI.Forms and a copy rule.
   - b. Per key, the last save wins. Simplest of the keyed rules. For a key
     changed by two windows, the later save silently replaces the earlier
     one, which the canon forbids, even if limited to one key.
   - c. The whole file, refuse and copy. Follows the document rule to the
     letter, without the dialog. With SwiftEdit's independent windows,
     every window after the first writes a copy on every save, and a user
     needs a chooser no program has.
   - d. One file per process, published only when the shared file is
     unchanged. No merge and no conflicts, but most windows' changes never
     reach the next start.

   Recommendation: a. It keeps everything the canon protects (detection by
   bytes, a frozen view, no silent overwrite, copies with a dot number). It
   departs only where settings have no author's text and no dialog. If the
   owner reads "no compare/merge" as covering settings too, then c, with the
   cost stated.
2. **Atomic replace on Windows.**
   - a. `FileRenameInfoEx` with POSIX semantics, falling back to
     `MoveFileExW` write-through, and `atomic_replace` reported per host.
     This is proven in Plan Paint.
   - b. `ReplaceFileW`. Keeps the ACL and attributes and makes a backup in
     one call. Not documented as atomic, fails when the target is missing,
     and needs its own fallback.
   - c. Keep `MoveFileExW` and rely on the last good copy.

   Recommendation: a.
3. **Roaming and local placement.**
   - a. Two stores per program, chosen per file (§4.1). `known_directory`
     gains LocalAppData on Windows. Window placement and recent paths stay
     on the machine they belong to.
   - b. Everything roams. Simpler, but a laptop opens windows sized for the
     desktop's monitors and lists paths that do not exist there.
   - c. Everything local. Preferences do not follow the user between
     Windows machines.

   Recommendation: a.
4. **Checksum and hand edits.**
   - a. BLAKE2s, already in STXlib. A file with a wrong checksum that still
     parses is accepted with a diagnostic (a hand edit). One that does not
     parse goes to the last good copy.
   - b. FNV-1a 64, as the games use. Shorter in the file, but needs a new
     public function.
   - c. Strict: a wrong checksum always means damage. Safest, but hand
     edits are lost.

   Recommendation: a. The hash is already there, and lenient handling keeps
   the games' "edit it by hand" freedom.
5. **A file newer than the program.**
   - a. Read the known keys and never write back while the file is newer,
     with a diagnostic. The newer program's settings survive. The older one
     cannot save until the user decides.
   - b. Read and write back, keeping unknown keys. Saves work, but values
     whose meaning changed in the newer schema may be written back with the
     old meaning.
   - c. Refuse the file and use the defaults.

   Recommendation: a.
6. **Existing settings in Plan Paint and the games.**
   - a. Import once into the identity's folder, then keep the old files for
     one release.
   - b. Leave them where they are and point the store at the old folders.
     No move, but the folders and placement stay inconsistent.
   - c. Start fresh. Users lose their settings.

   Recommendation: a.
7. **Change notification.**
   - a. No watch. Instead `reload_if_changed()`, called when a window is
     activated (§4.6).
   - b. A host file watch in STXlib. Changes appear at once, at the cost of
     three host back ends and a thread.
   - c. Nothing. Each window keeps what it loaded.

   Recommendation: a. A watch can come later without changing the format.
8. **Where it goes before 1.0.**
   - a. STXlib pieces and `stxlib::settings` now, the GUI.Forms layer next,
     SwiftEdit first among the programs.
   - b. After the product releases, together with MCP.
   - c. Only the STXlib fixes (Windows rename, `FileLock`, themes onto
     `known_directory`) before 1.0; the store after.

   Recommendation: a, if the 1.0 schedule allows it. Otherwise c, because
   the Windows rename and the themes fix are worth having alone.
