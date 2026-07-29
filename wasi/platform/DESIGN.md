# love-wasi platform seam — design decisions (build-order step 6)

Build-order step 6 reseams the roles SDL plays for desktop LÖVE — window + GL
context, input, filesystem, timer, system — onto browser primitives, under the
same guarded-seam discipline as the graphics (`opengl` → WebGL2) and audio
(OpenAL → WebAudio) seams. This note records the decisions the seam is built on,
and — because step 6 is where the roadmap starts leaning into agentic,
live-edited development — the decisions the downstream **live-edit / agent**
consumer forces, which are surfaced here **while still open** (AGENTS.md: never
hand the architect a result built on choices they never saw).

Where a passage reads as a plan, the code has not landed it yet. **Step 6 is
COMPLETE — 6.1–6.7 are all built** (the `love_fs` read seam; the real
`love.filesystem` replacing PhysFS; the real `love.window` replacing SDL; the real
`love.event`/`keyboard`/`mouse` on the `love_input` push seam; the real
`love.joystick`/`gamepad` on the `love_gamepad` poll seam; the real
`love.timer`/`love.system`; the **first full `main.lua` frame** (`conf` → canvas →
`love.load` → `love.draw` → present, pixel recovered); and — the capstone — **the
embedding contract** (6.7): the `love.filesystem` write path + save dir on new
`love_fs` write imports, the host-callable `pump_invalidate()` reload primitive
(write → invalidate → re-require = live-edit), and the documented host-import seam
(`EMBEDDING.md`) — see the ledger below), and issue #27's warning mechanism +
`love.sensor` warned stub have landed. The former "step 8" IDE work is dropped
from this repo's scope; the IDE is a downstream consumer of the 6.7 contract.

## The fidelity standard (project-wide): browser-native correctness first

Two use-cases share this one engine: a **LÖVE game that actually runs in the
browser** (the priority), and a **desktop-fidelity preview** of a game bound for
desktop. They imply two different bars, and the priority order is:

1. **100% correct browser game — the must-hit bar.** As a *browser* game, the
   engine must be complete and correct. This is achievable and non-negotiable.
2. **`.love` source-compatibility — a pillar.** The same source runs unmodified
   on desktop LÖVE; a game made here can go to desktop and back.
3. **Desktop *behavioral* parity — aspirational, the reference not the pass/fail
   line.** The browser genuinely cannot match desktop 100% (async storage
   durability, HRTF, mic rates, threading), and nobody expects it to. Where it
   can't be met, the divergence is *declared*, never faked.

The consequence for every seam decision: measure it against **"what does a
correct browser game do?"**, not "does it byte-match desktop?" Desktop is the
reference; browser-native correctness is the standard we hold to 100%. This
generalizes the principle already stated for audio (`wasi/audio/DESIGN.md`,
Decision 3: *"the bar is device-agnostic fidelity, not desktop parity"*) to the
whole engine, and it is why "browser preview only" is no longer the framing —
browser-native games are a first-class target (readme.md, Mission).

## The other principle: the game stays pure LÖVE; the host holds the powers

Features (live-edit, console, reload) and fidelity need not conflict, because the
feature surface is **host-side, never a game-facing API**. The shipped `.love` is
a normal LÖVE 12 game: run it on desktop LÖVE and it behaves identically, because
none of the live-edit / console / reload machinery is baked into the artifact or
visible to Lua. The IDE mutates the project *out of band* through host imports;
the game never references any of it.

The single place this tempts a fidelity violation is the console-control idea
(below, D6): if "control what's in the console" became a `love.log()` the game
*calls*, it would break on other engines. It stays faithful instead — `print`
is `print`; control is host-side.

## Sub-step ledger (proposed — the architect owns the ordering)

Boot order puts filesystem first: LÖVE reads `conf.lua`/`main.lua` before it
opens a window. Step 3's boot witness proves LÖVE's `main()` dies *at* the
`love.filesystem` seam; step 6's job is to carry it *past* that line.

