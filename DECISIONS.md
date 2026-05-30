# DECISIONS.md — Non-default decisions log

> Log entries here when a choice deviates from §4 v0 Defaults or is otherwise non-obvious.
> Format: **Date · Area · Decision · Reason**

---

## 2026-05-30 · Project setup · Module named `GravityGarden` (not `GravityEntityGarden`)

The Unreal project was already scaffolded with the module name `GravityGarden`. The brief uses `GravityEntityGarden` as the project name but the actual `.uproject` and source module are `GravityGarden`. Keeping as-is — renaming would require regenerating project files and is purely cosmetic.

---
