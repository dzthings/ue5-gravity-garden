# Gravity Entity Garden

A reusable **Unreal Engine 5.7 C++ plugin** (`GravityEntity`) for procedural gravity/magnetic field-beings, validated inside a small experimental "Garden" test project.

> **Governing question:** Can a few procedural entities — mass nodes, field links, spring physics — feel *alive, distinct, and worth interacting with* before any game is defined?

---

## What this is

Each entity is **nodes + links + solvers + procedural geometry + parameterized materials + idle breath**, all surfaced through a shared **state-channel interface**. Not a mesh. Not a physics ragdoll. The field *is* the nervous system — visual and behavioral.

- Part shapes generated at runtime via **ProceduralMeshComponent** with hand-computed geometry (sphere, capsule, cylinder) — no imported meshes
- Motion driven by a **custom C++ spring-damper** (no Chaos/physics sim)
- Per-node glow driven by **Material Instance Dynamics** (one MID per component) with `GlowValue` parameter set from breath phase + tension each tick
- Idle life ("breath") is a signal — geometry offset and glow are both readouts of it
- Every subsystem is *procedural foundation + authored override + a clean signal/param seam*
- Each entity **duplicates the profile's solver and breath signal** at spawn — independent runtime state per instance from a shared profile asset

---

## Repo layout

```
GravityEntity/                    # the plugin (never depends on Garden)
  Source/
    GravityEntityRuntime/         # runtime module — actors, solvers, profiles, state channels
    GravityEntityEditor/          # editor-only — detail customizations, validators

Source/GravityGarden/             # Garden test project (depends on plugin only)
  GravityEntitySpawner            # place one in the level; spawns N entities at random positions
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
| **Spike 0** — throwaway debug spring chain | ✅ Done | Motion confirmed in PIE; deleted |
| **M0** — plugin skeleton + profile asset | ✅ Done | Plugin enables clean; empty profile opens in Details panel |
| **M1** — one living Worm spine | ✅ Done | Motion gate passed. Spring-damper chain, S-curve ground locomotion, authored breath wave, state channels |
| **M2** — variation | ✅ Done | Worm + Orbital Cluster; each reads as distinct within 30s |
| **M3** — glow | ✅ Done | ProceduralMeshComponent node rendering, MID-driven glow breath, traveling wave visible on both families |
| **M4** — parametric parts + materials | ✅ Done | Capsule segments (Worm), spheres (Orbital), glowing link tubes, `UGravityMaterialProfile` (tension+breath→glow) |
| **Infra** — spawner + entity independence | ✅ Done | `AGravityEntitySpawner` (profile + count + radius); per-entity `DuplicateObject` solver/breath so all instances are independent; node-node repulsion prevents self-intersection |

### Current focus — Demo Garden

The governing question is answered: entities feel alive and distinct. Effort is now on making the garden compelling enough to share as a concept.

| Work item | Status | Notes |
|---|---|---|
| **Floor material** | ⬜ | Dark reflective surface that makes glow pop |
| **Terrain collision** | ⬜ | Per-node raycast replaces flat-plane ground constraint |
| **Spine improvements** | ⬜ | Taper, smoother curvature, distinctive head shape |
| **Flora** | ⬜ | Rooted/reactive entities — stalks that sway toward passing worms; same node/link/breath/material system, different solver |
| **Post-process pass** | ⬜ | Bloom, vignette, dark atmosphere |
| **M5 — Portability** | ⬜ Deferred | Validation pass; plugin into second project |
| **M6 — Resonance v0** | ⬜ Deferred | Hold-to-resonate, breath/glow sync |

---

## Entity families

| Family | Topology | Locomotion | Status |
|---|---|---|---|
| **Worm** | Linear spine | S-curve ground crawl, node repulsion | ✅ Live |
| **Orbital Cluster** | Fibonacci sphere | Lissajous core drift, tilted per-node orbits | ✅ Live |
| **Flora (Stalk)** | Anchored chain | Reactive — sways toward nearby entities | ⬜ Planned |

---

## Stack

- **Engine:** Unreal Engine 5.7
- **Language:** C++ (runtime); Blueprint for thin glue only
- **Geometry:** `UProceduralMeshComponent` with hand-computed primitives (sphere, capsule, cylinder)
- **Materials:** hand-authored `M_GravityNode` master material + `UMaterialInstanceDynamic` per component; `GlowValue` scalar parameter driven from state channels
- **Platform:** Windows PC, single-player / observer

---

## Development model

Human-in-the-loop. The agent writes C++ and ships instrumentation (debug draw, cvars, Details-panel sliders). The developer builds in **JetBrains Rider** and judges motion/feel in Simulate mode. The agent cannot run the editor or self-certify visual results.