- **6.1 — `love_fs` VFS seam, read round-trip.** **Done** (this note's companion
  code). Isolates the host↔wasm file-bytes plumbing — a binary asset with
  embedded NULs recovered byte-exact through the seam under node:wasi + Chromium
  — before the real module rides on it. No LÖVE core linked; the analogue of
  graphics' 4.1a raw-GL leg.
- **6.2 — real `love.filesystem` on the seam. Done** (scripted; node:wasi + real
  Chromium; CI step added). D1=A: a real `love::filesystem::wasi_fs` backend
  (`wasi/platform/fs-backend.{h,cpp}`) replaces the PhysFS module and the boot
  stub. `require("love.filesystem")` now succeeds (the step-3 stop-line is gone)
  and `read`/`getInfo`/`openFile`/`File:read`/`load`/`require` recover host files
  byte-exact (incl. binary/NUL) through the real module. Driven directly from a
  witness coroutine (not full `boot.lua`, which needs `love.system`/window/event
  — those are 6.3–6.6). Read-only: `write`/`mount`/enumerate throw/false loudly,
  not faked; the write/save-dir path (D2's OPFS) is the 6.7 sub-step. Shared
  engine touched only through guarded seams (`Filesystem.cpp` `getExecutablePath`
  + `<filesystem>`; `wrap_Filesystem.cpp` factory + SDL `extloader`), byte-clean
  for desktop. The `extloader` native-C `require` searcher is dropped on wasm (no
  `dlopen`) — a declared divergence.
- **6.3 — `love.window`. Done** (scripted; real Chromium; CI step added). D3=A: a
  real `love::window::wasm` backend (`wasi/platform/window-backend.{h,cpp}`) on a
  `love_win` host seam (`window_setmode`/`window_get_pixel_dimensions`/
  `window_present`). `setMode` drives the host to create the real `<canvas>` +
  WebGL2 context and make it current for the `love_gl` imports **before**
  `graphics->setMode(nullptr,…)` runs — retiring the fake `setMode`
  `graphics-ext.cpp` plays. With a registered, open window, `Graphics::isActive()`
  is true, so `present()` runs for real — which **completed step 4**:
  `captureScreenshot` reads the presented backbuffer (FBO 0) back through
  `newImageData`, drawn + clear pixels recovered exactly. One guarded seam
  (`wrap_Window.cpp` factory), byte-clean for desktop; the window-irrelevant
  surface (fullscreen, displays, dialogs, …) is honest no-ops.
- **6.4 — `love.event` + keyboard/mouse. DONE** (node:wasi + real Chromium; CI
  step added). The real `love.event`/`love.keyboard`/`love.mouse` on the
  `love_input` host seam (`wasi/platform/input-backend.{h,cpp}`), replacing the
  three SDL backends. This is the first **host→guest push** seam — every prior
  seam was guest→host pull (guest asks, host answers synchronously); DOM events
  fire on the browser event loop, the host queues them, and
  `event::wasm::Event::pump()` drains that queue once per frame, translating each
  record into a `love::event::Message` (the exact job `event/sdl/Event.cpp
  ::convert` does for SDL) that the unchanged Lua dispatch in `callbacks.lua`
  fires as `love.keypressed` / `love.mousepressed` / … . One shared `InputState`:
  `pump()` is the single writer (pressed-key/scancode sets, mouse position,
  button mask), keyboard/mouse are pure readers — the same split SDL has
  (`SDL_PumpEvents` updates what `SDL_GetKeyboardState`/`GetMouseState` read). The
  DOM↔LÖVE name/button mapping lives in C++ next to LÖVE's Key/Scancode enums;
  the physical-`code`→US-key translation is a declared, documented divergence
  from SDL's live-layout mapping (the typed character still rides through as the
  `textinput` payload). Three guarded factory seams (`wrap_Event`/`wrap_Keyboard`/
  `wrap_Mouse`), byte-clean for desktop, plus one generic version-guarded
  `lua_cpcall`→`lua_pcall` shim (Lua 5.2 removed `lua_cpcall`; `love.event`'s
  modal-draw path is the only caller — offered upstream). `love.image` +
  `love.filesystem` link because `love.mouse`'s Cursor is image/file-backed;
  witnessed windowlessly, so it runs on node **and** Chromium (no WebGL2).
  `isModifierActive` (lock latch), custom image cursors, and pointer confinement
  are the honest warn-once edges.
