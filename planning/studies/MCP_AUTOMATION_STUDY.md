# Study: MCP built into every GUI.Forms program

Date: 2026-11-17, revision 3 (answers and extensions added 2026-11-18). Status: the owner's rulings of 2026-11-17
are recorded in section 1 and the design follows them. Section 17 records
the answers of 2026-11-18 and section 18 the owner's extensions. Code-free: no GUI.Forms or STXlib code was changed
and nothing was built. Read against `checkpoint/2026-11-15-cp32` (#483),
branch `claude/batch-cp33` at 65f7dad03.

Revision history:
- 1 (`d1635cb`): first study: an automation core, nine tools, stdio and
  loopback HTTP, a pointer lease.
- 2 (`ceb0891`): the MCP programs on this computer; how Anthropic, OpenAI
  and Apple do it.
- 3 (this one): rewritten after the owner's review ("frankly, its
  terrible"): anonymous pipes only, `stx_mcp` in STXlib, first come first
  served input, `model.directed`, the border glint, the destructive-act
  question. The transports, locks and leases of revisions 1 and 2 are
  withdrawn.

## 1. Owner rulings, 2026-11-17

**Scope**
1. Every GUI.Forms program can be inspected and driven by a model through
   MCP, "for everything". Hosts: macOS, Windows and Linux X11. The
   Emscripten host is not needed.
2. It goes in before the 1.0 freeze, right after the product releases, but
   not this week.

**Build and code**
3. MCP is a capability in each program's build configuration. It
   **defaults to on**, and a program can turn it off.
4. Each program has an MCP identity of the form `app.<vendor>.<appname>`
   by default. Ours look like `app.malkuthsoft.planpaint`.
5. GUI.Forms is designed for it, but the MCP code lives in **STXlib**, as
   **`stx_mcp`**.
6. It is written in C++ but behaves somewhat like Go.
7. rpclib is ported forward, or used as a guide.

**Transport**
8. **Anonymous pipes only.** The client starts the program and the two
   talk over the pipes the program inherits. There is no other transport:
   no sockets, no HTTP, no attaching to a program already running.
9. No sign-in.

**Input and control**
10. **First come, first served.** A model is treated like a person. The
    real mouse and keyboard are never ignored by default: whoever clicks,
    types or activates first is first.
11. **Debug and special builds only:** a chain of commands may suspend user
    input, and input comes back on its own when the chain completes.
    Release builds cannot normally block user input.
12. **`model.directed`** (any build): a program launched with this option,
    on the command line or in its configuration, accepts an instruction in
    the command sequence that briefly makes the interface ignore user input.
    A popup then asks whether the user wants to take back control. Only the
    popup can be clicked. Taking control back interrupts the model's work.
    When the model's work completes, input comes back and the popup closes
    by itself.

**Safety, following Anthropic's lead**
13. Password field focus is never exposed.
14. While a dialog is open, the model is refused control. A program can
    instrument a dialog, such as a setup dialog in which a model guides a
    user through an install. The model may start it and drive it up to the
    point where a security approval is needed; there the user takes back
    control briefly.
15. Consent per program and client is approved, and it is built in: the
    user can take control back at any time.
16. Read-only capability is marked as read-only.
17. **Destructive capability** asks once per launch, in the program: "Allow
    <model> to edit files and perform potentially destructive acts?" This
    is independent of the client's own approvals (claude.ai's harness, for
    example).
18. Developers decide what is destructive. By default every act is
    treated as non-destructive, and there is a way to mark acts
    destructive.

**What the user sees**
19. While a model is connected, the window uses the application border
    capability (begun, not finished) with an animated glint that shows the
    model refreshing its picture of the program or acting on it.
20. A small tag line tooltip on the top border names the model now
    controlling the program.

**Other answers**
21. Inspection beyond accessibility (parts, part states, theme rules,
    metrics) is exposed.
22. Batching is approved.
23. The existing automation flags are retired in favour of this (the
    Windows `WM_COPYDATA` channel and the X11 `--automation` flag).
24. Clipboard: as recommended, readable only when the program allows it.
25. The owner does not know what simulated time means; section 12
    explains it, and it is open.
26. On Apple's design: "the most terrible of all." Xcode's is the model to
    avoid (section 13.2).

## 2. Summary

- A model's client (Claude Code, for example) starts a GUI.Forms program
  with `--mcp` and talks to it over the program's standard input and
  output, which are anonymous pipes the client created. The program opens
  its windows as usual; a reader in the same process answers MCP requests.
  This is how most MCP servers already work, so any client reaches it
  with one line of configuration.
- The protocol code is `stx_mcp` in STXlib: JSON-RPC framing over pipes, a
  method dispatcher in rpclib's manner, tool schemas, and a small
  Go-like runtime of tasks, channels and request contexts, with errors as
  values.
- GUI.Forms supplies the content: an **automation core** that reads the
  semantic tree, acts through the same path assistive technology uses,
  injects input through the one portable input door every host shares,
  waits for idle, captures pixels and reads metrics.
