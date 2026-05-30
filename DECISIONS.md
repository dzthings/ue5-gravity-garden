# DECISIONS.md — Non-default decisions log

> Log entries here when a choice deviates from §4 v0 Defaults or is otherwise non-obvious.
> Format: **Date · Area · Decision · Reason**

---

## 2026-05-30 · Project setup · Module named `GravityGarden` (not `GravityEntityGarden`)

The Unreal project was already scaffolded with the module name `GravityGarden`. The brief uses `GravityEntityGarden` as the project name but the actual `.uproject` and source module are `GravityGarden`. Keeping as-is — renaming would require regenerating project files and is purely cosmetic.

---

## 2026-05-30 · Locomotion · Ground locomotion is entity behavior, not game design

The brief's "compression/release locomotion" and "movement is the primary identity axis" were initially implemented as a mid-air circular attention target (placeholder). User correctly identified that a worm crawling on a surface is a core identity behavior, not a game feature.

Decision: locomotion style lives on the movement solver, not in shared infrastructure. `UGravityWormMovementSolver` constrains nodes to a ground plane and walks the attention target along the surface. Floating entities (`UGravityOrbitalMovementSolver`) have no ground constraint — they own their own locomotion model. Each future family declares its own.

---

## 2026-05-30 · Collision · Ground constraint is flat-plane only — terrain collision deferred

Current `bGroundConstrained` clamps nodes to a fixed `GroundZ` (set from spawn height). Nodes pass through actual level meshes. Known limitation, intentionally deferred — motion identity is being validated on flat ground first.

Fix when needed: per-node downward raycast in `StepSimulation`, use hit Z as floor instead of fixed `GroundZ`. ~One raycast per node per sub-step, acceptable cost for 8–12 nodes. Target milestone: M5 or when non-flat terrain testing begins.

---