- **6.5 — `love.joystick` + `love.gamepad`. DONE** (node:wasi + real Chromium; CI
  step added). The real `love.joystick`/`love.gamepad` on a new `love_gamepad`
  host seam (`wasi/platform/joystick-backend.{h,cpp}`) over the browser **Gamepad
  API** — **required for fidelity, not optional**: gamepads are a capability the
  browser genuinely *has*, so warned-stubbing them (as we did `love.sensor`, a
  genuinely-absent capability) would violate the "correct browser game held to
  100%" bar. Unlike 6.4's host→guest *push* queue, the Gamepad API is
  **poll-based** (no event stream, only a per-frame snapshot array) — so the seam
  is guest→host *pull* (`gamepad_count`/`gamepad_read`, mirroring
  `navigator.getGamepads()`): once per frame the guest reads the current gamepad
  slots and **diffs** them against the previous poll to *synthesize* the
  `joystickadded`/`joystickremoved`, `joystick{pressed,released,axis}` and
  `gamepad{pressed,released,axis}` events SDL would have delivered, emitting BOTH
  the raw-joystick and the mapped-gamepad event for one physical change exactly as
  SDL sends both families. That synthesis **reuses 6.4's push mechanism**: the
  diffed events are `love::event::Message`s pushed onto the same `love.event`
  queue. The poll is wired into 6.4's `pump()` by a **weak hook**
  (`wasi_poll_gamepad_events`, declared null in `input-backend.cpp`, defined
  strong in `joystick-backend.cpp`), so the 6.4 build — which does not link the
  joystick module — is unaffected (the symbol is null and the call skipped; the
  6.4 witness re-runs green with the hook in place). LÖVE's `love.gamepad` is
  SDL's standard-controller mapping, ~1:1 with the **W3C "standard gamepad"**
  mapping, so it rides it directly; the W3C-index↔LÖVE-button and axis translation
  lives in C++ next to LÖVE's enums (host forwards browser truth, the backend owns
  LÖVE semantics — the same split the input backend has). W3C buttons 6/7 (the
  analog triggers) map to LÖVE trigger **axes**, not buttons, so a trigger emits
  `gamepadaxis`, matching SDL. One guarded factory seam (`wrap_JoystickModule.cpp`,
  byte-clean for desktop under `#else`). Enabling `love.sensor` (the #27 warned
  stub) is **required** here, not incidental: `wrap_Joystick.cpp` registers
  `Joystick:getDevicePowerInfo`/`:getDeviceConnectionState` unconditionally but
  only *defines* them under `LOVE_ENABLE_SENSOR` (upstream bug #23), so joystick
  won't link with sensor off — enabling it moots #23 by config. The honest
  warn-once edges: **vibration** (the browser gamepad *has* a `vibrationActuator`,
  but it is a host effect the windowless witness cannot observe, so 6.5 reports it
  unsupported and `setVibration` is a no-op returning false rather than faking an
  unwitnessable rumble), the **gamepad-mapping string** (no SDL controller DB in
  the browser — the W3C standard mapping is implicit, so `getGamepadMappingString`
  / `setGamepadMapping` / `loadGamepadMappings` are empty/no-op), and **gamepad
  motion sensors** (no gamepad sensor stream). The input path itself is real.
  Witnessed windowlessly on node **and** Chromium.
- **6.6 — `love.timer` + `love.system` + the first full `main.lua` frame. DONE**
  (6.6a windowless on node:wasi + real Chromium; 6.6b Chromium-only; CI steps
  added). Two phases:
  - **6.6a — `love.timer` + `love.system`.** `love.timer` is a concrete class (no
    backend split): `Timer.cpp` routes through
    `clock_gettime(CLOCK_MONOTONIC)`/`gettimeofday` under a guarded `LOVE_WASI` arm
    of its POSIX `#if` (wasi-libc provides both; the WASI host fulfils
    `clock_time_get`), and `love::sleep` is an **honest browser no-op**
    (`wasi/platform/delay-wasi.cpp`) — a browser must not block its main thread;
    frame cadence is the host's `requestAnimationFrame`, not a guest spin — in
    place of the SDL `SDL_DelayNS` `common/delay.cpp` (excluded from every wasm
    build). `love.system` is backend-split: a real `love::system::wasm::System`
    backend (`wasi/platform/system-backend.{h,cpp}`) on a new `love_system` host
    seam carries the **genuine browser capabilities** — processor count
    (`navigator.hardwareConcurrency`), the text clipboard (a host cell fronting the
    async Clipboard API), `openURL` (`window.open`), preferred locales
    (`navigator.languages`) — and reports honest defaults for the rest (memory size
    0; power `unknown` — the Battery Status API is gated across engines);
    `getOS()` returns `"Web"` via a guarded seam in `System.cpp`. Three guarded
    seams (`Timer.cpp` POSIX arm, `System.cpp` `getOS`, `wrap_System.cpp` factory),
    byte-clean for desktop.
  - **6.6b — the first full `main.lua` frame (THE MILESTONE).** The **union** build
    (`build-frame.sh`: real filesystem 6.2 + window 6.3 + graphics/opengl-on-WebGL2
    step 4 + image + font + event/keyboard/mouse 6.4 + timer + system 6.6a + data +
    math) boots LÖVE's **real `boot.lua`** under the pump and runs an actual game
    end to end: `conf.lua` (read through the real `love.filesystem`) sizes/titles
    the canvas, `love.window.setMode` opens the real WebGL2 context at the conf
    dimensions, `love.load` runs (a unique marker to the host tap proves it), and
    `love.run`'s loop yields once per pumped frame running
    `event.pump`/`timer.step`/`update`/`clear`/`draw`/`present`. `love.draw` fills
    the canvas RED; the driver reads the presented backbuffer's centre pixel back
    through the WebGL2 context and recovers `(255,0,0,255)` — proving
    conf → canvas → load → draw → present ran a real frame. `frame-deps-stub.cpp`
    replaces the windowless graphics build's `graphics-deps-stub.cpp` (the union
    compiles the real filesystem + timer, so only the genuinely-absent
    audio/video/thread module symbols love.graphics links against are stubbed —
    reusing the graphics stub would duplicate `File::type`/`luax_getdata`/
    `Timer::getTime`). `love.joystick` is deliberately not linked (the event module
    needs only the joystick HEADER; `input-backend.cpp`'s `wasi_poll_gamepad_events`
    weak hook stays null, so `pump()` skips it). Chromium-only — a real WebGL2
    context, node has none — exactly like the 6.3 window witness and the step-4
    graphics witnesses; no node leg (expected). The one integration subtlety: the
    canned `conf.lua` disables every module the union does NOT link
    (thread/joystick/touch/sound/sensor/audio/video/physics), because `boot.lua`'s
    module loop `require`s each enabled module unconditionally.