- Model and person are equals by default. `model.directed` lets a model
  hold the interface briefly, with a "take back control" popup always
  available.
- The window's border glints while a model works, and a tag line names the
  model.
- About 10-14 worker units, in stages that start after the product
  releases (section 16).

## 3. The motivating case

On 2026-11-17 Plan Paint's selected Pencil ribbon tab drew wrongly when
hovered. To reproduce it, we needed to select the Pencil tab, hover it,
wait about 200 ms, capture, and read the Metrics. That was impossible from
outside:
- Plan Paint's `main` takes only file arguments;
- GUI.Forms had only a Windows-only automation channel, and the X11 host
  ignores its `--automation` flag.

The workaround was a probe test inside Plan Paint's AppKit test harness,
with `cliclick` parking the real cursor. Its first version could not fail,
because it never checked the hover plate.

With this design the reproduction is one `batch` request (section 15.1),
on any host. It asserts the plate's hover state and compares the presented
pixels with freshly rendered ones, so it fails for the right reason.

## 4. How the connection works

### 4.1 Anonymous pipes

An anonymous pipe is a one-way byte channel with no name. A process
creates it, and only processes that inherit one of its ends can use it.
When an MCP client starts a server, it creates two pipes and starts the
program with one end of each as the program's standard input and standard
output. Requests flow in on one, replies flow out on the other. This is
MCP's "stdio" transport.

What follows from it:
- **Only the process that started the program can talk to it.** Nothing
  is listening, so there is no port to find, no file to protect and no
  password to check. That is why no sign-in is needed (ruling 9).
