# Study: MCP built into GUI.Forms

Date: 2026-11-17. Status: study for the owner; nothing below is decided
unless it cites an owner ruling. Code-free: no GUI.Forms code was changed
and nothing was built. Read against `checkpoint/2026-11-15-cp32` (#483),
branch `claude/batch-cp33` at 65f7dad03.

Owner rulings already given for this study (2026-11-17):
- every GUI.Forms program should be drivable and inspectable by a model
  through MCP, "for everything";
- the Emscripten host is not needed; macOS, Windows and Linux X11 are;
- MCP built into the program is preferred to a separate server, as long as
  a model reaches it as easily.

## 1. Summary

GUI.Forms already has almost everything a model needs. It is just not
reachable from outside the process:
- a semantic tree with stable IDs, roles, names, values, states, bounds and
  actions (`SurfaceRoot::semantic_snapshot`, with `to_json`);
- semantic actions, the same path UI Automation and AX use
  (`perform_semantic_action` by stable ID);
- text layout and selection for assistive technology
  (`semantic_text_layout`, `set_semantic_text_selection`);
- one portable input path that every host goes through (`HostSession::
  dispatch(HostEvent)`), native adapters and the surface window manager
  alike;
- captures (`Window::capture_window`, `capture_control`, PNG encoding);
- structured counters (`metrics_snapshot`, `dispatcher_snapshot`,
  `interaction_snapshot`, `visual_inspection_snapshot`), theme provenance
  (`Theme::provenance`), and replaceable time sources (`InputTimeSource`,
  `MotionTimeSource`);
- a retained core where "idle" means something exact: no frames are
  scheduled while nothing changes.

The proposal: a portable **automation core** inside the library that turns
those seams into a small set of requests and replies, plus an **MCP front**
built into the same program. No separate server program is needed:
- **When the model starts the program** (the common case, and the one in
  the Plan Paint bug), the program speaks MCP over its own standard input
  and output. The MCP client (Claude Code, for example) launches it like
  any other MCP server. No port, no file, no password.
- **When the model attaches to a program already running**, the program
  serves MCP over HTTP on 127.0.0.1 only, with a per-run token.

Both are opt-in, and locked three ways (built in, allowed by the program,
asked for at launch). About 9-13 worker units in six stages; the first two
stages (about 4-6 U) would have made the Pencil tab bug a five-line script.

## 2. The motivating case

On 2026-11-17 Plan Paint's selected Pencil ribbon tab drew wrongly when
hovered. To reproduce it, we needed: select the Pencil tab, hover it, wait
about 200 ms, capture, and read the Metrics. That was impossible from
outside:
- Plan Paint's `main` takes only file arguments: no script mode and no
  capture mode;
- GUI.Forms has no portable automation hook. The only channel is the
  Windows host's `WM_COPYDATA` commands (section 4.3), and the X11 host
  accepts `--automation` and ignores it.

The workaround was a probe test inside Plan Paint's AppKit test harness,
using `cliclick` to park the real cursor. It took a coder round trip. Its
first version could not fail, because it never checked the hover plate.

With the hook, the same reproduction is a script any host can run (section
8.2). It fails for the right reason, because it asserts the hover state of
the tab's plate part and compares the presented pixels with the rendered
ones.

## 3. How a model talks to a program through MCP

The owner asked how the communication works. In short:

- **MCP** (Model Context Protocol) is JSON-RPC: small JSON request and reply
  messages. A *client* (the program the model runs in: Claude Code, Claude
  Desktop, an IDE) connects to *servers*. Each server offers **tools**
  (functions the model calls, with a JSON schema for their arguments),
  **resources** (readable data at URIs) and **prompts** (canned
  instructions). The model sees the tool list, decides to call
  `snapshot` or `act`, and the client sends the request and hands the reply
  back to the model.
- **Two standard transports.**
  - *stdio*: the client starts the server as a child process and writes
    requests to its standard input, reading replies from its standard
    output. Configured as a command line, for example
    `claude mcp add plan-paint -- /path/to/PlanPaint --mcp`.
  - *Streamable HTTP*: the server listens at a URL; the client posts
    requests to it. Configured as a URL and headers.
- **The current revision** is 2026-07-28. It made the core stateless (no
  initialise handshake or protocol sessions; every request carries its
  version), and moved long-running work (Tasks) and server-rendered UIs
  (MCP Apps) into extensions. This suits us: each request is complete in
  itself. (Read from secondary sources; check the official text at
  modelcontextprotocol.io before stage M2.)

**What "built in" means here.** A "server" in MCP is just whatever answers
the JSON. Nothing requires a separate program. A GUI.Forms program run with
`--mcp` *is* the MCP server: its window opens as usual, and a thread in the
same process reads requests from standard input and answers on standard
output. To the client it looks exactly like any other stdio server, so a
model reaches it just as easily. This is how the owner's preference can be
met fully:

| Situation | Transport | What the user configures |
|---|---|---|
| The model starts the program | stdio, inside the program | the program's path and `--mcp` |
| The model attaches to a running program | HTTP on 127.0.0.1, inside the program | a URL and a token, printed by the program or written to a per-user file |
| A program that cannot have its own standard input (launched by Finder, a shell association) | the same HTTP endpoint | as above |

A separate bridge program is then optional. It is only worth writing if
clients turn out to handle local HTTP servers badly (section 6.4).

Two consequences for programs:
- **Standard output belongs to MCP in `--mcp` mode.** Anything else printed
  there corrupts the stream. GUI.Forms' own diagnostics already go to
  standard error, except the Windows automation channel's replies (which
  this replaces). A program's own `printf` to standard output must be
  redirected to standard error by the library in this mode (when MCP is
  turned on, at application creation, it duplicates file descriptor 1 for
  MCP and points descriptor 1 at standard error), and the redirection
  counted in the metrics. Output printed before that point is the
  program's own problem, so the library's documentation says so.
