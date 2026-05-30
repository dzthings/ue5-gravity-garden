# Gravity Entity Garden

A reusable **Unreal Engine 5.7 C++ plugin** (`GravityEntity`) for procedural gravity/magnetic field-beings, validated inside a small experimental "Garden" test project.

> **Governing question:** Can a few procedural entities — mass nodes, field links, spring physics — feel *alive, distinct, and worth interacting with* before any game is defined?

---

## What this is

Each entity is **nodes + links + solvers + procedural geometry + parameterized materials + idle breath**, all surfaced through a shared **state-channel interface**. Not a mesh. Not a physics ragdoll. The field *is* the nervous system — visual and behavioral.

- Part shapes generated in-engine via **Geometry Script** (no imported meshes)
- Motion driven by a **custom C++ spring-damper** (no Chaos/physics sim)
- Per-node material state via **Custom Primitive Data** (no unique MID per node)
- Idle life ("breath") is a signal — geometry and glow are both readouts of it
- Every subsystem is *procedural foundation + authored override + a clean signal/param seam*

---

## Repo layout

```
GravityEntity/                    # the plugin (never depends on Garden)
  Source/
    GravityEntityRuntime/         # runtime module — actors, solvers, profiles, state channels
    GravityEntityEditor/          # editor-only — detail customizations, validators
  Content/
    Profiles/  MasterMaterials/  Niagara/  Debug/

Source/GravityGarden/             # Garden test project (depends on plugin only)
Prompts/
  CLAUDE.md                       # agent operating instructions
  PROJECT_BRIEF.md                # full design spec and milestone sequence
DECISIONS.md                      # log of non-default architectural choices
```

---

## Docs

| Document | Purpose |
|---|---|
| [PROJECT\_BRIEF.md](Prompts/PROJECT_BRIEF.md) | Full design spec — architecture, constraints, milestone sequence, v0 defaults |
| [CLAUDE.md](Prompts/CLAUDE.md) | Agent operating instructions and naming/architecture rules |
| [DECISIONS.md](DECISIONS.md) | Log of non-default decisions made during development |

---

## Milestone status

| Milestone | Status | Notes |
|---|---|---|
| **Spike 0** — throwaway debug spring chain | ✅ Done — deleted | Motion confirmed in PIE; Spike 0 code removed |
| **M0** — plugin skeleton + profile asset | ✅ Done | Plugin enables clean; empty profile opens in Details panel |
| **M1** — one living Worm spine | ✅ Done | Motion gate passed — spine reads as kinetic/alive. Spring-damper chain, attention spring, autonomous locomotion, traveling breath wave, role-colored debug draw, state channels populated |
| **M2** — variation | ✅ Code ready | `UGravityOrbitalTopologySolver` (Fibonacci sphere) + `UGravityOrbitalMovementSolver` (Lissajous core drift, per-node tilted orbit axes); place second pawn with `DA_OrbitalProfile` alongside Worm |
| **M3** — live tuning + glow | ⬜ Not started | PostEditChangeProperty rebuild, glow breath on state channel (after M1 motion gate) |
| **M4** — parametric parts + materials | ⬜ Not started | Geometry Script part kit, ISM/HISM, bounded master materials |
| **M5** — portability | ⬜ Not started | Plugin drops into a second project with zero Garden deps |
| **M6** — resonance v0 | ⬜ Not started | Hold-to-resonate, breath/glow sync as the visible payoff |

> **M1 human gate (cannot be self-certified):** with placeholder geometry and *no glow*, the spine must read as alive through motion + authored breath alone. Glow and materials enter only after this gate passes.

---

## Stack

- **Engine:** Unreal Engine 5.7
- **Language:** C++ (runtime); Blueprint for thin glue/test wiring only
- **Geometry:** Geometry Script (generated in-engine, never per-frame)
- **Materials:** hand-authored master materials + MIDs + Custom Primitive Data
- **Effects:** Niagara (field links, motion trails)
- **Platform:** Windows PC, single-player

---

## Development model

This is a **human-in-the-loop** project. The agent writes C++ and ships instrumentation (debug draw, cvars, Details-panel sliders, test maps). The developer builds in Visual Studio / Rider and judges motion/feel in PIE. The agent cannot run the editor or self-certify visual results.

CLI build (if needed — confirm engine path first):
```
{UE_ENGINE_PATH}/Engine/Build/BatchFiles/Build.bat GravityGardenEditor Win64 Development "{PROJECT_ROOT}/GravityGarden.uproject"
```
