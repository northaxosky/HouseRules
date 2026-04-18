# Survival Architect

An F4SE plugin that configures Fallout 4's Survival mode through a single MCM menu. Selectively unlock vanilla Survival restrictions (fast travel, console, saves, compass, etc.) and tune need rates (hunger, thirst, sleep) without leaving Survival difficulty.

## Status

v0.1 — scaffolding in place; unlocks and need-rate sliders coming next.

## Requirements

- Fallout 4 with F4SE
- Address Library for F4SE Plugins
- Mod Configuration Menu (MCM)

## Build

Requires [xmake](https://xmake.io). CommonLibF4 is a git submodule at `lib/commonlibf4`.

```sh
git clone --recursive https://github.com/northaxosky/SurvivalArchitect.git
cd SurvivalArchitect
xmake config -m releasedbg
xmake build
```

To point the build at an existing local checkout of CommonLibF4 instead of the submodule, set `COMMONLIBF4_PATH` before running `xmake config`.

## License

MIT — see [LICENSE](LICENSE).