- **6.7 — the embedding contract. DONE** (scripted; node:wasi + real Chromium; CI
  step added) — the runtime's capstone, and the boundary of this repo's
  responsibility. What makes the runtime *consumable* by a live-edit host:
  - **The filesystem write path** — the real `love.filesystem`
    `write`/`append`/`remove`/`createDirectory`/`File:open("w"/"a")` and the save
    dir, over three new `love_fs` write imports (`fs_write`/`fs_remove`/`fs_mkdir`,
    entirely in the out-of-tree `fs-backend.cpp` + host — **no new `src/` seam**).
    The host holds a **separate writable save namespace** (D2, OPFS-backed in the
    browser) beside the read-only project; reads resolve **save-first then
    project** (physfs mount order), so a written file shadows a project file and
    removing the save copy reveals the pristine project beneath — the witness
    proves, by transcript alone, that writes never mutate the project.
    `getSaveDirectory()` = `save:<t.identity>`. Writes are NUL-safe.
  - **The reload / invalidate primitive** (D5=A: minimal & explicit, whole-chunk
    re-eval) — a host-callable `pump_invalidate()` export (+ a Lua twin
    `__pump_invalidate()` for in-script driving) that drops **game** Lua modules
    from `package.loaded` while preserving `love`/`love.*` and the standard libs.
    `g_L` persists across `pump_boot`, so caches survive a reboot — this clears
    them. The **reload invariant** is witnessed: `require("mod").v==1` → host-edit
    the source via the write path (`return {v=2}`) → `pump_invalidate()` →
    `require("mod").v==2` — write + invalidate + re-eval compose into live-edit,
    and `love.load` does **not** re-run (edits change the future, not the past).
  - **The seam documented** — `wasi/platform/EMBEDDING.md` (referenced here): the
    full host-import surface a consumer fulfills (`love_fs` read+write, `love_win`,
    `love_gl`, `love_input`, `love_gamepad`, `love_system`, `love_audio`, the WASI
    shim), the pump ABI + reload entry points and how to drive them, and the
    supported-edit class. It documents the **seam**, not the downstream IDE.

  Built without resolving **D4** (hotswap vs whole-chunk) — the D4=B refinement
  layers onto step 3 of the reload handshake later without foreclosure. The IDE
  (LoveIDE: editor, git-wasm save flow, agent live-edit UX) is a separate repo
  that consumes this contract — out of scope here. **With 6.7 landed, Step 6 is
  COMPLETE.** The former "step 8" is dropped; "step 7" (`love.thread` via Workers)
  remains a large, separate, design-doc-first, demand-driven step after 6.7.

