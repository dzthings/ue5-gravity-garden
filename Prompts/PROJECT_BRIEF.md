# Gravity Entity System — Project Brief
## Claude Code kickoff document — Rev 3 (parametric geometry + parameterized materials + state channels + two-source breath)

**Purpose of this file:** Seed a Claude Code project and let it produce a build plan with a high chance of succeeding. It is a directive brief, not a discussion document. Read it fully before planning.

**Target:** Reusable Unreal plugin defining a procedural substrate for gravity/magnetic field-beings, validated inside a small experimental "Garden" project.

**Rev history:** R1 plugin/Data-Asset architecture · R2 fully parametric geometry, Blender removed · R3 parameterized materials, state-channel interface, two-source breath · **R3.1 adds Spike 0: a throwaway debug-draw moving spine, seen before any architecture.**

---

## 0. How to use this document (instructions to the agent)

1. Read the whole brief before writing any plan or code.
2. Begin with **Spike 0** (§10) — a throwaway visual spike — then produce a plan for **M0 and M1 only**. Later milestones are intentionally coarse.
3. Treat **§3 Fixed Decisions** as hard constraints. Don't propose alternatives unless you find a concrete blocker; if so, flag it.
4. For anything unspecified, use **§4 v0 Defaults**. If no default fits: pick the simplest *reversible* option, proceed, log it in `DECISIONS.md`. Pause only for irreversible/architecture-defining forks.
5. You cannot judge "feels alive," and you cannot run the Unreal Editor to evaluate motion. At each visual milestone your deliverable is the entity **plus instrumentation** (debug draw, cvars, sliders, a test map) that lets a human judge in under a minute. Optimize for human-in-the-loop iteration, not self-certification.
6. Keep the project building at all times; prefer small compilable increments.

---

## 1. What we're building

**Thesis:** The entity is parametric at every layer, and every visible layer is a *readout of the same field state*. One `UGravityEntityProfile` describes part shape, arrangement, motion, field, **surface/material**, and **idle life (breath)** — with no external art assets. The field is the spring network, the nervous system, and the motion engine; the geometry, the glow, and the breath are all expressions of its state.

**First deliverable (not a creature generator):** a reusable Unreal plugin that produces a *small family of highly distinct* field-being variants, validated in a standalone Garden project.

**The governing gate:** Can a few procedural entities of mass nodes and field links feel alive, distinct, and worth interacting with *before* any game is defined? If no, narrow or stop. If yes, it can later become a game, installation, simulation, or reusable tech. Do not build toward "a game" yet.

**Intended breadth:** ~4–6 topology *families*, each a bounded kit of parametric parts, varied by parameters, plus authored role identity. **Not** open-ended generation, at the part or body level. Depth over breadth.

---

## 2. Environment & stack

- **Engine:** Unreal Engine **5.7** (latest stable). Ignore UE6 (announcement only).
- **Language:** C++ for all runtime systems. Blueprint only for thin glue/test wiring.
- **Geometry:** generated in-engine via **Geometry Script / Modeling Mode**. No external DCC, no FBX/glTF part import.
- **Materials:** a small bounded set of **hand-authored master materials**, varied per creature via **Material Instances + Custom Primitive Data** (see §8). Geometry is *generated*; materials are *parameterized* — different mechanisms.
- **Effects:** Niagara for field links and motion trails.
- **Build:** Unreal Build Tool via the editor / Visual Studio.
- **Source control:** Git; keep `.uasset` churn low; rely on validation + naming discipline.

### Assumptions — correct in one line if wrong
- Platform **Windows / PC desktop**, single dev or small team.
- **Single-player only** (a procedural node-sim is not built to replicate).
- Standalone hobby/R&D project, not bound to an existing codebase.

---

## 3. Fixed decisions (hard constraints)