- **The program's lifetime is the client's.** When the client closes the
  pipe, the program receives the portable shutdown (`HostShutdownEvent`
  through every window's session), as on logout.
- **No attaching to a program already running** (ruling 8). A program
  opened from Finder, a file association or a shortcut has no MCP. To use
  a model with it, the client starts it.
- It works the same on all three hosts:
  - macOS: the client starts the executable inside the bundle
    (`PlanPaint.app/Contents/MacOS/PlanPaint --mcp`); `open` would drop the
    pipes;
  - Windows: a GUI-subsystem program has no console, but inherited pipe
    handles work (`GetStdHandle`), and no console window appears;
  - Linux X11: nothing special.

### 4.2 Starting MCP

The build option makes the capability present; a launch argument turns it
on for that run:
- `--mcp` (or the environment variable `GUI_FORMS_MCP=1`): standard input
  and output carry MCP for this run. The library removes the argument
  before the program sees `argv`.
- Without it, the program behaves exactly as without MCP: nothing reads
  standard input, and no thread is started.

A launch argument is still needed because a program started normally also
has a standard input (a terminal, or nothing), and must not treat it as
MCP. The name of the argument is open (question 1).

**Standard output belongs to MCP.** Anything else printed there breaks
the stream. When `--mcp` is on, `stx_mcp` duplicates file descriptor 1 for
its own use, then points descriptor 1 at standard error, so a program's
`printf` lands in the error stream. The redirection is counted.

### 4.3 Identity

Each program has an MCP identity, by default `app.<vendor>.<appname>`
(ruling 4), taken from the application's vendor and name:
`app.malkuthsoft.planpaint`, `app.malkuthsoft.playsuite`,
`app.malkuthsoft.swiftedit`. A program may set its own. The identity is
the server name in MCP replies and the name a client gives the server by
default. Client configuration is then one line, for example:

```text
claude mcp add app.malkuthsoft.planpaint -- "/Applications/Plan Paint.app/Contents/MacOS/PlanPaint" --mcp
```

### 4.4 Which model is connected

MCP clients state their name and version with their requests (the
2026-07-28 revision carries client information with each request, since it
dropped sessions). That is what the tag line shows (section 11.2). It is
the client's own statement, not verified; with anonymous pipes the client
is whoever started the program, which is enough for a label. When the
client also names the model in the request's metadata, the tag line shows
the model: "Claude Opus 5.5 via Claude Code". Otherwise it shows the
client.

## 5. Where the code lives

```text
STXlib ── stx_mcp                          (the protocol and its runtime)
   ▲ pinned and installed by stx
stx    ── pins STXlib                       (as today)
   ▲ pinned by gui_forms (stx.lock)
GUI.Forms ── automation core               (src/core/automation/, portable)
          ── MCP wiring                     (tools, identity, the build option)
          ── the window's visible signs     (border glint, tag line, popups)
programs  ── a build option, defaults on   (GUI_FORMS_MCP)
```

- **`stx_mcp` in STXlib** (ruling 5): JSON-RPC 2.0 framing over pipes, the
  MCP methods (tool listing and calls, resources, prompts, notifications),
  schemas and their validation, the dispatcher, and the Go-like runtime.
  It knows nothing of GUI.Forms; any STXlib program could serve MCP with it.
  STXlib's rules apply: no exceptions in release builds, failures as data,
  no `std::` in public headers. Those rules already match the Go style the
  owner wants (section 6).
- **The automation core in GUI.Forms**: portable C++ under
  `src/core/automation/`. It turns tool calls into actions on windows,
  only on each window's UI thread. It holds the input policy (section 8),
  the dialog and security rules (section 9) and the destructive-act rule
  (section 10).
- **The MCP wiring in GUI.Forms**: binds the core's operations as
  `stx_mcp` tools, sets the identity, reads `--mcp` and `model.directed`.
- **The build option** `GUI_FORMS_MCP`, on by default (ruling 3). Off
  removes the wiring and the core from the program, and `stx_mcp` is then
  not linked.
- Platform code: none in GUI.Forms. The pipes are standard input and
  output, which STXlib's platform layer already reaches.

Coordination: `stx_mcp` is STXlib work, so it goes through STXlib's
coordinator and integrator and reaches GUI.Forms through the stx pin, like
every other STXlib piece.

## 6. `stx_mcp`: C++ that behaves like Go

### 6.1 What rpclib gives

rpclib (github.com/rpclib/rpclib, MIT) is a small C++14 library for
msgpack-RPC over TCP, built on asio. Its strengths are what make it worth
following:
- `server.bind("name", function)`: a function is published under a name,
  and its argument types are read from its signature, so arguments are
  unpacked and checked automatically;
- one dispatcher that looks up the name and calls the function;
- `run()` on the calling thread or `async_run(n)` on worker threads;
- a client with blocking calls and results converted on request.

It no longer fits as it is:
- msgpack, not JSON: MCP is JSON-RPC;
- TCP through asio, not pipes;
- errors as C++ exceptions;
- lambdas and type erasure everywhere, which the house style forbids
  (sections 7, 9 and 11);
- unmaintained: the project is "looking for maintainers", and its own page
  points users who need JSON-RPC elsewhere;
- it depends on msgpack-c, asio and fmt, while STXlib carries no
  third-party code.

So `stx_mcp` follows rpclib's shape and replaces its parts:

| rpclib | `stx_mcp` |
|---|---|
| `bind(name, callable)` with signature reading | `bind(ToolSpec, handler, context)`: a named handler (a function taking a context pointer, a request record and a reply record), its argument and result schema declared in `ToolSpec`, and the arguments decoded into a named record before the handler runs. No lambdas, no `std::function`. |
| msgpack | JSON (STXlib's reader and writer, or `stx_mcp`'s own if STXlib has none) |
| TCP and asio | two pipe ends; newline-delimited JSON-RPC messages, bounded in size |
| exceptions | results as values (6.3) |
| worker threads | tasks and channels (6.2) |

### 6.2 Go's shape in C++

What "somewhat like Go" means here, within the house style (which forbids
coroutines, lambdas and `auto`):
- **Tasks**: a few long-lived threads with one job each, like goroutines:
  a reader (decodes requests from the pipe), a writer (encodes replies),
  and the handlers. Each owns its own state; nothing is shared but
  channels.
- **Channels**: bounded queues of records between tasks, with `send`,
  `receive`, `try_receive`, `close`, and `select` over several channels
  with a deadline. Closing a channel is how shutdown spreads, as in Go.
- **Contexts**: every request carries a context, like Go's `context.Context`:
  a deadline, a cancellation flag the client can set (MCP's cancellation
  notification), and the request's identity. Handlers check it; waits honour
  it.
- **Errors as values**: every call returns its result and a named error,
  like Go's `value, err`. MCP's own error codes are values of the same
  kind.
- **One owner per state**: a GUI.Forms window's state is touched only on its
  UI thread. A handler that needs it sends a request record to the window
  (through the window's dispatcher) and waits on a reply channel with the
  request's deadline.

```text
pipe in ─► reader task ─► requests channel ─► handler tasks ─┬─► window dispatcher (UI thread)
                                                             │        └─► reply channel ─┐
pipe out ◄─ writer task ◄─ replies channel ◄─────────────────┴──────────────────────────┘
```

Every arrow is a bounded channel. A full channel makes its sender wait or,
for notifications, drops the oldest and counts it. Shutdown closes the
requests channel; each task finishes what it holds and closes the next
channel, and the writer closes the pipe last.

### 6.3 Failures

- A malformed message: a JSON-RPC parse or invalid-request error, and the
  stream continues.
- A message over the size bound: refused, and the reader skips to the next
  line.
- A handler error: a tool result marked as an error, with the named code
  and a hint (section 7.5), so the model can recover.
- The pipe closes: every task stops, pending requests are cancelled with a
  named result, and the program receives the portable shutdown.
- Writes to a broken pipe never raise a signal (`SIGPIPE` ignored for that
  descriptor).

## 7. What a model gets: the tools

### 7.1 What makes tools easy for a model

1. Few verbs, with options as arguments.
2. Everything named by GUI.Forms' stable IDs, which survive between runs.
3. Text outlines first; pixels only for pixel questions.
4. Every action replies with what changed (a diff) and waits for idle.
5. Errors that say what to do next.
6. Bounded output that says what it left out.
7. The same answers on every host.

### 7.2 Tools

Each takes an optional `window` (stable ID or title; default the active
window).

| Tool | Marked | What it does |
|---|---|---|
| `windows` | read-only | every window: ID, title, kind, modal, active, bounds |
| `snapshot` | read-only | the control tree as an outline (7.3), from a root, with a filter and a node limit |
| `act` | per target (section 10) | a semantic action on a stable ID (`press`, `select`, `expand`, `collapse`, `focus`, `set_value`, `increment`, `decrement`, `show_menu`, `close`), or `invoke_command` on a command's stable ID |
| `pointer` | per target | move, hover, click, double click, right click, down, up, drag along a path, wheel; at a target (and an offset in it) or a position |
| `keyboard` | per target | key chords and sequences, committed text, composition |
| `text` | read-only, or per target to set the selection | read a text control's value, selection and lines; set the selection |
| `wait` | read-only | until the next frame, idle, a time, or a condition on a node |
| `capture` | read-only | a window or control as a PNG: `rendered` (drawn fresh from the current state) or `presented` (the pixels last shown) |
| `inspect` | read-only | metrics, dispatcher, interaction state, a control's parts and part states, its paint operations, the theme rules that set each field, what its schema says it can do |
| `batch` | the strongest mark of its steps | several of the above in one call, with one combined reply (ruling 22) |
| `control` | per launch rules (section 8) | `direct_begin` and `direct_end` for `model.directed`; `suspend_input` in debug and special builds |

Programs' commands (`Command` objects with stable IDs) are also listed as
tools automatically, each with its enabled state, so a model can call
"Save" or "Flip horizontal" by name. A program can hide any of them.

### 7.3 The outline

```text
window "Untitled - Plan Paint" [main] active 1280x800
  ribbon [ribbon]
    tab_group "Ribbon tabs" [ribbon.tabs]
      tab "Home" [ribbon.tab.home]
      tab "Pencil" [ribbon.tab.pencil] selected hovered
    tab_panel "Pencil" [ribbon.panel.pencil]
      slider "Width" [pencil.width.slider] value=3 range=1..64
      button "Smooth" [pencil.smooth] checkable checked
  pane "Canvas" [canvas] focusable
  status_bar [status]
    static_text "640 x 480 px" [status.size]
```

- One node a line: role, quoted name, `[stable ID]`, the states that are
  set, value and range; bounds on request.
- A node without a stable ID shows `[#r412]`, its runtime ID for this run
  only, and the reply counts such nodes.
- `hovered` and `pressed` come from the visual inspection, which a model
  debugging appearance needs.
- Password fields show as `text_box "Password" [login.password]
  protected`: no value, and never `focused` (section 9.1).

### 7.4 The reply after an action

```text
ok: press [ribbon.tab.pencil] (semantic action)
changed: [ribbon.tab.pencil] +selected; [ribbon.tab.home] -selected
added: tab_panel "Pencil" [ribbon.panel.pencil] (6 nodes)
removed: tab_panel "Home" [ribbon.panel.home] (11 nodes)
focus: [canvas] -> [ribbon.tab.pencil]
frames: 2 presented; idle after 34 ms
```

### 7.5 Errors

| Code | Meaning | Hint given |
|---|---|---|
| `not_found` | no node with that ID | the closest IDs by name |
| `stale_identity` | the control is gone (GF017) | take a new snapshot |
| `ambiguous` | the ID is repeated | name the parent |
| `disabled`, `not_visible` | node states | |
| `unsupported_action` | not among the node's actions | the actions it has |
| `dialog_open` | a dialog is open and not instrumented (9.2) | the dialog's ID; ask the user |
| `needs_user` | a security approval is waiting (9.3) | what the user must do |
| `user_took_control` | the user took control back (8.3) | the steps not run |
| `declined` | the user refused destructive acts for this launch (10) | |
| `protected` | a password field | |
| `timeout` | `wait` ran out | what was still busy |
| `not_in_this_build` | `suspend_input` in a release build | |

### 7.6 Resources and prompts

Tools are the main surface. Resources mirror the read-only parts for
clients that use them: the windows, each window's outline, metrics and
capture, and the control schema and member manifest. Three prompts:
reproduce a visual bug, explore this program, check this window's
accessibility.

## 8. Input: a model is treated like a person

### 8.1 The default: first come, first served

The real mouse and keyboard are never ignored (ruling 10). Model input and
user input go through the same door, in the order they arrive:
- every host already turns its events into portable `HostEvent`s and
  dispatches them through the window's `HostSession`; the model's input is
  built as the same events and dispatched through the same session, on the
  window's UI thread, at a dispatcher turn;
- whichever arrives first is handled first, whole. If the user clicks while
  the model hovers, the click happens and the model's next reply shows it in
  its diff;
- a model's synthetic pointer is one more pointer: when the real pointer
  moves, hover follows it, exactly as when two people share a machine. A
  model that needs a hover to stay put either asks the user to keep still or
  uses `model.directed`.

### 8.2 Suspending user input: debug and special builds

In debug builds, and in special builds made for testing (question 2 asks
what counts as special), `control suspend_input` holds user input for a
chain of commands, normally as the first step of a `batch`:
- the session drops the adapter's input for the program's windows and counts
  what it dropped;
- input comes back by itself when the chain completes, fails, times out or
  the pipe closes. There is no way to leave it suspended;
- a ceiling (default 30 s) restores input even if the chain is still
  running, and the chain then fails with `timeout`.

In release builds the command answers `not_in_this_build`. This is the
only part of the design that blocks input without a visible popup, which is
why it is not in release builds.

### 8.3 `model.directed`

For any build, including releases (ruling 12):
1. The program is launched with `--model-directed`, or its configuration
   sets `model.directed`. Without either, `direct_begin` is refused.
2. The model sends `control direct_begin` (alone or as a `batch` step). The
   interface stops taking user input, and a small popup appears:
   "<model> is working. Take back control?"
3. Only the popup can be clicked or typed into. Every other window ignores
   user input while the popup shows.
4. If the user takes back control, the model's chain stops at once with
   `user_took_control`, naming the steps not run. Input returns to the user
   and the popup closes.
5. When the model sends `direct_end`, or its chain completes, input comes
   back by itself and the popup closes. It also comes back if the chain
   fails, times out or the pipe closes.

House style: a small state machine per run, with one fallback that every
unexpected state reaches: **the user has control**.

```text
user_control ──direct_begin──► model_directed ──direct_end / completed──► user_control
       ▲                           │  ├─ take back control ─► user_control (chain stops)
       └──────── fallback ─────────┘  └─ fail / timeout / pipe closed ─► user_control
```

Notifications follow the change: the state changes first, then the popup,
the border and the model's reply learn of it.

## 9. Dialogs, passwords and security approvals

### 9.1 Passwords

Following Anthropic's lead (ruling 13):
- a password field's value is never read;
- **whether a password field has focus is never exposed** either: the
  outline never shows `focused` on it, and focus moves into or out of it are
  left out of diffs;
- captures paint its bounds out;
- the model may still type into a field it was told to fill, and the reply
  says only `ok`.

The rule follows the existing `SemanticState::protected_content` bit, so it
covers every control that sets it.

### 9.2 Dialogs

While a modal dialog is open, the model is refused control (ruling 14):
`act`, `pointer` and `keyboard` answer `dialog_open` with the dialog's ID,
and the read-only tools still work, so the model can tell the user what the
dialog asks.

A program can **instrument** a dialog: mark it as one a model may drive
(`Dialog::set_model_drivable`, and the matching C ABI flag). The setup
dialog of an installer is the example: a model starts it, fills it in and
moves through its pages.

Native dialogs of the operating system (NSOpenPanel, IFileDialog, the GTK
file chooser) cannot be instrumented: they are not GUI.Forms trees. The
model gets `dialog_open` and the user handles them.

(This reading of ruling 14, that "refused while a dialog is open" is the
default and instrumentation is the exception, is question 3.)

### 9.3 Security approvals

Inside an instrumented dialog, a program marks the steps that need a
person: a security approval, an administrator password, a licence
acceptance. When the model reaches one:
1. the model's call answers `needs_user`, saying what the user must do;
2. the interface shows that the user must act, with the step highlighted;
3. the user does it (the user takes back control briefly);
4. the model's next `wait` reports the step done, and it carries on.

Approvals that the operating system shows (an administrator prompt, a
keychain prompt) are outside the program and always need the user.

## 10. Consent and destructive acts

**Consent is built in** (ruling 15). It needs no extra prompt because:
- a model can only reach a program its own client started with `--mcp`;
- the window shows that a model is connected (section 11.1);
- the user can always act first (8.1) and can always take control back
  (8.3).

**Destructive acts** (rulings 17 and 18):
- by default every act counts as non-destructive;
- developers mark what is destructive: a `Command`
  (`Command::set_destructive(true)`), a control's action (a property in the
  schema, so form documents can carry it too; question 7), or a whole
  window. The C ABI gets the same flags;
- the first destructive act of a launch shows, in the program:
  "Allow <model> to edit files and perform potentially destructive acts?"
  with Allow and Don't Allow. The answer holds for the rest of the launch;
- Don't Allow makes every destructive act answer `declined`. Non-destructive
  acts still work;
- the question is the program's own and is independent of any approval the
  client's harness asks for;
- MCP's tool annotations carry the marks to the client: read-only tools say
  `readOnlyHint`, and a tool whose target is marked destructive says
  `destructiveHint`, so clients that show approvals can use them.

## 11. What the user sees

### 11.1 The border glint

While a model is connected, the window's application border (ruling 19)
shows it:
- the border is the active border capability (roadmap row 2.7, phase 2
  done in cp32 fold E) with the window band of cp33
  (`claude/cp33-window-band`), which is not finished;
- an animated glint runs along it whenever the model reads the program's
  state (`snapshot`, `capture`, `inspect`) or acts on it;
- at rest, while connected, the border keeps a quiet tint;
- with reduced motion, the glint becomes a short change of tint;
- it is drawn by the engine as an effect, so themes can restyle it, and
  it costs nothing while no model is connected (idle still schedules no
  frames).

### 11.2 The tag line

A small tag line tooltip on the top border names the model in control
(ruling 20), for example "Claude Opus 5.5 via Claude Code", from what the
client states (4.4). It shows on hover over the border, and briefly when a
model connects.

### 11.3 The popups

Two popups belong to the program, drawn by GUI.Forms in its own design
language: the "take back control" popup of `model.directed`, and the
destructive-act question. Both name the model.

## 12. Simulated time, explained

The owner asked what this means. Some of a program's behaviour depends on
time passing: a hover plate fades in, a tooltip appears after half a
second, a menu opens after a delay, an animation plays. Normally a model
that wants to see the hover after 200 ms must wait 200 real milliseconds,
and the result can vary with how busy the machine is.

**Simulated time** means the program's clocks stop following the real
clock while a model is testing, and move only when the model says "wait
200 ms". Then the program jumps exactly 200 ms ahead at once: the hover
plate is exactly where it is at 200 ms, every time, on every machine, with
no waiting. GUI.Forms already has replaceable clocks for input and motion
(`InputTimeSource`, `MotionTimeSource`), so this is mostly wiring. The
cost: while time is simulated, animations stand still unless the model
waits, so it suits tests and bug reproductions, not a model helping a user
live.

Question 4 asks whether to offer it (recommendation: debug and special
builds only, like `suspend_input`).

## 13. Prior art: what to take, what to avoid

### 13.1 On this computer

Read from the app bundles, the Codex configuration and Chrome's
native-messaging manifests, without running anything:

| Program | Shape | Take | Avoid |
|---|---|---|---|
| **Claude in Chrome** (Anthropic; Rust helper in `Claude.app`) | a user-only Unix socket in front of a native-messaging relay; rejects peers of another user | a thin, careful relay | a socket at all: our pipes need none |
| **Claude computer use** (Anthropic; Rust helper) | named refusals before every act: secure input, a password field, a sheet over the window, the user typing | refusing with reasons; never touching password fields | needing operating-system permissions to see the screen |
| **Codex computer use** (OpenAI; Swift MCP client plus a privileged service) | per-app approvals remembered once, per turn, per session or always | approvals with a remembered scope | "Action completed, call `get_app_state`": the model must re-read everything after each act |
| **Codex browser tools** (OpenAI; one `js` tool) | accessibility state as diffs by default; several actions per call; page-declared tools preferred to clicks | diffs, batching, program-declared actions | running model-written code inside the program |
| **Codex app tools** (OpenAI; Node) | per-tool approval modes: reads automatic, deletes prompt | marking tools read-only or destructive | |

### 13.2 Apple: what to avoid

Xcode 26.3 (February 2026) offers its capabilities over MCP through `xcrun
mcpbridge`, a separate program that relays to the running Xcode over XPC.
The owner judged it the worst of all (ruling 26), and the reports agree on
why:
- a separate bridge program, and a running Xcode with a project open,
  before anything works;
- an "Allow 'agent' to access Xcode?" prompt for every new agent process,
  tiresome enough that users script a click on it;
- replies that did not match their own declared schemas
  (`structuredContent` missing), which broke stricter clients;
- MCP behind a settings switch the user must find.

Our design avoids each: no bridge (the program is the server), consent
built into the launch and the take-back popup instead of prompts per
process, every reply validated against its schema in tests, and the
capability on by default in the build.

### 13.3 Protocols and frameworks

- Chrome DevTools Protocol: input dispatched through the real input path;
  virtual time; avoid its hundreds of methods.
- Playwright and Playwright MCP: waits built into actions; locators by role
  and name; accessibility snapshots with handles.
- Espresso: wait for the UI thread to be idle rather than sleeping.
- Flutter: the closest relative, a toolkit that draws itself and publishes
  its own semantics tree.
- Anthropic's "Code execution with MCP" (2025-11-04): tool lists and large
  intermediate results are the main cost for a model; our answers are few
  tools, diffs and `batch`.

## 14. GUI.Forms seams the core uses

| Need | Seam |
|---|---|
| Windows | `Application::windows()` |
| Control tree | `SurfaceRoot::semantic_snapshot()`, `semantic_generation()` |
| Actions | `perform_semantic_action` (by stable ID, or runtime ID and virtual child), which refuses stale identities |
| Text | `semantic_text_layout`, `set_semantic_text_selection` |
| Input | `HostSession::dispatch(HostEvent)`, which every host already uses |
| Pixels | `Window::capture_window`, `capture_control`, `encode_window_capture_png`; the host's presented raster |
| Counters | `metrics_snapshot`, `dispatcher_snapshot`, `interaction_snapshot` |
| Inside controls | `visual_inspection_snapshot`, `Theme::provenance` |
| Commands | `Command::stable_id()` |
| Clocks | `set_input_time_source`, `MotionClock::set_time_source` |
| Reentrancy | the Windows channel's rule, moved into the core: a request waits while the thread is inside a dispatch, and a modal loop sets that aside |

**Idle**, which every action waits for, means: no posted dispatcher work,
no pending layout or paint, no animation asking for frames, no present
outstanding, and no timer due within 50 ms.

**Modal loops**: the core never waits for an action's call to return,
only for idle, which a nested loop also reaches. Pressing "Save As" replies
`opened: dialog [save_as] modal` while the handler is still on the stack,
and the dialog rules of 9.2 then apply.

## 15. Testing

The same requests run without a model:
- as a script file of `batch` steps, run by the program
  (`--mcp-script file.jsonl`) or in process by ctest on the headless host;
- a model's session can be saved as a script, so a bug a model finds
  becomes a regression test once a person adds the assertion.

### 15.1 The Pencil tab

```text
{"tool":"batch","steps":[
  {"tool":"act","target":"ribbon.tab.pencil","action":"select"},
  {"tool":"pointer","action":"hover","target":"ribbon.tab.pencil"},
  {"tool":"wait","until":"ms","ms":200},
  {"tool":"wait","until":"idle"},
  {"expect":"visual","target":"ribbon.tab.pencil","part":"plate","state":"hover","drawn":true},
  {"expect":"capture","target":"ribbon.tab.pencil","compare":"presented=rendered"},
  {"tool":"inspect","target":"ribbon.tab.pencil","what":["theme","metrics"]}]}
```

In a debug build, `control suspend_input` as the first step keeps the real
mouse from disturbing the hover. In a release build launched
`--model-directed`, `direct_begin` does the same with the popup.

### 15.2 What it replaces

The Windows `WM_COPYDATA` channel, `tools/menu_probe.py`,
`tools/popup_margin_probe.py`, the X11 `--automation` flag (ruling 23),
Plan Paint's AppKit probe harness with `cliclick`, and most per-host
normative trace drivers' scripted scenes.

### 15.3 `stx_mcp`'s own tests

Framing and size bounds, malformed input, cancellation, shutdown order,
channel bounds, and every reply checked against its declared schema (the
mistake in Xcode's bridge).

## 16. Plan

After the product releases, before the 1.0 freeze, not this week (ruling
2).

| Stage | Where | Contents | Depends on | U |
|---|---|---|---|---|
| X1 | STXlib | `stx_mcp`: channels, contexts, tasks, JSON-RPC over pipes, the dispatcher with declared tool specs, schemas and validation, MCP tools, resources, prompts and notifications; its tests | STXlib's coordinator | 2-3 |
| G1 | GUI.Forms | the automation core on the headless host: outline, diffs, `act`, input through `HostSession`, idle and `wait`, captures, `inspect`, `batch`, scripts | none (can start beside X1) | 2-3 |
| G2 | GUI.Forms | the wiring: `GUI_FORMS_MCP` on by default, `--mcp`, identity, standard output redirection, commands as tools; on macOS, Windows and X11 | X1 on the stx pin, G1 | 1.5-2 |
| G3 | GUI.Forms | control and safety: first come first served, `suspend_input` (debug and special builds), `model.directed` with its popup and state machine, dialog refusal and instrumentation, security steps, password rules, destructive marks and the per-launch question | G2 | 2 |
| G4 | GUI.Forms | the visible signs: the border glint on the active border and window band, the tag line | G2, the window band | 1 |
| G5 | GUI.Forms | the C ABI (destructive marks, model-drivable dialogs, hidden commands, the identity) and the C# binding; retire the Windows channel and the X11 flag; convert the probes | G3 | 1.5-2 |
| G6 | GUI.Forms | section 18: event propagation, layout reasons, resources and state summaries in `inspect`; `forms export`, `validate`, `apply`, `revert` and the build toggle to write out form documents | G2, FD | 2-3 |

About 12-17 U with G6. The ABI additions in G5 must land before the 1.0 freeze.

## 17. Answers of 2026-11-18, and what is still open

1. **The launch argument: `--mcp`** (owner). Why one is needed at all, as
   the owner asked: the build option only makes the capability *present*.
   Every program, however it is started, has a standard input: a terminal,
   a file, another program's pipe, or nothing. If a GUI.Forms program
   always read its standard input as MCP, a program started from a terminal
   would sit waiting for JSON that never comes, and one started by a script
   that pipes text into it would read that text as broken requests. `--mcp`
   is the client saying "these pipes are an MCP conversation". Without it
   the capability stays dormant: no thread, nothing read.
2. **Special builds.** The owner answered "--mcp" here. Read as: a special
   build is one configured with an MCP test switch beside the ordinary one
   (proposed spelling: the build option `GUI_FORMS_MCP_TEST_CONTROLS`, set
   by `tools/ci_build.py --mcp-test`), which admits `suspend_input`. To be
   confirmed with the owner; nothing depends on it until stage G3.
3. **Dialogs: agreed** as written in 9.2 and 9.3.
4. **Simulated time: dropped from scope.** The owner does not see where it
   is needed, and on reflection it is needed nowhere essential: the tests
   (now in `gui_forms_tests`) already drive the headless host with
   caller-supplied timestamps, which is deterministic. Its only use would be
   reproducing a timing bug exactly in a live program. Revisit only if such
   a bug comes up.
5. **The tag line's name: agreed** (the client's own statement).
6. **rpclib: follow its shape, write `stx_mcp` fresh** (owner).
7. **Destructive marks in form documents: decided by the existing model,
   no owner call needed.** Form documents (`.form` files, stage FD) describe
   a window's controls, properties and commands as data and are compiled
   into C++. The forward programme makes the C++ API, form documents and
   themes projections of one registry. The destructive mark is a registry
   property of commands and actions, so it is settable in code and in
   `.form` alike, with code as the default place.

## 18. Extensions from the owner's roadmap notes (2026-11-18)

### 18.1 Automatic semantic instrumentation

The owner wants the MCP surface to tell a model that cannot see the screen
as much as is useful, concisely, because GUI.Forms owns the whole stack.
Beyond the outline and `inspect` of section 7, `inspect` gains:
- **event propagation**: for the last input, or the next one, the route it
  took: hit test, capture, the controls offered it, who handled it, the
  commands it fired. The core already decides each step; it records them in
  a bounded trace while a model is connected;
- **layout reasons**: why a control has its size and place: the panel, its
  rules (flex, grid, anchors), the constraints that bound it, and which ones
  were tight;
- **resources**: images, fonts, theme bakes and allocations attributable to
  a window or control (the existing allocation statistics and image
  registry snapshots);
- **state summaries**: a one-paragraph plain description of a window,
  generated from the tree (what it is, what is selected, what is enabled,
  what is waiting on the user), so a model can start from words rather than
  a tree.

Each is opt-in per call, bounded, and text first, so the concise default
does not grow.

### 18.2 Dynamic interfaces and structural patching

GUI.Forms compiles interfaces from form documents, but already changes and
invalidates them at run time (data, tables, ribbon panels). The owner's
note: a compiled program exposes a typed vocabulary (controls, properties,
layouts, commands), and a model could compose new interface from it at run
time **without introducing executable code**, validated before it is
instantiated, because every primitive is compiled in.

In practice (owner): an MCP capability plus a build toggle that lets a
program write out the form documents it was built from (when they are not
shipped with it) and accept replacements, as is already planned for themes
(user themes, `--theme`, theme export). Proposed tools, in a stage G6:
- `forms export`: the form document of a window, as the program has it;
- `forms validate`: a candidate document checked against the registry and
  the program's commands and bindings, with the theme language's diagnostic
  rules (unknown properties warn, bad values fail);
- `forms apply`: a validated document replaces a window's structure, or a
  named part of it (structural patching), keeping bindings to the program's
  data and commands; bad documents leave the window as it was;
- `forms revert`: back to the compiled form.

Rules: only registry vocabulary, no code; bindings only to what the program
already exposes; a replaced form is marked in the window (the border tag
line says so); applying counts as destructive unless the program says
otherwise. Use: prototyping looks and layouts by sending documents to a
demo program; a model composing a workflow-specific panel over a program's
existing commands.

The owner also listed better animation of ribbon transforms for
discussion; it belongs with the ribbon (RB), not here.

## Sources

Repository (read): `AGENTS.md`; `src/host/surface/README.md` and
`input/surface_input.hpp`; `include/gui_forms/semantics/types/
semantic_types.hpp`; `include/gui_forms/surface_root/surface_root.hpp`;
`include/gui_forms/host/types/host_types.hpp`; `src/host/headless/session/
headless_host.hpp`; `src/host/surface/manager/surface_manager.cpp`;
`src/surface/windows/application/windows_host.cpp`;
`include/gui_forms/visual_inspection/types/visual_inspection_types.hpp`;
`tools/normative_trace.py`, `tools/menu_probe.py`; `CMakeLists.txt`.
Notes (this branch): `planning/FORWARD_PROGRAMME.md`,
`planning/ROADMAP_TO_1.0.md`, `planning/stages/README.md`,
`planning/stages/release-1.0/README.md`.

This computer: `~/.codex/config.toml`, the Codex plugin manifests and
browser documents, the strings of `SkyComputerUseClient`,
`chrome-native-host`, `app-cu-helper` and `github-mcp-server`, Chrome's
native-messaging manifests, the Claude configuration files. Account
identifiers and tokens seen there are not reproduced.

Outside: rpclib's repository page (github.com/rpclib/rpclib); the MCP
2026-07-28 revision as summarised by blog.modelcontextprotocol.io and
secondary write-ups (to confirm against the official text before X1);
Anthropic, "Code execution with MCP" (2025-11-04); Xcode 26.3 reports
(InfoQ, 2026-02; danielsaidi.com, 2026-04-30); Playwright MCP's
documentation.