## Decisions

Each is stated with options, trade-offs, and a recommendation. 6.1 depended on
none of them (the raw seam is shared by every option), so building it did not
front-run any choice. Resolution status (architect-ratified):

| # | Topic | Resolution |
|---|---|---|
| D1 | Filesystem seam | **A — replace the module.** Gates 6.2. |
| D2 | Save-dir backing | **Closed — OPFS, separate untracked namespace, eager-flush (eventual durability, declared).** See below. |
| D3 | Window/context | **A — `setMode` drives the real canvas/context.** Gates 6.3. |
| D4 | Reload granularity | **Open — deferred past Beta.** Not C; between A and B. Beta ships module-granularity live-edit (the 6.7 `pump_invalidate` + write path) with **restart** as the fallback for `main.lua`-direct edits; `main.lua`-direct live-edit (D4=A whole-chunk) is not required for Beta. |
| D5 | Supported-edit class | **A — minimal & explicit**, restart fallback. |
| D6 | Console channel | **A — pure stdio now**, architected so B (host structured tap) can layer on without engine changes. |
| D7 | Archive/`.love` mounting: who unzips | **Open** — host-side JS unzip vs a guest-side zip reader over the in-tree zlib. Directory enumeration (`getDirectoryItems` over `fs_list`) landed pre-step-7; runtime zip mounting is deferred (most browser games do not mount a `.zip` at runtime). See D7 below. |

### D1 — Filesystem seam: replace the module, or keep PhysFS and reseam its IO

The real backend is PhysFS-based (`src/modules/filesystem/physfs/`). Two ways to
back it with the host:

- **Option A — replace the module.** Write a `love::filesystem::Filesystem`
  backend implementing the abstract interface (`filesystem/Filesystem.h`) whose
  `read`/`write`/`getInfo`/`mount`/`setSource`/`setIdentity`/`getDirectoryItems`
  call `love_fs` host imports, plus a matching `File`. Replace the boot stub's
  `luaopen_love_filesystem`.
  - **Pros:** no `src/libraries/physfs` tree in the build (readme already lists
    PhysFS as *replaced at the seams, not compiled*); the host controls every
    path, so the live-edit **invalidate** and the save dir are just host calls;
    smallest wasm; the `.love` and save namespaces are host concepts, not OS
    ones.
  - **Cons:** reimplements the whole `Filesystem.h` surface (a real backend, as
    `webaudio` was); risk of subtle divergence from PhysFS semantics (mount
    ordering, path canonicalization, symlink policy, `.love` zip mounting) that
    the `testing/` corpus must catch.