- **The program's lifetime belongs to the client in stdio mode.** When the
  client closes standard input, the program receives a portable close
  (`HostShutdownEvent` through every window's session), exactly as on
  logout.

Platform notes:
- macOS: the client must start the executable inside the bundle
  (`PlanPaint.app/Contents/MacOS/PlanPaint --mcp`), not through `open`,
  which drops standard input. AppKit is unaffected.
- Windows: a GUI-subsystem program has no console, but handles inherited
  from the parent work (`GetStdHandle`), which is how clients start stdio
  servers. No console window appears.
- Linux X11: nothing special.

## 4. What exists today

### 4.1 Seams the automation core reads

| Need | Existing seam | Notes |
|---|---|---|
| Windows of the run | `Application::windows()` | at most 64 |
| Control tree | `SurfaceRoot::semantic_snapshot()`, `semantic_generation()` | stable IDs, runtime IDs, roles (55), states (14), actions (10), bounds, text selection; virtual children get hashed runtime IDs |
| Actions | `perform_semantic_action(stable_id, action, value)` and the runtime-ID overload | refuses stale identities (GF017) and controls outside the focus scope; deferred while painting |
| Text | `semantic_text_layout`, `set_semantic_text_selection`, `fill_semantic_text_boundaries` | the A-T1 decisions; same rules on every host |
| Input | `HostSession::dispatch(HostEvent)` | `PointerEvent`, `KeyEvent`, `TextInputEvent`, `HostHoverEvent`, `HostContextRequestEvent`, `DragEvent` and the lifecycle events; validated (sequence, thread, geometry) |
| Single-surface window manager input | `SurfaceInputQueue::post` | covers frames, title strips and the capsule, which are outside any window's session |
| Pixels | `Window::capture_window`, `capture_control`, `encode_window_capture_png` | re-records and replays the window's CPU raster (what the state draws now) |
| Counters | `metrics_snapshot`, `dispatcher_snapshot`, `interaction_snapshot`, `deferred_input_snapshot`, `paint_lease_snapshot`, `validation_snapshot` | structured, as `AGENTS.md` requires |
| Inside a control | `visual_inspection_snapshot` (per-control hovered, pressed, focus cue; the display chunk's paint operations) and `Theme::provenance` (which rule set which field) | beyond anything UIA or AX publish |
| Commands | `Command::stable_id()` | menu and ribbon items bound to commands can be invoked by ID |
| Time | `set_input_time_source`, `MotionClock::set_time_source` | the base for virtual time |
| Schema | the control registry, schema export, `manifests/members.json` | what a role or control can do, for the model's reference |
| Headless runs | `HeadlessHost`: caller-supplied timestamps, no clock, explicit pumping | deterministic CI |

### 4.2 Where every host converges

`HostSession::dispatch` is the one door: the macOS view, the Windows host,
the X11 host and the surface window manager (`surface_manager.cpp`, lines
386-548) all build `HostEvent`s and dispatch them there. The normative
traces (release 1.0 condition 4) check that the hosts agree on what happens
after that door. Injecting there gives identical behaviour on every host
by construction, with no per-adapter code, which is the owner's
requirement.

The one thing past the door that differs is the native window frame: on
macOS, Windows and X11, moving and resizing a window is the platform's
job. The automation core asks for it through the existing host services
(`host_windowing.hpp`), and reports honestly what a host cannot do, as
capabilities already do.

### 4.3 Prior ad hoc channels in the tree

These show the need, and what to retire:
- **Windows `WM_COPYDATA` automation** (`windows_host.cpp`, from line
  3991): `GUI.Forms.Automation/1` text commands (`snapshot`, `capture`,
  `capture-composite`, `close`, `overlay`, a pointer move, `focus <id>`,
  text, keys, `activate <id>`), enabled only when the program calls
  `host::accept_automation_environment()` and `GUI_FORMS_WINDOWS_
  AUTOMATION=1` is set and the build option allows it (A-gf-host-03-F6).
  Replies are printed to standard output. Its reentrancy handling is right
  and should be kept: a command that feeds the model waits while the
  thread is inside a dispatch or a size or move loop, and a modal loop sets
  that count aside (`ModalDispatchDepth`). It lives in a platform adapter,
  so no other host has it.
- `tools/menu_probe.py` and `tools/popup_margin_probe.py` drive that
  channel; `tools/windows_automation_probe.cpp` is a UIA client.
- The X11 Gallery accepts `--automation` and ignores it.
- Plan Paint's AppKit probe harness and `cliclick`.
- The normative trace drivers (one per host) replay a scripted scene.

## 5. The tool surface

### 5.1 What makes a tool set easy for a model to use well

From Playwright MCP, Chrome DevTools MCP and the desktop MCP servers, and
from how models fail with them:
1. **Few verbs, each obvious.** A model chooses better among ten clear
   tools than among fifty. Options go in arguments, not in more tools.
2. **Name things by stable IDs.** Everything the model is shown carries an
   ID it can pass back. GUI.Forms' stable IDs survive between runs, which
   Playwright's per-snapshot refs do not: a script a model writes today
   still works tomorrow.
3. **Readable snapshots, text first.** An indented outline of role, name,
   ID and states is far cheaper and more reliable for a model than a
   screenshot. Pixels are for pixel questions.
4. **Every action replies with what changed.** The semantic diff (nodes
   added, removed or changed; focus moved; windows or popups opened), the
   frames drawn and whether the program is idle. Then the model rarely
   needs a second snapshot, and never acts on a stale picture.
5. **Waiting is built in.** Actions wait until the program is idle before
   replying (Playwright's auto-wait, Espresso's idling resources). A model
   that has to guess sleeps either flakes or wastes time.
6. **Errors say what to do.** A named code, the reason, and a hint: "the
   Save As dialog is modal; act on window `save_as` or press its Cancel
   button (`save_as.cancel`)".
7. **Bounded output.** Snapshots cap their node count and say what they
   left out, with the arguments to see it.
8. **Same answers everywhere.** One host's snapshot equals another's
   (stage A already holds this for the semantic tree).

### 5.2 Tools

Nine tools. Each takes an optional `window` (stable ID or title; default:
the active window).

| Tool | Arguments | Reply |
|---|---|---|
| `windows` | none | every window of the run: ID, title, kind (primary, owned, tool, popup, dialog), modal, active, bounds, minimised |
| `snapshot` | `root` (a stable ID, default the window), `depth`, `filter` (`interactive`, `all`, or a role or name match), `bounds` (yes or no), `format` (`outline` or `json`), `max_nodes` | the outline (5.3), the semantic generation, and what was cut |
| `act` | `target` (stable ID, or a command's stable ID), `action` (`press`, `select`, `expand`, `collapse`, `focus`, `set_value`, `increment`, `decrement`, `show_menu`, `close`, `invoke_command`), `value` | result, diff (5.4) |
| `pointer` | `action` (`move`, `hover`, `click`, `double_click`, `right_click`, `down`, `up`, `drag`, `wheel`), `target` and optional `offset` in its bounds, or `position` in window units; `path` for a drag; `button`, `modifiers`, `delta` | result, diff, the control under the pointer |
| `keyboard` | `keys` (`ctrl+s`, `alt+f4`, `F10`, `tab`, a sequence), or `text` to type committed text, or `composition` | result, diff |
| `text` | `target`, then `read` (value, selection, lines, with offsets), `select` (anchor, caret, or a unit: word, line, all) | the text state; never for protected content (6.5) |
| `wait` | `until`: `frame` (the next presented frame), `idle`, `ms` (real or virtual time), or a condition (`exists`, `gone`, `state`, `name`, `value` on a target); `timeout` | what happened, the diff since the call, the time spent |
| `capture` | `target` (window or control), `kind` (`rendered` or `presented`), `margin`, `scale`, `overlays` | a PNG image plus its logical bounds and scale |
| `inspect` | `target`, `what`: `metrics`, `dispatcher`, `interaction`, `visual` (control state, parts, paint operations), `theme` (the winning rules per part and field), `schema` (what this control can do, from the registry); `since` (a mark, for deltas) | structured records, and a new mark |

Why these and not more:
- Hover, scroll and drag are `pointer` actions, not tools of their own.
- `act` covers menus, ribbons, tabs, trees and combo boxes through the
  roles' own actions. Only gestures that have no semantic action need
  `pointer`, and the reply says which path was used.
- Finding is `snapshot` with a filter. A separate `find` tool would
  duplicate it.
- `inspect` gathers every read-only diagnostic into one tool with a
  selector, so the list stays short. Its `visual` and `theme` parts are
  what no platform accessibility API offers; they answer "is the hover
  plate drawn, and which rule coloured it?" directly.

Two kinds of capture, both needed:
- `rendered` uses `capture_window` or `capture_control`: what the current
  state draws now, freshly recorded.
- `presented` copies the pixels the host last presented (the window's
  raster or the surface window manager's composed surface). It catches
  damage and presentation bugs (stale pixels, a missing repaint after
  hover) that a fresh render hides. The Pencil tab bug may have been
  either kind; the comparison of the two tells which.

### 5.3 The snapshot outline

```text
window "Untitled - Plan Paint" [main] active 1280x800
  ribbon [ribbon]
    tab_group "Ribbon tabs" [ribbon.tabs]
      tab "Home" [ribbon.tab.home]
      tab "Pencil" [ribbon.tab.pencil] selected hovered
    tab_panel "Pencil" [ribbon.panel.pencil]
      group "Width" [pencil.width]
        slider "Width" [pencil.width.slider] value=3 range=1..64
      button "Smooth" [pencil.smooth] checkable checked
  pane "Canvas" [canvas] focusable focused
  status_bar [status]
    static_text "640 x 480 px" [status.size]
```

Rules:
- One node a line: role, quoted name, `[stable ID]`, then the states that
  are set, as words (`enabled` and `visible` are left out unless false), then
  value and range. Bounds only when asked.
- A node without a stable ID shows `[#r412]`, its runtime ID, valid only for
  this run; the reply counts such nodes so a program can be told to name
  them (a coverage figure, 8.4).
- Values longer than 200 characters are cut, with their length.
- `hovered` and `pressed` come from the visual inspection, not the
  semantic tree, and are shown because a model debugging appearance needs
  them.
- `format: json` returns the existing `SemanticSnapshot::to_json` shape.

### 5.4 The reply after an action

```text
ok: press [ribbon.tab.pencil] (semantic action)
changed: [ribbon.tab.pencil] +selected; [ribbon.tab.home] -selected
added: tab_panel "Pencil" [ribbon.panel.pencil] (6 nodes)
removed: tab_panel "Home" [ribbon.panel.home] (11 nodes)
focus: [canvas] -> [ribbon.tab.pencil]
frames: 2 presented; idle after 34 ms
```

The diff is the difference of two semantic snapshots keyed by runtime ID,
taken before the action and after it settles. A dialog or popup that opens
appears as `opened: dialog "Save As" [save_as] modal`.

### 5.5 Errors

Named codes, most taken from existing result enums:

| Code | From | Hint given |
|---|---|---|
| `not_found` | no node with that ID | the closest IDs by name |
| `stale_identity` | GF017 | take a new snapshot |
| `ambiguous` | a stable ID repeated (virtual children) | name the parent |
| `disabled`, `not_visible` | node states | what would enable it, when the tree says |
| `outside_focus_scope` | `SemanticTextResult` and actions | the scope that holds focus |
| `modal_blocked` | the window is blocked by a modal one | the modal window's ID and its buttons |
| `unsupported_action` | the node's `actions` | the actions it has |
| `off_boundary` | text selection | the nearest grapheme boundaries |
| `host_cannot` | a host capability missing | which capability |
| `timeout` | `wait` | what was still busy (dispatcher work, motion, timers) |
| `protected` | protected content | none |

`during_paint` is never returned: the core retries at the next turn.

### 5.6 Resources and prompts

Tools are the main surface, because every client supports them and many
handle resources poorly. Resources mirror the read-only parts for clients
that use them:
- `gui-forms://windows`, `gui-forms://window/{id}/snapshot`,
  `gui-forms://window/{id}/metrics`, `gui-forms://window/{id}/capture.png`;
- `gui-forms://schema/controls` and `gui-forms://schema/members` (the
  registry and the member manifest, so the model can look up what a
  `property_grid` or a ribbon can do);
- change notifications for a window's snapshot keyed on its semantic
  generation, where the client subscribes.

Prompts are cheap and modestly useful. Three to start:
- *reproduce a visual bug* (snapshot, act, wait idle, capture both kinds,
  inspect `visual` and `theme`);
- *explore this program* (windows, snapshot interactive, list commands);
- *check accessibility of this window* (nodes without names or stable IDs,
  actions without keyboard paths, contrast from `theme`).

A program may later add its own tools and resources (Plan Paint's layers,
SwiftEdit's buffers) through a registration table (stage M5).

## 6. Architecture

### 6.1 Layers

```text
MCP client (Claude Code, ...)
   │  stdio, or HTTP on 127.0.0.1
   ▼
MCP front (optional library gui_forms_mcp; its own thread)
   │  JSON-RPC <-> automation requests; tool schemas; PNG and text replies
   ▼
Automation core (in gui_forms; UI thread only)
   │  requests and replies as named records; the wait machine; diffs
   ├──► semantic snapshots and actions   (SurfaceRoot)
   ├──► synthetic HostEvents              (HostSession, every host)
   ├──► surface input records             (SurfaceInputQueue, surface hosts)
   ├──► captures, metrics, inspection     (Window, Theme)
   └──► time sources                       (virtual time, opt-in)
```

- **Automation core**: portable C++ under `src/core/automation/` with a
  small public header for policy and program registration. Requests and
  replies are records (`AutomationRequest`, `AutomationReply` with an
  explicit result enum), not JSON, so the core is testable without MCP
  and usable by scripts and the C ABI. It touches the model only on the UI
  thread, at a dispatcher turn (house style 5: one execution context per
  piece of mutable state).
- **MCP front**: JSON in and out (the existing `src/core/json` reader;
  writing is simple), the tool schemas, the transports. Its own thread
  reads requests and posts each into the target window's dispatcher
  (`WindowDispatcher`, which any thread may hold), then waits for the
  reply. It holds nothing taken from model state across the post. A
  separate library keeps JSON-RPC and sockets out of programs that never
  enable it.
- **No platform code** beyond what transports need (a loopback socket and
  file permissions in `src/surface/posix/` and `src/surface/windows/`, per
  the host consolidation rule). The core includes no platform header.

### 6.2 Input injection without per-host code

- **Semantic actions first.** `act` uses `perform_semantic_action`, which
  is what assistive technology calls. It needs no coordinates and does not
  depend on layout or the pointer.
- **Synthetic input second.** `pointer` and `keyboard` build `HostEvent`s
  and dispatch them through the window's `HostSession`, the same door as
  the native adapters. On a single-surface host they go into the
  `SurfaceInputQueue` instead, so the window manager's own frames and
  capsule respond as they do to real input. Sequence numbers and
  timestamps come from the session's own counters, so the session's
  validation still applies.
- **The real cursor.** A real mouse on a native host keeps sending moves
  and leaves, which would undo a synthetic hover (the reason the Plan Paint
  probe needed `cliclick`). Proposal: a **pointer lease**. While a session
  holds it for a window, `HostSession` drops pointer events that come from
  the adapter for that window (counted, so a test can see them), and only
  synthetic ones reach the model. This lives in `HostSession`, so no
  adapter changes. Keyboard input from the real user is not dropped.
  The lease ends with the session, a `release` argument, or a timeout.
- **Hover timing.** Hover plates, tooltips and menu delays depend on time.
  By default `wait ms=200` waits 200 real milliseconds. With **virtual
  time** (opt-in, decision 4) the core installs its own input and motion
  time sources and advances them, so a 200 ms hover is exact and instant.
  Timers and the scheduler must read the same source; whether they do today
  needs checking at M4 (the window's timers were not traced for this
  study).

### 6.3 Idle, frames and the wait machine

GUI.Forms' "idle schedules zero frames" makes idle exact. A window is idle
when all hold:
- no posted dispatcher work (`dispatcher_snapshot().pending == 0`);
- no pending layout or paint, and no paint wake pending;
- the motion clock asks for no frames (no animation running);
- no present outstanding (a surface host's `presented` record has come back);
- no timer due within a small window (default 50 ms; a running caret blink
  does not count);
- no automation-visible work registered by the program (an idling token a
  program takes around its own background jobs, like Espresso's idling
  resources; background workers can take one automatically).

Each request is a small state machine with a named completion:

```text
queued ─► dispatched ─► settling ─► replied
                 └────► failed (named code)
```

- *queued*: posted to the window's dispatcher;
- *dispatched*: the action ran (or was refused);
- *settling*: waiting for idle, at any loop depth, up to the timeout;
- *replied*: the diff is taken and the reply sent.

### 6.4 Modal loops and reentrancy (house style 4 and 5)

The hard case. Pressing "Save As" runs a handler that enters a nested modal
loop and does not return until the dialog closes. An automation request
that waited for the action's call to return would hang.

Rules:
1. The core never waits for an action's call to return. It waits for
   *idle*, which a nested loop also reaches once the dialog is shown and
   settled. The reply then reports `opened: dialog [save_as] modal`, while
   the handler is still on the stack.
2. Later requests run inside the nested loop, at its dispatcher turns. A
   request for the modal window runs; one for a window it blocks gets
   `modal_blocked`, naming the modal window.
3. A request never runs inside a dispatch into model code. It waits for the
   next turn, as the Windows channel already does (`host_dispatch_depth`),
   and a modal loop sets the depth aside (`ModalDispatchDepth`). The rule
   moves from the Windows adapter into the core, and every host gets it.
4. Completing a change before notifying: the reply is built after the
   action's state change has settled, and the front copies what it sends.
   Nothing taken from model state is carried across a call into
   application code.
5. Revocation: when a window closes, requests for it fail with
   `stale_identity` (its runtime ID is never reused). When the session ends,
   leases and virtual time are given back, and queued requests are
   cancelled with a named result.
6. Live resize and move loops (Windows' size-move loop, AppKit's live
   resize) count as busy, as today on Windows.

**Native dialogs and menus.** NSOpenPanel and IFileDialog are not
GUI.Forms trees, and no tool can see inside them. While a session holds a
*dialog lease*, `HostServices` dialog requests (open, save, folder, colour)
are answered by the session instead: the reply to the action reports
`dialog_requested: save` and a later `act target=host.dialog
action=set_value value=<path>` answers it. The surface window manager
already has this shape (`SurfaceInputKind::dialog` with an outcome).
Paths answered this way must lie under a folder the session was given at
launch (6.5). The macOS application menu items that are bound to
`Command`s are reached by `invoke_command`. Popups, menus and drop-downs
that GUI.Forms draws itself (warm Popup windows) are ordinary windows in
`windows` and fully drivable.

### 6.5 Safety

**Three locks** (the Windows channel's model, kept):
1. *Built in*: build option `GUI_FORMS_AUTOMATION` (decision 1 sets its
   default for products).
2. *Allowed by the program*: an application option
   (`ApplicationOptions::automation`: `refuse`, `allow_when_requested`).
   Default `refuse`, so a program that never thought about it stays closed.
3. *Asked for at launch*: `--mcp` (stdio) or `--mcp-listen` (HTTP) on the
   command line, or `GUI_FORMS_MCP=stdio|listen`. The library removes the
   argument before the program sees `argv`.

**Local only.**
- stdio needs no network and no authentication: only the process that
  started the program holds its pipes.
- HTTP binds 127.0.0.1 (and ::1) only, never 0.0.0.0, on a random port.
  A 256-bit token is generated per run and written with the port to a file
  readable only by the user (`$XDG_RUNTIME_DIR/gui-forms/<pid>.json`,
  `~/Library/Caches/...` on macOS, `%LOCALAPPDATA%` with a user-only ACL on
  Windows). Every request must carry it; compared in constant time.
  Requests whose `Origin` header names a web page are refused, and the
  `Host` header must be the loopback address, which defeats DNS rebinding
  from a browser. No CORS.

**A visible sign.** While a session is connected, every window shows it (a
title suffix, or a tinted capsule on surface hosts), as Chrome shows
"controlled by automated test software". Recommended in product builds;
optional in development builds (decision 9).

**Never exposed:**
- values of nodes with `protected_content` (passwords). Typing into them is
  allowed; reading answers `protected`, and captures of a window that
  holds one mask its bounds;
- controls and windows a program marks `automation_hidden` (a licence key
  panel);
- files: no tool reads or writes files; dialog answers are limited to a
  folder named at launch;
- other processes' windows or the screen: captures are of GUI.Forms' own
  rasters only, unlike screen-capture servers;
- the clipboard, unless the program allows it;
- arbitrary code: no evaluate, no property setter that bypasses what a user
  can do; inspection is read-only.

Bounded everything: request size, queued requests (64, as the Windows
channel), snapshot nodes, capture pixels.

Text read from a program (a document's contents) is data, not instructions
to the model. The outline puts values in quotes and labels them, and the
tool descriptions say so; the rest is the client's defence against prompt
injection.

### 6.6 The C ABI and the C# sibling

- The core lives under `Application`, so programs on the C ABI (PlaySuite
  and SwiftEdit, after their ports) get it without any ABI addition. Only
  policy and registration need ABI:
  - one entry in the application table: the policy (`refuse`,
    `allow_when_requested`), the hidden marking, the idling token;
  - later (M5), a registration table for a program's own tools: name,
    JSON schema, and a callback on the UI thread that returns a JSON reply
    (the house style's named target with explicit context, revoked when the
    application ends).
  These are additive: an ABI minor version, with a class-table and
  manifest entry like any other.
- The C# sibling exposes the same policy through its `Application`. In
  mirrored mode (the swap-in launcher, roadmap 5.5) a model could then drive
  a real WinForms program such as SDR# running on GUI.Forms, and read the
  unsupported-member log through `inspect`. That is strong dogfooding.

### 6.7 Relation to UIA, MSAA, AX and AT-SPI

Why not use the existing desktop MCP servers over the platform APIs:
- each platform differs, while the owner requires identical behaviour;
- the platforms publish no part states, no theme provenance, no metrics, no
  frame or idle signal, no render capture, no virtual time;
- macOS AX clients need the user's accessibility permission, and AX reads
  are slow and need settling (#249, #266, #275);
- they poll and guess, and coordinate clicks are fragile.

The native publication stays the source of truth for assistive
technology, and the automation core reads the same semantic tree, so the
two cannot drift in content. A cross-check tool (M6) compares, on each
host, the platform API's view with the core's snapshot: stage A's client
tests, made available on demand.

## 7. Prior art: what to copy and what to avoid

| Source | Copy | Avoid |
|---|---|---|
| **Chrome DevTools Protocol** | domains and versioning; events; `Accessibility.getFullAXTree`; `Input.dispatch*` going through the real input path; `Page.captureScreenshot`; `Emulation.setVirtualTimePolicy` (virtual time) | its size: hundreds of methods no model can choose among |
| **Playwright** | auto-wait and actionability checks (visible, stable, enabled, receives events); locators by role and name; ARIA snapshots; the trace viewer (every step's snapshot and pixels) | CSS and XPath selectors; flaky waits on timeouts |
| **Playwright MCP** | text snapshots with refs as the main view; a reply that carries the new state | refs that die with each snapshot (ours are stable) |
| **Chrome DevTools MCP** | performance traces and console as first-class tools (our `inspect`) | depending on a browser |
| **Appium, WinAppDriver, W3C WebDriver** | sessions, element handles, explicit and implicit waits; a pattern vocabulary | chatty HTTP per element; stale-element exceptions; WinAppDriver is unmaintained, so do not build on a platform's automation stack |
| **UIA and AX** | the control patterns (Invoke, Toggle, Selection, ExpandCollapse, Value, Text), which `SemanticAction` already mirrors | polling; host differences; AX permission prompts |
| **Espresso and XCUITest** | synchronisation with the UI thread's idle (idling resources); `waitForExistence` | Android or Apple specifics |
| **Flutter (semantics tree, `flutter_driver`, the Dart and Flutter MCP server)** | the closest analogue: a toolkit that owns its pixels exposing its own tree, in process | a VM service reachable over the network |
| **Squish and Qt Test** | object names as stable IDs; scripts that run on every platform; record and replay | commercial and per-toolkit tooling outside the program |
| **Desktop MCP servers (Windows-MCP and macOS ones)** | they show models can use such tools | screenshots plus coordinate clicks as the main path: slow, fragile, and blind to state |

## 8. Testing and use

### 8.1 One command set, three users

- **A model through MCP**, interactively.
- **A script**: the same requests as JSON lines, run by the program itself
  (`--automation-script file.jsonl`, writing replies and captures next to
  it) or in process by a ctest harness on the headless host. No model, no
  client, deterministic with virtual time.
- **Recorded sessions**: every request and reply of an MCP session can be
  saved as a script, so a model's exploration that found a bug becomes the
  regression test, after a person adds the assertion.

Assertions are one more request kind in scripts (`expect`: a node's state,
name or value; a part's state; a capture against a golden or against the
other capture kind with a tolerance; a metric within bounds). Models
assert by reading replies.

### 8.2 The Pencil tab, as a script

```text
{"tool":"act","target":"ribbon.tab.pencil","action":"select"}
{"tool":"pointer","action":"hover","target":"ribbon.tab.pencil"}
{"tool":"wait","until":"ms","ms":200,"clock":"virtual"}
{"tool":"wait","until":"idle"}
{"expect":"visual","target":"ribbon.tab.pencil","part":"plate","state":"hover","drawn":true}
{"expect":"capture","target":"ribbon.tab.pencil","compare":"presented=rendered","tolerance":0}
{"tool":"inspect","target":"ribbon.tab.pencil","what":["theme","metrics"]}
```

It runs unchanged on macOS, Windows, X11 and the headless host. It can
fail: the plate's hover state is asserted, and a damage bug shows as a
difference between presented and rendered pixels. `inspect theme` names
the rule that painted the plate.

### 8.3 What it replaces

- the Windows `WM_COPYDATA` channel, `menu_probe.py` and
  `popup_margin_probe.py` (retired after M3, decision 8);
- Plan Paint's AppKit probe harness and `cliclick`;
- most of the per-host normative trace drivers, whose scenes become
  scripts (the trace comparison itself stays);
- part of the products' own UI tests, which today reach into internals.

### 8.4 Checks it makes possible

- **Stable ID coverage** per program: nodes without stable IDs, as a
  number in the conformance suite (useful to the products before 1.0);
- **Keyboard reachability**: every action reachable by keys alone;
- **Snapshot equality across hosts**: the same scene's outline on every
  host, a cheaper companion to the normative traces.

## 9. Plan

Sizes in worker units (U), as in the roadmap. The web host is excluded
(owner, 2026-11-17).

| Stage | Contents | Depends on | Kind | U |
|---|---|---|---|---|
| M0 | This study; owner decisions | none | S+O | done when accepted |
| M1 | Automation core on the headless host: requests and replies as records, outline and JSON snapshots, diffs, `act`, synthetic input through `HostSession`, the wait machine and idle, `rendered` capture, `inspect` metrics, dispatcher and interaction; the script runner and `expect`; tests | M0 | C+B | 2-3 |
| M2 | MCP front built in: stdio and loopback HTTP, the three locks, the token file, the sign while connected, standard output redirection, tool schemas, resources; `presented` capture; the pointer lease; on macOS, Windows and X11 | M1 | C+B+M+W | 2-3 |
| M3 | Modal loops and reentrancy rules moved into the core; popups, menus, the ribbon, owned windows; the dialog lease; `invoke_command`; the surface window manager path; retire the Windows channel and its probes | M2 | C+B | 1.5-2 |
| M4 | Inside controls: `visual` (parts and part states), `theme` provenance, metric marks and deltas; virtual time (and the timer and scheduler check) | M1 | C+B | 1.5-2 |
| M5 | C ABI table (policy, hidden marking, idling token, program tools), C# binding, the products' own tools (Plan Paint first) | M2, the ABI audit | C+B+W | 1.5-2 |
| M6 | Native cross-check against UIA, AX and AT-SPI; stable ID coverage in conformance; prompts | M2 | C+B | 1 |

Total about 9-13 U. Verification needs each host (M, W and the X11 VM), as
usual.

**Fit with the programme.** The current priority is landing #483 and
releasing the products, so nothing here starts before that. Afterwards:
- M1 and M2 are worth doing early, before the conformance rebases of the
  products (roadmap sequence step 6). They give every product a way to
  reproduce bugs like the Pencil tab, and the coordinator and owner a way
  to check builds without hand-written probes.
- M5 adds to the ABI, so it is additive and needs a minor version. If
  automation should be part of 1.0 (backend), the table must land before
  the ABI is sealed for 1.0; otherwise it is a 1.x minor.
- It fits "accessible by construction" in the programme's first page: a
  model is one more kind of assistive user, and the same tree serves both.

## 10. Open decisions for the owner

1. **Product release builds.** (a) Compiled out of products, development
   builds only; (b) compiled in but locked, so a user can start a product
   with `--mcp` to have a model operate it for them (an accessibility use);
   (c) compiled in and allowed by every program by default.
   *Recommendation: (b)*, with the program lock defaulting to refuse and
   each product choosing; it serves "for everything" without opening
   anything by default.
2. **Transport.** (a) Built in: stdio when the client starts the program,
   loopback HTTP with a token to attach; (b) also a separate stdio bridge
   program; (c) stdio only. *Recommendation: (a)*, matching the owner's
   preference; add (b) only if clients handle local HTTP badly.
3. **The real pointer.** (a) A pointer lease that drops the adapter's
   pointer events for the window while a session holds it; (b) mix real and
   synthetic input. *Recommendation: (a)*.
4. **Virtual time.** (a) Headless and scripts only; (b) also on demand in
   live sessions (`clock: virtual`), animations then advance only when the
   model waits. *Recommendation: (b)*, off by default.
5. **Inspection beyond accessibility** (`visual`, `theme`, paint
   operations) in product builds, or only in development builds.
   *Recommendation: in both*, behind the same locks; it is read-only.
6. **Program tools through the C ABI** (M5): now, or after 1.0.
   *Recommendation: the policy entry before the 1.0 ABI seal, program tools
   in the same table if M5 is reached in time.*
7. **Where the MCP front lives:** a separate optional library
   (`gui_forms_mcp`) beside the core in `gui_forms`, or all in one.
   *Recommendation: separate*, so programs that never enable it carry no
   JSON-RPC or socket code.
8. **Retire the Windows `WM_COPYDATA` channel** and the X11 `--automation`
   flag once M3 lands. *Recommendation: yes.*
9. **A visible sign while connected:** in product builds, development
   builds, or both. *Recommendation: both*, with an option to turn it off in
   development builds.
10. **Clipboard access:** never, or when the program allows it.
    *Recommendation: when the program allows it.*
11. **Placement:** M1 and M2 right after the product releases, ahead of the
    conformance rebases, or after 1.0 (backend). *Recommendation: right
    after the releases.*

## Sources

Read in the repository: `AGENTS.md`; `src/host/surface/README.md`;
`src/host/surface/input/surface_input.hpp`; `src/host/web/README.md` (for
the window manager's input path); `include/gui_forms/semantics/types/
semantic_types.hpp`; `include/gui_forms/surface_root/surface_root.hpp`;
`include/gui_forms/host/types/host_types.hpp`; `src/host/headless/session/
headless_host.hpp`; `src/host/surface/manager/surface_manager.cpp`;
`src/surface/windows/application/windows_host.cpp` (the automation channel);
`include/gui_forms/visual_inspection/types/visual_inspection_types.hpp`;
`tools/normative_trace.py`, `tools/menu_probe.py`; `CMakeLists.txt`
(`GUI_FORMS_WINDOWS_AUTOMATION_ENVIRONMENT`). On this branch:
`planning/FORWARD_PROGRAMME.md`, `planning/ROADMAP_TO_1.0.md`,
`planning/stages/README.md`, `planning/stages/release-1.0/README.md`.

Outside: the MCP 2026-07-28 revision as summarised by
blog.modelcontextprotocol.io and secondary write-ups (to confirm against
modelcontextprotocol.io/specification/2026-07-28 before M2); Playwright
MCP's documentation on accessibility snapshots (playwright.dev/mcp); the
Chrome DevTools MCP comparisons. The other prior art in section 7 is from
general knowledge of those projects and was not re-read for this study.
