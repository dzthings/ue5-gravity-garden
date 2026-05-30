# GravityEntityGarden — CLAUDE.md
## Agent operating instructions

**Read `PROJECT_BRIEF.md` fully before writing any plan or code.**
It contains all design decisions, v0 defaults, architectural constraints, and the milestone sequence. This file summarises conventions only; the brief is the authority.

---

## Project context

Reusable Unreal 5.7 C++ plugin (`GravityEntity`) defining a procedural substrate for gravity/field-beings. This repo (`GravityEntityGarden`) is the host test project. The plugin lives at `Plugins/GravityEntity/`; the Garden project code lives in `Source/GravityEntityGarden/`. **The plugin must never depend on the Garden.**

---

## First task

Build **Spike 0** as described in §10 of the brief before anything else: one self-contained C++ actor in the Garden module, debug-draw spring chain, no plugin yet. Read §10 for the full spec and the explicit instruction to delete it afterward.

---

## Building & the human-in-the-loop

- **Primary build:** Visual Studio / Rider on the developer's machine. You edit; the developer builds and judges.
- **CLI build if needed:** `{UE_ENGINE_PATH}/Engine/Build/BatchFiles/Build.bat GravityEntityGardenEditor Win64 Development "{PROJECT_ROOT}/GravityEntityGarden.uproject"` — confirm the engine path with the developer before running.
- **You cannot run PIE or evaluate motion.** At every visual milestone your deliverable is code plus instrumentation (debug draw, cvars, Details-panel sliders, a test map) that lets the developer judge in under a minute. State this plainly when you reach a checkpoint.
- Keep the project building at all times. Prefer small compilable increments.

---

## Naming conventions

| Thing | Pattern |
|---|---|
| Runtime actor/component | `AGravityEntity*` / `UGravityEntity*` |
| Runtime structs | `FGravity*` |
| Topology solvers | `UGravity*TopologySolver` |
| Movement solvers | `UGravity*MovementSolver` |
| Part generators | `UGravity*Part` |
| Breath producers | `UGravityBreath*` |
| Plugin runtime module | `GravityEntityRuntime` |
| Plugin editor module | `GravityEntityEditor` (editor-only; compiles out of shipping builds; no editor includes in runtime) |

---

## Architecture rules (non-negotiable)

- Plugin-first. Garden code never leaks into the plugin.
- Solvers, part generators, material profiles, and breath producers are all `Instanced`, `EditInlineNew` UObjects — one pattern across the system.
- **Parts are generated** (Geometry Script), **materials are parameterized** (master materials + MIDs). Never author shaders at runtime.
- Per-node state via **Custom Primitive Data**, not a unique MID per node.
- Parts: generate once at construction, cache by param hash, instance via ISM/HISM. Never per-frame.
- **State channels** (`UGravityStateChannels`) are the shared signal interface. Consumers read named channels; they do not recompute another subsystem's state.
- Do not build the Forge editor, runtime morphology, networking, or a JSON authoring pipeline.

---

## Decision protocol

1. Check the brief §4 v0 Defaults — use the listed default.
2. If no default fits: choose the simplest reversible option, proceed, log it in `DECISIONS.md` at the repo root.
3. Pause only for irreversible or architecture-defining choices — flag them explicitly.
4. Never pause for anything already covered by the brief.

---

## Records

- Log all non-default decisions in `DECISIONS.md`.
- If a build fails unexpectedly, note the error and fix before continuing.