- Plugin-first; runtime + editor modules. Garden depends on the plugin; **the plugin never depends on the Garden.**
- A variant is `UGravityEntityProfile : UPrimaryDataAsset`; the Unreal asset is the source of truth.
- Solvers, part generators, **and material/breath producers** are `Instanced, EditInlineNew` UObjects configured in the Details panel. One pattern across the system.
- **Geometry is generated** (Geometry Script), profile-driven. No imported static part meshes for core families.
- **Materials are parameterized, not generated** — author a few master materials by hand, vary them per creature via parameters. Never author shaders at runtime.
- **State channels are a first-class interface.** The solver exposes named runtime signals (tension, charge, velocity, resonance phase, instability, attention, breath phase/amplitude). Geometry, materials, breath, and effects *consume* these channels rather than recomputing state. Define the channel interface in M1.
- **Breath is a signal, not a behavior:** per-node phase + amplitude, produced behind one interface by two interchangeable sources — authored and emergent — combined by a blend weight. Authored is the default/shipping path; emergent is an isolated experiment (see §9).
- Authoring happens **in-engine against the real runtime solver.** No external previews.
- **Movement is the primary identity axis.** Identifiable by motion feel alone within 30s, with placeholder geometry.
- Hybrid model: procedural/emergent makes the *substrate*; authored makes *identity*. **Build humanoid last.** Custom C++ spring-damper first. Design *for* runtime morphology; don't build it yet.

**Signature pattern (use as a decision heuristic):** every behavioral subsystem is *procedural/emergent foundation + authored override + a clean signal/parameter seam between them.* When a new "author it or generate it?" question arises, the default answer is **both, with the seam in the middle.**

---

## 4. v0 Defaults (use these; do not stall)

| Question | v0 Default | Door open for |
|---|---|---|
| Justification | Capability first, cost-saving second | Re-evaluate after M2 |
| Part geometry | Parametric Geometry Script generators, generated once + cached + instanced | DCC hero parts later, only where generators can't express a form |
| Materials | Bounded master materials + Custom Primitive Data; debug-simple look until M4 | Membrane/translucent shaders, advanced tech-art later |
| State channels | Define the named-signal interface at M1 (solver already computes most) | More channels added as needed |
| Breath producer | **Authored** (sine + per-segment phase offset + seeded jitter); blend weight = authored | Emergent producer behind same interface, after isolated prototyping |
| Glow breath | Emissive bound to breath-phase channel; **added after the M1 motion gate passes** | Phase-decoupled glow (lead/lag), traveling core→link glow |
| Morphology | Fixed variants in v0; architecture preserves morph option | Runtime topology + part-param swaps later |
| Player role | Observer + possession only | Game/sim/installation direction later |
| Resonance | Triggered (hold-to-resonate) | Player-performed tuning; breath/glow sync as the readout |
| Solver | Custom C++ spring-damper | Chaos/Control Rig/MassEntity later |
| Source of truth | Data Assets + validation | Exported JSON snapshots if a team needs diffs |

**Why generated geometry but parameterized materials:** parts are abstract hard-surface forms (cheap to generate, and per-variant shape variation is the point). Shaders are not generated at runtime — you author a few masters and drive params, which is easier, more robust, and where the "looks good" effort actually lives (a few times, not per creature).

**Why authored breath is the default and emergent is an experiment:** a passive spring-damper system *decays to rest* — it does not breathe on its own. Emergent breath requires a deliberately designed self-excited oscillation (limit cycle) plus coupling, which is the least controllable thing in the project. Authored sine is reliable and ships M1; emergent is prototyped in isolation and adopted only if it lands in the alive-not-broken zone (see §9).

---

## 5. Architecture & module layout (scaffold to this)

```text
GravityEntity/                       # the plugin
  GravityEntity.uplugin
  Source/
    GravityEntityRuntime/
      AGravityEntityPawn             # owns node array, ticks subsystems
      UGravityEntityComponent
      UGravityEntityProfile          # UPrimaryDataAsset — the variant
      UGravityTopologySolver         # abstract, Instanced/EditInlineNew
      UGravityMovementSolver         # abstract, Instanced/EditInlineNew
      UGravityPartGenerator          # abstract, Instanced/EditInlineNew
        (Plate/Arc/SpineSegment/Sphere/Ring/Strand ...)
      UGravityPartCache              # param-hashed generated-mesh cache
      UGravityStateChannels          # named runtime signals (the interface)
      UGravityMaterialProfile        # base look + state-channel -> param bindings
      UGravityBreathSignal           # interface: per-node phase + amplitude
        UGravityBreathAuthored       #   sine/curve producer (default)
        UGravityBreathEmergent       #   coupled-oscillator producer (experiment)
      UGravityResonanceComponent
      UGravityFieldRenderer          # Niagara links + trails
      (node graph / spring-damper / runtime validation)
    GravityEntityEditor/
      Detail customizations · preview tooling · UEditorValidatorBase
      (Forge editor — LATER, do not build now)
  Content/
    Profiles/  MasterMaterials/  Niagara/  Debug/
GravityEntityGarden/                 # test project, depends on plugin only
```

