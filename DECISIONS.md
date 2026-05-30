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

## 2026-05-30 · Collision · Ground constraint uses per-node terrain raycast

`UGravityWormMovementSolver` now does a downward `LineTraceSingleByChannel` (ECC_WorldStatic) per node per sub-step. Hit Z is used as the floor; falls back to the flat `GroundZ` plane if no hit. `SetWorld()` is called at entity init so the solver has a valid world reference. Floor plane requires collision enabled — `Engine/BasicShapes/Plane` has no collision by default and must be replaced or have collision added manually.

---

## 2026-05-30 · Geometry · ProceduralMeshComponent instead of Geometry Script

The brief specifies Geometry Script for procedural part generation. `GeometryScript` module was not resolvable via UBT in this UE5.7 install (plugin module lookup failure). Switched to `UProceduralMeshComponent` with hand-computed vertex/triangle/normal arrays for sphere, capsule, and cylinder primitives.

`ProceduralMeshComponent` is a standard engine module, definitely runtime-safe, no plugin dependencies. The geometry is identical in result. Can be migrated to Geometry Script / `UDynamicMeshComponent` later if needed for more complex forms.

---

## 2026-05-30 · Materials · MID per component instead of CustomPrimitiveData

The brief specifies per-node state via Custom Primitive Data (no unique MID per node). In M3–M4, the `PerInstanceCustomData` ISM node was unavailable in the material editor search, and the `CustomPrimitiveData` node was similarly unreliable. Switched to one `UMaterialInstanceDynamic` per `UProceduralMeshComponent`, setting `GlowValue` scalar parameter each tick.

For the entity count in this project (~20–30 components per scene), MID overhead is negligible. The brief's constraint was primarily about ISM at scale (thousands of instances). Revisit if performance becomes a concern.

---

## 2026-05-30 · Architecture · Per-entity solver instancing via DuplicateObject

Initially, all entities using the same `UGravityEntityProfile` shared one solver UObject. The solver stores runtime state (heading angle, attention target, locomotion time) on the UObject, so all instances drove to the same position.

Fix: `UGravityEntityComponent::InitializeEntity` calls `DuplicateObject<UGravityMovementSolver>` and `DuplicateObject<UGravityBreathSignal>` from the profile, storing per-component `SolverInstance` and `BreathInstance`. The profile is config-only; the component owns all live state.

---

## 2026-05-30 · Scope · Demo-garden focus replaces M5/M6 as next priority

After M4, the governing question (do entities feel alive and distinct?) is answered. Rather than proceeding to portability (M5) or resonance (M6), effort shifts to making the garden compelling enough to share as a concept demo: floor material, terrain collision, spine improvements, and a new **Flora** family (rooted/reactive entities using the same node/link/breath/material system with a different solver).

M5 and M6 remain on the roadmap but are deferred until the demo is presentable.

---

## 2026-05-30 · New family · Flora — rooted reactive entities (implemented)

Flora are field-beings that don't locomote — they respond. Implemented via:
- `UGravityFloraTopologySolver`: rooted vertical chain; node 0 anchored, nodes extend upward with seeded lean per stalk
- `UGravityFloraMovementSolver`: node 0 pinned each tick; upper nodes spring toward rest positions with tip-weighted flexibility; attracted toward registered fauna node positions with configurable radius and falloff
- `UGravityFieldRegistry` (`UTickableWorldSubsystem`): fauna GEC broadcasts display positions each tick; flora solvers register at init and receive positions via `SetNearbyEntityPositions`; cleaned up via `EndPlay` unregister

Adds a third environmental layer (ground: worms, air: orbitals, vertical: stalks) and creates spatial memory — stalks settle slowly after a worm passes, making its path readable.

---