- **Option B — keep PhysFS, reseam its IO.** Compile `src/libraries/physfs` and
  back it with a `PHYSFS_Io` (or custom archiver) whose callbacks pull bytes
  from the `love_fs` host; provide a writable path for the save dir.
  - **Pros:** PhysFS's real mount/path/zip logic stays verbatim — least semantic
    divergence on the read side.
  - **Cons:** drags the whole PhysFS tree (currently excluded) into the build;
    PhysFS still wants a real writable FS + directory scans via OS calls that
    don't exist on wasm/browser (the save dir needs a shim either way); the
    live-edit invalidate must poke *through* PhysFS's own caching; more
    indirection for no browser-visible benefit.
- **Recommendation: Option A (replace).** It is the readme's committed direction,
  gives the host the clean control the live-edit write/invalidate path needs, and
  avoids dragging PhysFS's OS-dependent write/scan machinery onto wasm. The cost
  (reimplementing the interface) is bounded and directly checkable against the
  `testing/` filesystem suite. **DECIDED — Option A.** Gates 6.2.

### D2 — Save directory (writable) backing — CLOSED

Where `love.filesystem.write` / save data lives. Mechanism, store, layout, and
durability are all settled:

- **Mechanism — host-backed writable namespace** via `love_fs` write imports (not
  a WASI preopen; the browser has no fd layer). Same seam as the read path.
- **Store — OPFS (Origin Private File System).** Chosen over localStorage
  (~5 MB, strings, sync-but-janky — a hard cap desktop doesn't impose, so it
  breaks the tail of games that write user content/replays/worlds) and over
  IndexedDB (would model a filesystem on a key-value store). OPFS *is* a
  per-origin filesystem: large, binary-native, hierarchical — a direct fit for
  `love.filesystem`'s tree + `t.identity`. No permission prompt; needs only a
  secure context (HTTPS/localhost), already met. Requires **no** Emscripten, **no**
  COOP/COEP, **no** SharedArrayBuffer — it lives in the JS host behind the seam,
  exactly like the WebGL2 and WebAudio hosts, so it changes nothing on the wasm
  side.
- **Layout — a separate, untracked namespace,** keyed by `t.identity`, beside (not
  inside) any git-wasm working tree. Save data must never dirty the source repo or
  pollute history; keeping save-dir ≠ source is also the desktop-faithful shape.
  (git-wasm is the *source* axis; the save dir is the *runtime* axis — different
  problems, possibly sharing OPFS as substrate in separate directories.)