---

## 6. Runtime entity model

A Gravity Entity is **nodes + links + part generators + solvers + material profile + breath signal + resonance**, surfaced through **state channels**. Not a mesh.

**Mass node:** stable ID, role, part ref (generator + params → cached mesh), size, transform, target transform, velocity/spring state, attachment state, per-node state values (tension/charge/breath/etc.), optional tags.
Roles (how authored identity survives procedural variation — keep first-class): `Lead, Spine, Core, Orbital, Anchor, Shield, ScoutTip, LeftBranch, RightBranch, Damaged, Detached, Recallable`.

**Field link:** start/end node, tension, stretch, glow intensity, instability, thickness, render data. The link system is the nervous system — visual *and* behavioral, and a primary glow carrier.

**State channels:** the solver already computes tension, charge/compression, velocity, instability; resonance and breath producers add the rest. Expose them as a named interface that geometry offsets, the material profile, the field/trail effects, and secondary motion all read. One source of state, many consumers.

**Part generation flow:** profile part params → Geometry Script builds mesh → cache by param hash → instance via ISM/HISM. Generate at construction, never per frame.

**Preserve morphology without building it:** stable node IDs, role-preserving rebuilds, solver/breath state reset+blend hooks, component pool reuse, re-resolvable part params.

---

## 7. Variant asset & parametric parts

`UGravityEntityProfile` carries: topology type, node graph, solver stack, **part kit** (parametric generators + params by role), **material profile**, **breath config**, resonance profile, size/spacing/stiffness/damping curves, role assignments, authored constraints, validation rules.

