# POLITIKA local development and reference tools

Tool installations and source checkouts live in the ignored `.tools/` directory. These are local developer dependencies, not vendored game code. The authoritative C&C source/reference and extracted-data locations remain `/mnt/data/CommandAndConquer/`, `/mnt/data/CommandAndConquer/UM/`, and `/mnt/data/CommandAndConquer/REA/`.

## Installed tools

| Tool | Location / entry point | POLITIKA use | Notes |
|---|---|---|---|
| Ghidra 12.1.4 + Ghidra MCP 7.0.0 | `.tools/ghidra/ghidra_12.1.4_PUBLIC/ghidraRun`; bridge at `~/.local/bin/bridge-mcp-ghidra` | Static binary analysis with MCP integration | Extension installed to `~/.config/ghidra/ghidra_12.1.4_PUBLIC/Extensions/GhidraMCP`. Launch Ghidra, enable `GhidraMCPPlugin` in plugin configuration (not yet enabled because this session has no desktop display), then the local bridge endpoint is `http://127.0.0.1:8089`. OpenClaw MCP server is registered as `ghidra-mcp` in the user config. The extension and Ghidra versions must match (12.1.4). |
| angr 10.0.5 | `.tools/angr/.venv/bin/python` | Symbolic execution, CFG recovery, and binary analysis | Use the dedicated Python 3.13 environment; import the `angr` module in analysis code. |
| UE4SS 3.0.1 | `.tools/ue4ss/` | Inspect/mod Unreal Engine game runtime for compatible Windows builds | Official bundle is Windows-only. It cannot instrument a native Linux Unreal build; use against a Windows packaged POLITIKA build. Upstream describes UE 4.7–5.8 targeting; compatibility remains build/game-specific. |
| OpenRA engine + Mod SDK | `.tools/openra/source/`; Linux Red Alert AppImage extracted at `.tools/openra/appimage/` | Study RTS movement, construction, combat, fog-of-war, AI, maps, scripted missions, and mod SDK patterns | Source checkout is upstream `bleed`; AppImage is upstream `release-20250330`. Run its utility with `.tools/openra/appimage/AppRun --utility` (add `ra` and a utility command as needed). Source build uses `make` and requires .NET 10. |
| Vanilla-Conquer | `.tools/vanilla-conquer/` | Study C++ Tiberian Dawn/Red Alert engine, pathfinding, units, and combat | Upstream `vanilla` branch source is included. CMake/Ninja native Linux build is under `.tools/vanilla-conquer/build`. Original game data is not bundled. |
| cnc-formats 0.1.0-alpha.5 | `.tools/cnc-formats/`; CLI `.tools/cnc-formats/install/bin/cncf` | Parse/inspect MIX, SHP, PAL, AUD, TMP and additional Westwood formats | Alpha release; confirm parser coverage for each file variant before relying on it for production asset conversion. |
| ImHex 1.38.1 | `.tools/downloads/imhex-1.38.1-x86_64.AppImage` | Inspect undocumented binary formats and build/iterate patterns | Launch the AppImage directly. |

## Quick commands

```sh
.tools/ghidra/ghidra_12.1.4_PUBLIC/ghidraRun
.tools/angr/.venv/bin/python -c 'import angr; print(angr.__version__)'
.tools/cnc-formats/install/bin/cncf --help
.tools/openra/appimage/AppRun --utility
.tools/vanilla-conquer/build/vanillatd
.tools/downloads/imhex-1.38.1-x86_64.AppImage
```

UE4SS is a runtime mod loader, not a general-purpose static analyzer; install its DLLs into a compatible Windows game build's `Binaries/Win64` directory only when testing that build.
