# POLITIKA — Platform Support and Portability Plan

Status: planned product scope and engineering acceptance criteria (2026-10-27). **No playable builds or platform certifications are asserted by this document.**

POLITIKA is a **multi-platform real-time strategy game**. Linux, Windows, SteamOS/Steam Deck and Android are within the primary planned platform scope from the architectural design stage. Platform-specific delivery can be staged; listing a target does not guarantee a simultaneous release, a signed package, storefront approval or performance on every device.

## 1. Platform commitment matrix

| Platform | Status | Intended experience | Qualification boundary |
|---|---|---|---|
| **Linux** | Planned primary target | Native desktop build; mouse/keyboard RTS controls, local campaign/skirmish, scalable HUD | Define distributions/graphics drivers, package format, minimum hardware and real-device test coverage |
| **Windows** | Planned primary target | Native desktop build; mouse/keyboard RTS controls, local campaign/skirmish, high-resolution display modes | Define Windows versions, GPU/driver baseline and packaged-build testing |
| **SteamOS** (including **Steam Deck**) | Planned primary target | Handheld PC-compatible experience, with gamepad/Steam Input-oriented interaction, legible compact HUD and portable performance modes | Establish whether native Linux packaging and/or a compatibility layer is used; test actual hardware. No Steam Deck Verified claim |
| **Android** | Planned primary target | Native mobile app target with touch-first selection, camera and command interaction; reduced graphics and memory profiles | Prototype UE Android packaging, mobile GPU/performance limits, screen/orientation support, touch UX, OS/API level and device coverage before promising release |
| **PlayStation, Xbox, Nintendo** | Possible later expansion | Console-appropriate RTS controller/HUD adaptations | Requires access to platform SDKs, console hardware, licensing/certification and sustained support; none are confirmed targets for an initial release |
| **Apple** (macOS; possibly iOS/iPadOS) | Possible later expansion | Desktop and/or tablet/phone control and rendering profiles, depending on target | Require separate porting, tooling, distribution, performance and usability assessment; no launch commitment |

"SteamDock" is not a platform name used in the specification; the intended handheld PC is **Steam Deck**. SteamOS is Linux-based, but a Linux build alone does not demonstrate acceptable handheld controls, performance or compatibility.

## 2. Shared-game architecture

A common C++/Unreal simulation owns the authoritative state: factions, unit statistics, selection/order validation, combat, pathfinding, economy, intelligence, mission rules, AI, save-state format and victory conditions. Platform adapters handle input, display, local storage, packaging and optional storefront integration. Device-specific UX may differ; mechanics and scenario data should not fork by OS unless an explicitly reviewed design decision requires it.

Define logical actions such as `Select`, `ExtendSelection`, `IssueMove`, `IssueAttack`, `TargetAbility`, `Cancel`, `PanCamera`, `ZoomCamera` and `OpenCommandMenu`. Bind them to mouse/keyboard, controller/Steam Input and touch gestures without different game rules. Always expose valid-target, confirmation, rejection and cancel feedback.

## 3. Input and interface profiles

- **Linux and Windows desktop:** conventional pointer RTS controls, keyboard shortcuts, drag-selection, precise positioning and resizable panels; maintain a readable high-resolution 4K target and test lower-resolution/ultrawide modes.
- **SteamOS / Steam Deck:** compact, gamepad-focusable menus and mission briefings; inspectable unit data; accessible tactical orders through physical controls or contextual/radial interfaces; appropriately sized typography; test trackpads, controller mapping, external input and handheld pause/resume.
- **Android:** touch-friendly hit regions, selection and target affordances, clear feedback for invalid gestures, reliable map panning/zoom, and collapsible HUD hierarchy. Do not require hover, precise right-click, desktop shortcut keys or persistent desktop-sized panels. Prototype landscape-first play, orientation policy, mobile interruptions and optional controller support.

The 4K requirement is for the capable desktop presentation, **not a minimum render resolution on handheld or Android devices**. Maintain a shared visual identity inspired by Yugoslav maps and archival interfaces, without sacrificing legibility, safe-area compliance or touchscreen usability.

## 4. Performance, builds and services

Set measurable budgets per actual hardware tier (CPU simulation time, memory, GPU frame time, unit counts, draw calls, texture pool, thermal throttling and battery behaviour). Adapt rendering, streaming, UI scale and effects to device capability. Test the same representative high-unit-count mission on each target tier so portable builds do not silently alter tactical balance.

Use per-platform configurations, packaged build verification and an incremental CI matrix as toolchains become available. Android requires an independent mobile packaging and input prototype; SteamOS needs actual Steam Deck testing. Native Linux and Windows builds need their own packaging and driver/OS qualification. No release should be called supported based solely on editor play or an untested build.

Keep campaign, local skirmish, saves and settings usable offline without mandatory account creation or external identity services, consistent with the game's offline-first direction. Use platform-specific save locations and suspend/resume handling. Cloud saves, shared saves between OSes, cross-play, online accounts, storefront integration and multiplayer are **not implied** by multi-platform support.

## 5. Test and release gates

For each planned primary target, acceptance must include successful packaging and clean launch on supported hardware; navigable startup/settings/briefing screens; a complete offline mission and AI skirmish; all core RTS selection/target/cancel operations; accessible readouts and subtitles; correct save/load; resolution/scale or safe-area handling; suspend/resume where applicable; and measured representative CPU/GPU/memory performance.

A primary target can remain *planned* while any gate is untested. Change its status to *experimental*, *playable*, *supported* or *released* only when repository evidence, build artifacts and testing justify the language. No console, Apple, Steam Deck Verified or Android storefront certification should be advertised until independently obtained.

## 6. Release phasing and open questions

Choose technical implementation order through prototypes instead of inferring priority from this matrix. The primary planned scope remains all four platform families, even if an early playable milestone covers fewer devices. Resolve minimum hardware, supported Linux distributions, Windows version, SteamOS deployment method, Android screen/orientation policy, mobile controls, input rebinding, packaging formats, and the support/testing capacity for every delivered platform.

Reevaluate PlayStation, Xbox, Nintendo and Apple only after baseline core gameplay, portable performance and device-control viability are established. These expansions are optional and should not delay the architectural goal of shared gameplay and portable presentation.

Related documents: [README](../README.md) · [Engine Outline](ENGINE_OUTLINE.md) · [Proposed Engine Layers](PROPOSED_ENGINE_LAYERS.md) · [UI Overview](POLITIKA_UI_OVERVIEW.md).