Part generators are `Instanced, EditInlineNew` UObjects with clamped `UPROPERTY` shape params: PlatePart (length/width/thickness/bevel/taper/mirror), SpherePart, ArcPart, SpineSegmentPart, RingPart, and a thin **StrandPart** for tendrils/claws/tails/tentacles (the image's manta tail, stalker claws, jelly strands). Bounded kit — parametric depth, not arbitrary geometry.

Deriving a Guardian from a Scout = same generators, broader/thicker params, different topology + movement solver. No new assets. **DCC is an optional late escape hatch** for hero forms only; core families stay generated.

---

## 8. Parametric materials & state channels

Geometry is generated; **materials are parameterized.**

- **Master materials (bounded set):** e.g. `M_PlateMetal`, `M_GlowCore`, `M_Membrane`, `M_FieldLink`. Hand-authored shader logic, rich exposed params (base metal/roughness/reflection, emissive color+intensity, rim/fresnel, pulse, instability noise). Don't proliferate masters.
- **Per-creature variation:** Material Instances (MIC static look, MID for runtime-changing params).
- **Per-node state:** **Custom Primitive Data / PerInstanceCustomData** feeds per-node float state to the material without a unique MID per node — required since plates are ISM/HISM-instanced. This is how a tense node brightens or a damaged one flickers, cheaply.
- **Shared/global field state:** **Material Parameter Collections** for values many materials read at once (world field-pulse, shared resonance state).
- **Material profile = base look + bindings from state channels to material params.** Author the response once in the bindings; every variant inherits it. The surface is another readout of the field, not a separately animated skin.

Glow still needs project-level bloom/post; reflections need the usual metallic/roughness setup. Params give per-creature variation *within* that global setup.

---

## 9. Idle life — breath as a two-source signal

Idle micro-motion is most of what sells *alive*. With no faces, the segments carry resting life. Breath is **central, not polish.**

- **Breath = a signal:** per node, a phase + amplitude. Consumers read it: geometry (subtle scale / gap / tilt offsets, additive on top of solver targets) and material emissive (glow). One signal, two expressive layers.
- **Two producers behind one interface:**
  - **Authored (default, ships M1):** sine + **per-segment phase offset** (breath becomes a traveling peristaltic wave, not a mechanical synchronized pulse) + **seeded per-node jitter** in rate/phase (organic, not lockstep).
  - **Emergent (experiment, later):** coupled self-excited oscillators. Reality check for the agent: *passive springs decay; they do not self-oscillate.* Emergence requires a designed limit cycle (van der Pol–style energy injection at low amplitude) plus inter-node coupling so phase waves and resonance-sync *arise* rather than being scripted. Least controllable subsystem — **prototype in isolation** (single chain, sliders on damping/coupling/injection) before adoption.
- **Blend, don't switch:** combine via a weight, and have the authored layer **modulate the emergent producer's parameters** (target frequency, amplitude, phase reset) rather than override its output — a hard cut from a free-running limit cycle to an imposed curve will pop. "Emergent always running, authored steering when a moment needs it."
- **Glow breath is the cheapest, highest-impact channel.** Emissive on the breath-phase channel via Custom Primitive Data — no physics risk, gentle. Likely delivers most of the alive read for almost nothing. **Add it only after the M1 motion gate passes**, so it can't mask weak motion. Then: decouple glow phase from geometry phase so glow *leads* motion = anticipation; let glow travel core→links→plates = a circulatory heartbeat.
- **Breath as a face-substitute:** rate/depth/regularity = emotional/health readout (calm/agitated/dormant/arrhythmic-when-damaged). On successful resonance, two entities sync breath + glow (and drift color toward each other) — the most legible-at-distance signal of connection, and at population scale a field of out-of-phase pulsing lights is the mesmerizing "watch for ten minutes" garden.
- **Per-family character:** Worm breathes in traveling waves; Scout shallow/quick; Lattice Ancient slow field-ripples; for the Shield Jelly, breath *is* the locomotion primitive.

---

## 10. First increment — Spike 0 → M0 → M1

### Spike 0 — Throwaway visual spike (before M0)
**Purpose:** the fastest possible *moving* entity on screen, so a visual thinker can see where this goes before any clean architecture exists. Sacrificial by design.
One self-contained C++ actor in a throwaway test level: an array of nodes (position + velocity), a spring-damper on Tick, `DrawDebugSphere` for the masses and `DrawDebugLine` for the field links, and 2–3 `UPROPERTY(EditAnywhere)` floats (stiffness, damping, spacing) tunable live in the Details panel during PIE. No plugin, no profile, no part generators, no Geometry Script, no materials, no state channels.
**Verify:** runs in PIE; a spine of debug spheres moves; dragging the sliders changes stiffness/spacing live.
**Then throw it away.** Its only job is to be *seen*. Do **not** carry its code into M0 — M0 starts clean (see §11).

### M0 — Contract & skeleton
plugin + runtime/editor stubs; `UGravityEntityProfile`; abstract topology/movement/part-generator/breath bases; **`UGravityStateChannels` interface**; part-cache + validation stubs; preview pawn shell.
**Verify:** compiles; plugin enables clean; an empty profile opens in the Details panel. No visuals yet.

### M1 — One living spine
one Worm profile; node spawning along a spine; spline/field links; custom spring-damper motion; compression/release locomotion; directional attention; **authored idle breath (geometry only)**; the **state-channel interface populated** by the solver (tension/charge/velocity at minimum — nearly free); debug sliders/cvars; **trivial placeholder geometry only** (one crude generated box per node), **debug materials only (no glow)**.
**Verify (agent-checkable):** compiles/loads; `ge.SpawnWorm` or a placed pawn spawns it; debug draw shows nodes+links with role colors; Details/cvar tuning of stiffness/spacing/node-count updates live; state-channel values logged.
**Human gate (cannot self-certify):** with placeholder geometry and **no glow**, the spine reads as alive through motion + authored breath alone — anticipation, compression, release, lag, overshoot, recoil, settle, directional attention, idle breathing. **If it doesn't feel alive here, do not add glow, materials, or Niagara to rescue it — stop and rethink.** Glow/materials enter only after this gate passes.

---

## 11. Guardrails (do-not list)

- **Spike 0 is sacrificial — delete it.** Do not build M0+ on top of the spike, and do not invest in making its code clean; it exists only to be seen, then discarded.
- Do **not** build the Forge editor yet (Details panel + preview actor is the M0–M3 surface).
- Do **not** use a physics sim for core motion in M0/M1; custom solver only. Don't let the first proof become a physics-debugging project.
- Do **not** build the full part kit before motion is proven; M1 uses one trivial generated box per node.
- Do **not** import static DCC/FBX meshes for core families. Do **not** regenerate dynamic meshes per frame (generate once, cache by hash, instance). Do **not** make parts arbitrary geometry.
- Do **not** author shaders at runtime; materials are parameterized off a bounded set of master materials. Per-node state via **Custom Primitive Data**, not a unique MID per node.
- Do **not** add glow/materials to M1 before the motion gate passes — they must not mask weak motion.
- Do **not** build emergent breath in the main path; authored sine + (post-gate) glow is the shipping path. Emergent is a **separate isolated prototype**; if adopted, blend by parameter-modulation, never a hard output cut.
- Keep breath **subtle** — small amplitude, slow rate. Everything pulsing at once is nauseating; restraint wins.
- Do **not** build runtime morphology, networking, or a JSON authoring pipeline. Don't over-randomize variants.
- Every milestone after M1 must add at least one *controllable/influenceable* behavior, not just visual behavior (screensaver trap).

---

## 12. Milestone roadmap (coarse — detail only when reached)

- **M2 — Variation.** Worm + Orbital Cluster (maybe Shield) from one pawn class, different solvers, multiple instances, possession cycling. **Gate:** each variant feels different within 30s.
- **M3 — Live tuning + glow.** `PostEditChangeProperty` preview rebuild; cvar/debug panel; curve tuning; profile duplication. Add **glow breath** on the state channel now that motion is proven; phase-decouple it for anticipation.
- **M4 — Parametric parts + materials.** Geometry Script kit (plate/arc/spine/sphere/ring/strand) + part cache + ISM/HISM; the bounded master-material set + material profile (state-channel bindings). Replaces placeholder boxes.
- **M5 — Portability.** Validation pass; clean plugin content; drop into a second project; spawn a profile with zero Garden deps.
- **M6 — Resonance v0.** Resonance component, frequency/pulse params, proximity, hold-to-resonate, one success + one mismatch response, **breath/glow sync as the visible payoff.**
- **Experiment (parallel, isolated, not on the critical path): emergent breath.** Single-chain dynamics sandbox; adopt behind the breath interface only if it reliably lands subtle-and-stable.
- **Later (do not plan now):** evolved humanoid; runtime morph; membrane surfaces (Jelly/Manta); detach/recall; group resonance; ancient lattice; ecology; authored signature animations; optional DCC hero parts; the Forge; game-specific camera/control; combat.

---

## 13. Working conventions (can become the repo CLAUDE.md)

- **Naming:** `AGravityEntity*`, `UGravity*`, `FGravity*`. Solvers end `Topology`/`Movement`; part generators end `Part`; breath producers `UGravityBreath*`.
- **Pattern consistency:** solvers, part generators, material profiles, breath producers are all `Instanced, EditInlineNew` UObjects with clamped `UPROPERTY(EditAnywhere)` params. One mental model.
- **State channels are the seam.** New behavior reads/writes named channels; consumers never recompute another subsystem's state. Geometry, materials, breath, effects all hang off the channel interface.
- **Decision heuristic:** procedural/emergent foundation + authored override + signal/param seam. Author-or-generate questions default to *both, with the seam between.*
- **Part caching:** hash by generator + params; identical parts share one mesh + one ISM/HISM; never per-frame.
- **Placement/reuse:** runtime in `GravityEntityRuntime`; editor-only in `GravityEntityEditor` (compiles out of shipping; no editor includes in runtime). If code references the Garden, it's misplaced.
- **Style:** Unreal C++ conventions (UCLASS/USTRUCT/UPROPERTY, `TObjectPtr`). Malformed profiles fail loudly in-editor via validation.
- **Decision protocol:** unspecified → §4 default → else simplest reversible choice, logged in `DECISIONS.md`. Pause only for irreversible/architecture-defining forks.
- **Verification:** keep it compiling; ship a test map + debug instrumentation for every visual feature; state plainly when something needs human eyes.

---

## 14. One-paragraph summary for the agent

Build a reusable UE 5.7 C++ plugin whose core asset, `UGravityEntityProfile`, fully describes a creature in parameters — part shapes (generated by Geometry Script), arrangement, motion (custom spring-damper), a bounded set of parameterized master materials, and idle breath — all surfaced through a named **state-channel interface** that every visible layer reads. Prove first, with a single Worm in placeholder boxes and debug materials, that **motion plus authored breath alone read as alive**; do not add glow or materials until that gate passes. Then expand to a few distinct families with parametric parts and materials, add glow on the breath channel, and a triggered resonance whose visible payoff is breath/glow sync. Treat emergent (self-oscillating) breath as an isolated experiment behind the breath interface, never on the critical path. Everything is procedural/emergent at the foundation, authored at the override, with a clean signal/parameter seam between — and identity comes from roles, topology, and motion. Don't over-build ahead of the gates.