- **Durability — eager-flush, eventual durability, declared.** OPFS on the main
  thread is async under a sync `write()`, so the host serves `write`/`read` from
  an in-memory cache and flushes to OPFS asynchronously (flush after each write +
  on `pagehide`/`visibilitychange`; request `navigator.storage.persist()` against
  eviction). Under the project standard this is not a compromise but **the correct
  browser-game behavior held to 100%** — it is exactly how shipped browser games
  persist (Unity WebGL's IDBFS is the same async-flush model). In-session
  read-after-write / `getInfo` / listing behave identically to desktop; the only
  residual is a force-kill within the last-write window, a declared cross-platform
  timing note, shared by every browser game. **True sync durability** (desktop-
  exact) is available *only* via the engine-in-Worker + OPFS-sync-access-handle
  pivot — a deployment-architecture upgrade (not COOP/COEP, not SAB), parked for a
  shipping variant that genuinely needs it; not required here.
- **Scope:** the read/boot path (6.2) needs none of the write path; the save-dir
  write path is its own sub-step, now fully specced by the above.

### D3 — Window / GL-context creation

- **Option A — `love.window.setMode` drives the host** to size the `<canvas>`
  and create the WebGL2 context, then hands that context to the step-4 static GL
  imports.
  - **Pros:** faithful (LÖVE creates its own context, as on desktop); retires the
    `graphics-ext.cpp` fake `setMode`; unblocks `present()`/`captureScreenshot`.
  - **Cons:** the witness harness currently creates the context itself; this
    inverts that — the wasm now asks the host, so the graphics legs must move to
    the real window seam.
- **Option B — keep context creation in the harness**, `love.window` a thin stub
  reporting size. Lower effort, but leaves a permanent fake in the graphics path
  and never witnesses the real create.
- **DECIDED — Option A**, at 6.3 — the point of step 6 is to *build* the seam
  graphics faked.

### D4 — Reload granularity (live-edit): whole-chunk re-eval vs. function-body hotswap

The mechanism that must satisfy the reload invariant (below). A file-scope
`local` is both how a *tuning constant* (`GRAVITY`) and *evolved state* (`score`)
are written, and Lua can't tell them apart syntactically.

- **Option A — whole-chunk re-eval.** Re-run the edited chunk; reassign its
  functions/globals.
  - **Pros:** dead simple, deterministic.
  - **Cons:** resets file-scope locals → violates the invariant for state held
    there; a `local x` assigned in `love.load` becomes nil (load isn't re-run) →
    crash. Safe only if game state lives in tables/globals the chunk top level
    doesn't overwrite.
- **Option B — function-body hotswap** (rxi's `lume.hotswap`): load the new chunk
  sandboxed, copy new function bodies into the old function objects, preserving
  upvalues/state.
  - **Pros:** preserves live state; satisfies the invariant for the tuning /
    update / draw case ("notebook magic").
  - **Cons:** leaky at the edges (new/removed upvalues, changed function identity
    held by live references, added/removed functions); needs the debug library.
- **Option C — convention + re-eval.** Require state to live in a designated
  table populated in `love.load` (not re-run); re-eval reassigns functions +
  top-level constants but never that table.
  - **Pros:** simpler than full hotswap; predictable; teachable.
  - **Cons:** imposes a game convention; non-conformant games fall back to
    restart.
- **OPEN — not C; between A and B; needs more discussion.** Restart is the blessed
  fallback for whatever the chosen mechanism can't apply. Post-step-6; blocks
  nothing in step 6.
- **Beta disposition (architect-ratified):** deferred past Beta. Beta ships the
  6.7 mechanism as-is — module-granularity live-edit (`pump_invalidate` + the
  write path, live for `require`'d game modules) with **restart** as the honest
  fallback for `main.lua`-direct edits (which `main.lua` is not `require`'d, so
  the whole-chunk D4=A path would be needed to make live). `main.lua`-direct
  live-edit is explicitly **not** a Beta requirement; the interactive shell
  demonstrates module-granularity live-edit only. D4 is revisited if/when a
  consumer needs `main.lua`-direct hotswap.

### D5 — Supported-edit class (live-edit): what is guaranteed live

- **Option A — minimal & explicit:** function-body edits to callbacks and the
  functions they call, plus file-scope constant literals. Everything else →
  restart.
  - **Pros:** small, predictable, documentable; the invariant holds by
    construction; matches "fine-tuning variables" as the intended use.
  - **Cons:** the IDE must classify an edit's tier (and offer restart for the
    rest).
- **Option B — attempt-any, restart-on-failure.** Try every edit live; restart
  only when hotswap throws.
  - **Pros:** fewer explicit restarts.
  - **Cons:** silently keeps stale state on edits that *appear* to apply but
    shouldn't — the failure mode the invariant exists to forbid.
- **DECIDED — Option A** — the invariant wants a *classifier*, not best-effort.
  Restart is the correct answer for anything outside the class.

### D6 — Console / diagnostic channel shape

The agent needs sight on a live game's output, and (the architect's ask) some
control over what's included — kept faithful.

- **Option A — pure stdio.** `print` → fd 1, errors → fd 2, host taps both. No
  new API.
  - **Pros:** perfectly faithful; already how WASI works; zero divergence.
  - **Cons:** unstructured; no verbosity control beyond host-side string
    filtering; callbacks (`keypressed`, …) invisible unless the game prints them.
- **Option B — stdio + host-side structured tap.** Keep `print` faithful; the
  host tags/timestamps/filters lines and optionally taps the pump (it already
  drives `update`/`draw` and sees `love.errorhandler`), so the agent gets a
  richer, filterable signal — the "control what's included," done host-side.
  - **Pros:** faithful game side; the control the architect wants; callback/error
    visibility for the agent.
  - **Cons:** the callback tap needs a hook in the pump; more host code.
- **Option C — a game-facing `love.log()` API.** **Rejected:** a divergence that
  breaks on other engines unless it degrades to `print`.
- **DECIDED — Option A now, architected toward B.** Ship pure stdio (`print` stays
  `print`, host taps fd 1/2); keep that tap a single clean seam so B's structured/
  verbosity/callback layer can be added **host-side** later with no engine change,
  if A proves insufficient. The stdio half exists already (the witnesses read
  fd 1).

### D7 — Archive / `.love`-zip mounting: who unzips (OPEN)

Splitting the two features PhysFS used to provide behind `love.filesystem`:

- **Directory enumeration** (`getDirectoryItems`) — **landed** (pre-step-7). A
  new `fs_list` host import (size-then-fill, mirroring `fs_read`) returns the
  immediate children of a directory; the host merges the read-only project and
  the writable save namespace and de-dupes, exactly the merged listing PhysFS
  gave across a mounted search path. Games call this routinely (asset discovery,
  level lists), so it was the clearly-needed half. Witnessed:
  `wasi/platform/run-fs-list.sh`.
- **Archive / `.love`-zip mounting** (`mount*`) — **still deferred, and it forces
  a decision**, because PhysFS's zip archiver is gone. Two use cases:
  - *`.love`-as-source* (boot-time) — the host already presents the project
    pre-unpacked (the `.love` pillar); no engine work needed. Low priority.
  - *runtime `mount` of a zip asset* — a running game mounting a downloaded or
    generated archive. This is the one that forces the choice:
    - **Option A — host unzips in JS.** `mount` becomes a host import; the JS
      host decodes the zip (browser `DecompressionStream` or a JS unzip) and
      exposes the entries as a new namespace layer. No zip code in wasm (the
      D1=A ethos); but `mount(Data*)` mounts an in-wasm-memory blob, so its bytes
      would have to cross back out to JS to decode.
    - **Option B — guest-side zip reader over the in-tree zlib.** `wasi/vendor/zlib`
      is already in the tree and gives raw `inflate`; a small zip-central-directory
      reader on top keeps mount ordering guest-side and handles `mount(Data*)`
      without a round-trip — but partially rebuilds the archive machinery D1=A
      shed.
  - **OPEN.** Not resolved: most browser games don't runtime-mount a zip, so the
    corpus is the right evidence for which use case (and thus which option) is
    worth building. Recorded here so the choice is deliberate, not defaulted.

## Resolved by the reload invariant (recorded as decided, not open)

The architect set the reload contract:

> **`reload(edit)` at state S ≡ a fresh run of the new code that has reached S.**
> Live edits change the **future, not the past**; if you break your code and save
> mid-run, it breaks — exactly as a fresh run of broken code would.

Two questions fall out as *decided*:

- **Error containment: dropped.** A broken save breaks the game; LÖVE's error
  screen appears, same as always. No last-good rollback, no containment mode, no
  divergence — the *more* faithful choice.
- **Does `love.load` re-run on reload? No.** A fresh run reaching S ran `love.load`
  once, in the past; re-running it would violate the invariant. Only the
  per-frame path picks up edits. (Corollary: an edit to already-executed init has
  no well-defined "same state" — the trajectory would diverge — so it simply does
  not manifest until a real restart. Consistent with the rule.)
