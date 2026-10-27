# Thyme architecture study — patterns for POLITIKA

**Purpose:** extract general RTS engineering lessons from a public C++ source project, then map them to POLITIKA's UE 5.8.3 design. This is an architecture study, not a proposal to port Thyme or its game rules.

**Reference snapshot:** [TheAssemblyArmada/Thyme](https://github.com/TheAssemblyArmada/Thyme), branch `develop`, commit `ccef1e11c1355c6db577a057c06e7d790f1a0333` (2024-10-31). Local checkout: [`/mnt/data/CommandAndConquer/Thyme`](</mnt/data/CommandAndConquer/Thyme>). The checkout was clean when reviewed. Thyme identifies itself as a bottom-up reimplementation of *Command & Conquer: Generals — Zero Hour*, not C&C4; this is therefore an adjacent RTS-engine reference, not evidence for C&C4-specific behavior ([README](</mnt/data/CommandAndConquer/Thyme/README.md>)).

## Evidence reviewed

This pass sampled the object/module and weapon model, the central game-logic update, game-state CRC serialization, repository subsystem layout, build options, and unit-test organization. It did not build or run Thyme, and it did not inspect the entire 1,270-file source tree line by line.

| Area | Observed in Thyme | Local evidence |
|---|---|---|
| Subsystem boundaries | Separate `game/logic`, `game/client`, `game/network`, `game/common`, `platform`, `w3d`, `editor`, and `tools` trees | [`src/` tree](</mnt/data/CommandAndConquer/Thyme/src>) |
| Unit composition | A named `ThingTemplate` keeps distinct body, draw, and client-update module collections; module metadata declares the interfaces a module fulfills | [`thingtemplate.h`](</mnt/data/CommandAndConquer/Thyme/src/game/common/thing/thingtemplate.h:267>), [`moduleinfo.h`](</mnt/data/CommandAndConquer/Thyme/src/game/common/thing/moduleinfo.h:24>), [`module.h`](</mnt/data/CommandAndConquer/Thyme/src/game/common/thing/module.h:41>) |
| Content validation | The INI definition parser requires unique module tags and checks that a selected module is legal for the requested role; template override paths distinguish add, remove, and replace cases | [`thingtemplate.cpp`](</mnt/data/CommandAndConquer/Thyme/src/game/common/thing/thingtemplate.cpp:710>) |
| Factory boundary | A registry maps module name/type to a data parser, constructor, and interface mask instead of directly constructing every module at each call site | [`modulefactory.h`](</mnt/data/CommandAndConquer/Thyme/src/game/common/thing/modulefactory.h:31>), [`modulefactory.cpp`](</mnt/data/CommandAndConquer/Thyme/src/game/common/thing/modulefactory.cpp:91>) |
| Definition vs runtime state | `WeaponTemplate` contains authored weapon rules; `Weapon` is an instance with status, ammo/reload behavior, and snapshot hooks | [`weapon.h`](</mnt/data/CommandAndConquer/Thyme/src/game/logic/object/weapon.h:189>), [`weapon.h`](</mnt/data/CommandAndConquer/Thyme/src/game/logic/object/weapon.h:392>) |
| Simulation phases | `GameLogic::Update` processes command messages, scheduled object modules, AI/build assistance, spatial partitioning, deferred destruction, weapons, locomotion, and victory conditions in an explicit order before advancing the frame | [`gamelogic.cpp`](</mnt/data/CommandAndConquer/Thyme/src/game/logic/system/gamelogic.cpp:954>) |
| State diagnostics | Periodic multiplayer/replay CRCs serialize objects and selected simulation systems such as the partition manager, player list, and AI | [`gamelogic.cpp`](</mnt/data/CommandAndConquer/Thyme/src/game/logic/system/gamelogic.cpp:2796>) |
| Tests and tools | A separate test target covers reusable facilities (file system, compression, audio, text, video and W3D); optional developer tools have their own build flag | [`tests/CMakeLists.txt`](</mnt/data/CommandAndConquer/Thyme/tests/CMakeLists.txt:1>), [root `CMakeLists.txt`](</mnt/data/CommandAndConquer/Thyme/CMakeLists.txt:128>) |

## Transferable patterns

### 1. Keep authored definitions separate from match instances

**Pattern:** static archetypes describe what can be spawned; runtime entities hold mutable health, cooldowns, order, owner, cargo, morale, and other match state. Thyme's distinction between a unit's template and per-weapon runtime state makes that boundary visible.

**POLITIKA application:** keep the planned stable-ID `UPrimaryDataAsset` definitions authoritative for static faction/unit/squad/weapon/ability tuning. Spawned C++ objects/components own mutable state. Never use a Data Asset or widget as a live health/order container. The current first-slice two-archetype fixture can prove this boundary without pretending the full schema is implemented; production schema and validation remain the later milestone in [ENGINE_OUTLINE.md](ENGINE_OUTLINE.md#4-content-model-and-authoring).

### 2. Make behavior composition typed and checkable

**Pattern:** Thyme composes object behavior from modules, but each module advertises an interface role and tags are validated during content loading. This offers extensibility while catching illegal configurations early.

**POLITIKA application:** borrow the *contract*, not its legacy module registry. For the vertical slice, use ordinary explicit C++/UE Actor components and narrow interfaces—e.g. movement capability, weapon capability, selectable/presentable—because a general module factory would be excess machinery. As the roster grows, define typed capability sets or component profiles in POLITIKA-owned assets, reject duplicate IDs/tags and incompatible capabilities in editor/CI, and keep client-only presentation capabilities out of authoritative simulation. Avoid a string-only runtime factory that fails late.

### 3. Establish one observable simulation order

**Pattern:** the central game-logic update defines when commands, entity updates, AI, destruction, weapon/locomotion stores, and victory checks happen. It also allows update modules to sleep until their next eligible frame rather than doing every task every tick.

**POLITIKA application:** define phase order before adding many interacting systems: admit/validate orders → movement/targeting → combat and damage → lifecycle/deferred removal → objectives/victory → publish state events for presentation. This is a design contract, not an instruction to recreate Thyme's scheduler or replace UE ticking immediately. For the first RTS slice, keep the Actor/component implementation small; introduce an explicit C++ simulation phase coordinator only when actual ordering conflicts or measured scale justify it. Separate high-frequency movement from slower AI/objective evaluation.

### 4. Treat player input as a command, not a state mutation

**Pattern:** gameplay messages are processed through the logic layer and the replay/network path can observe the same serialized command stream. The UI/client is not the authority that directly changes a unit's health or order result.

**POLITIKA application:** preserve the Engine Outline contract—input becomes a validated, typed order; match rules apply it; state changes emit events; presentation reads the resulting state. Give orders stable type/parameters and explicit rejection reasons. This enables an offline local command path first and gives saves, telemetry, and replay a future seam without requiring multiplayer now.

### 5. Plan state serialization and desync diagnostics with state ownership

**Pattern:** snapshot/CRC hooks are distributed across simulation state owners, and the game computes periodic checks over objects and selected systems. The source demonstrates that meaningful diagnostics need a defined state set, not an arbitrary hash of rendered Actors.

**POLITIKA application:** keep save/replay serialization attached to authoritative state owners and stable IDs. Add a deterministic state-summary/checkpoint only when the prototype needs debugging evidence. Do **not** infer from Thyme that UE physics/floating point is deterministic across platforms or promise lockstep; POLITIKA's existing posture remains timestamped commands plus checkpoints unless a dedicated determinism prototype proves more.

### 6. Treat content errors as authoring failures

**Pattern:** Thyme checks module-role compatibility, unique module tags, and template override operations while parsing content.

**POLITIKA application:** reinforce the existing content contract: unique stable IDs, schema/version validation, required capability checks, typed cross-references, finite/ranged numbers, legal faction permissions, and explicit editor errors with asset/property context. Implement the rules from POLITIKA's own factions and mechanics; do not copy Thyme's INI schema, categories, defaults, or unit logic. Keep UE validation authoritative and let the Rust audit orchestrate/report it rather than duplicating game rules.

### 7. Keep tools and tests outside runtime dependencies

**Pattern:** Thyme organizes map/asset/debug tooling and reusable tests separately from its runtime layers. Its test suite focuses on bounded facilities and includes fixtures rather than requiring a complete match for every check.

**POLITIKA application:** maintain the planned separation between `Politika` runtime and `PolitikaEditor`/Rust audit tooling. Build small reusable rule-level fixtures for order rejection, weapon cooldown/damage, unit death and objective contest/progress when implementation reaches them; use packaged-client interaction evidence for the end-to-end slice. This is a proposed testing strategy; no tests were added or run as part of this study.

## Apply to the current milestone

The [First RTS Vertical Slice](RTS_VERTICAL_SLICE_MILESTONE.md) already makes the right initial trade-offs: two prototype archetypes, C++-owned movement/combat/objective state, player input through orders, read-only HUD, one control point, and no premature MassEntity, general scripting framework, lockstep or production content schema. Thyme supports tightening the *boundaries* of that slice, not expanding its scope:

1. Represent each prototype's static identity/tuning separately from mutable per-match state.
2. Route move/attack through one typed command validation/application path and report rejection without mutating state.
3. Define one deliberate order for movement, attack/damage, death cleanup and objective updates.
4. Have the HUD observe selection/order/health/objective results; keep animation/VFX/audio as feedback only.
5. Keep any future replay/hash hooks tied to authoritative state and defer determinism claims.

No change to the milestone's stated acceptance criteria is necessary before implementation. The only useful architectural addition is to document the simulation phase order when the first implementation work begins, then keep it aligned with measured behavior.

## Limits and licensing

This review is source-level evidence about **Thyme at the pinned commit**, whose README describes a Generals: Zero Hour reimplementation that relies partly on the original binary. It does not establish C&C4 internals, runtime behavior, or that Thyme's legacy techniques are suitable for UE 5.8.3. No source from Thyme was copied into POLITIKA, no dependency was added, and Thyme was not built or executed.

The repository states GPL version 2 or later, with a special exception for linking its resulting code against the original closed-source game while Thyme still relies on it ([Thyme README license section](</mnt/data/CommandAndConquer/Thyme/README.md#licence>), [`LICENSE`](</mnt/data/CommandAndConquer/Thyme/LICENSE>)). That exception is specific to its project context; it should not be assumed to authorize embedding Thyme code in POLITIKA. Keep the checkout as a read-only study reference unless a separate code-reuse and licensing decision is made.
