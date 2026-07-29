# Handoff — pre-step-7 "unblock a real game" + the road to Beta

Working session continuity doc. Read this first when picking the work back up.
It records where the project stands, the decisions the architect has ratified,
and the concrete next step. It is not a spec — the specs are `readme.md` (where
we are), `wasi/platform/DESIGN.md` + `EMBEDDING.md` (seam design/decisions), and
`AGENTS.md` (how we work).

## Where we are

**Milestone reached: a real game runs.** The LÖVE engine, compiled to
wasm32-wasi, now runs an actual game end to end in real Chromium — conf → canvas
→ `love.load` → `love.update`/`love.draw` → present — with graphics, filesystem,
sound/audio, and physics all working together in one artifact.

This session cleared the "pre-step-7" module gaps (the modules a real game
actually uses that weren't yet linked) and proved them together. Branch:
`claude/project-roadmap-next-steps-9x9ej2`, cut from `wasi`. Five commits, all
witnessed on real engines and CI-enforced, tree clean and pushed:

| Commit | What | Witness (verified node:wasi + Chromium unless noted) |
|---|---|---|
| `0429373` | `love.physics` linked (in-tree Box2D) | `wasi/platform/run-physics.sh` — a body falls under gravity |
| `6af8399` | `love.sound` decoders linked (Wave/Vorbis/FLAC/MP3/ModPlug) | `wasi/platform/run-sound.sh` — real `clickmono.ogg` → 2927 PCM samples |
| `758bbd9` | `love.filesystem` directory enumeration (`fs_list`) | `wasi/platform/run-fs-list.sh` — merged project+save, de-duped |
| `7c89741` | union "real game" build | `wasi/platform/run-game.sh` (Chromium) — sound + physics + draw together |
| `ee9bd4f` | doc pass — README/DESIGN + CI wiring | — |

All additive: **no new guarded seam** (the count stays ten). These were
module-completion passes over the step-6 platform seams.

### What "a real game runs" means precisely (don't oversell it)
- The game we ran is a ~30-line LÖVE 12 program written **into the witness**
  (`wasi/platform/run-browser-game.mjs`, the `gameConf`/`gameMain` strings) — a
  real, desktop-compatible LÖVE project, but a **test fixture**, not a
  published third-party game.
- It runs **headless in Chromium** via Playwright (a driver asserts pixels +
  stdout markers), **not** an interactive window you can play.
- We have **not** yet run the `testing/` corpus or any third-party `.love`.

Closing that gap — interactive + real games + corpus parity — is the road to
Beta below.

## Beta definition (architect-ratified)

**Beta = runs real games interactively from a standalone dev artifact, with the
sliced `testing/` corpus green modulo declared divergences.** Shipping/packaging
(the single-`.js` form, issue #7 / step 8) is **post-Beta** — "operable on its
own in a limited capacity" ≠ "build every downstream consumer." love-wasi must
be demonstrably operable standalone; it need not be the shipped product to be
Beta.

## The five steps to Beta (decisions ratified this session)

Recommendations below are all architect-approved unless marked otherwise.

### 1. Interactive standalone shell — THE NEXT STEP
A minimal browser page that instantiates the union wasm, wires the existing host
seams to **live** sources, pumps on `requestAnimationFrame`, and shows a real,
playable canvas. Assembly of seams that already exist (6.4 input, 6.7 reload,
`love_gl`/`love_win`/`love_fs`/`love_system`/`love_audio`), not new engine work.
- **Shape:** game-player only (load a `.love`/project + play). **Not** a REPL,
  **not** an editor/save/agent UI (that's the downstream consumer).
- **Live controls:** forward real DOM key/mouse events into the `love_input`
  seam. Input defaults (ratified): forward the common events, `preventDefault`
  on game keys, pause on tab blur; **IME + pointer-lock deferred**.
- **Live-edit:** module-granularity only (the 6.7 `pump_invalidate` + write
  path). `main.lua`-direct live-edit is **deferred** (that's the open D4 call —
  see below); restart is the fallback. Not needed for Beta.
- **Packaging:** stays a **local dev artifact loading the raw `.wasm`** — does
  not touch issue #7 / step 8.
- Needs a test game (see Step 2).

### 2. Real third-party games running
Run actual open-source LÖVE 12 games, not the fixture.
- **Do NOT bundle a game in the repo** (not yet, not for distribution). Keep a
  **local folder of a few small free LÖVE 12 games for testing** — it need not
  live in the repo.
- **Selection criteria:** (a) pure LÖVE inside the linked envelope — no
  `love.thread`, no `love.video`, no networking; (b) **has `expected/` outputs
  in a corpus form where possible** (ties into Step 3's parity).
- **Acceptance bar:** boots + playable + no crash + visually plausible. Pixel
  parity is Step 3's job, not this one.

### 3. Sliced corpus parity vs desktop
Run the `testing/` `love.test.*` suites for the **linked modules** and diff.
- **Reference = the committed `testing/**/expected/` outputs** (no fresh desktop
  run needed; note version-skew risk).
- **Pass threshold:** 100% of the linked-module suites pass, with declared
  divergences marked **expected-fail** (never silently failing). The divergence
  list must be explicit.
- The full corpus can't run in one shot (it exercises unlinked modules —
  `thread`, `video`); run it **by module slice**.

### 4. `love.thread` (build-order step 7)
The last major engine module. **Design-doc-first** — a `wasi/thread/DESIGN.md`
pass that surfaces its own decisions before any code.
- **In-Beta? No — declared Beta limitation.** Most `.love` games don't use it;
  ship Beta with thread as a documented gap, add it after.
- **Architecture (ratified constraint):** **message-passing Web Workers only.**
  `SharedArrayBuffer` / COOP / COEP are ruled out by the project's no-COOP/COEP
  pillar, so the share-nothing Channel path is the only faithful option. This is
  a fixed constraint for the future design doc, not an open fork.
- **Acceptable divergences** from desktop `love.thread` (shared mutable state
  beyond Channels, etc.) — to be enumerated in that design doc for sign-off.

### 5. Declared divergences (stay declared for Beta)
- **Video (Theora):** stays dropped. A future `<video>` seam is the right path.
- **Networking (`enet`/`luasocket`/`luahttps`):** stays absent for Beta. A
  web-native transport (WebSocket/WebRTC — a different API) is a later
  exploration, not Beta scope.
- **Archive / `.love`-zip mounting (D7):** ship **enumeration-only** for Beta;
  leave D7 open (host-JS unzip vs. guest-zlib reader) until a real game needs
  runtime mount. Enumeration (`getDirectoryItems`) already landed this session.

## Open decisions still pending (deliberately)
- **D4** (reload granularity) — deferred past Beta; module-granularity + restart
  is the shipped mechanism. Revisit if a consumer needs `main.lua`-direct
  hotswap. (`wasi/platform/DESIGN.md` D4.)
- **D7** (who unzips a runtime-mounted archive) — open; enumeration shipped,
  mounting deferred. (`wasi/platform/DESIGN.md` D7.)
- **Step-7 divergences** — enumerated when the thread design doc is written.
- **Issue #7** (packaging: single `.js` vs `.js`+`.wasm`) — decided at step 8 by
  measurement; post-Beta.

## Reproducing the witnesses
Each is one command; needs the step-0 sysroot at `$PREFIX` (in an interactive
session `.claude/hooks/session-start.sh` fetches it to `/root/.love-wasi/wasi-eh`),
Node ≥ 24.15, and `playwright-core` + Chromium. Pattern:
`PREFIX=/root/.love-wasi/wasi-eh wasi/platform/run-<name>.sh`.
- `run-physics.sh`, `run-sound.sh`, `run-fs-list.sh` — the three module passes
  (node + Chromium).
- `run-game.sh` — the union real game (Chromium; heavy ~6 min build).
- The new steps are wired into `.github/workflows/witness.yml` (physics/sound/
  fs-list in `pump-witness`; game in `graphics-witness`), so they run on PR/push
  to `wasi`. **Note:** they were verified locally green but have **not** yet run
  in CI (this branch's pushes don't trigger it — only `wasi` and PRs-to-`wasi`
  do), so the first PR to `wasi` is where CI exercises them.

## Immediate next step
Start **Step 1 (interactive shell)**: a minimal player page wiring live input +
module-granularity live-edit onto the union artifact, plus a local folder of a
few small free LÖVE 12 games (Step 2 selection) to drive it. That's the piece
that makes love-wasi *demonstrably* a game you can open and play, standalone.
