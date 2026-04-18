# Survival Architect

An F4SE plugin that configures Fallout 4's Survival mode through a single MCM menu. Selectively unlock vanilla Survival restrictions (fast travel, console, saves, compass, etc.) and tune need rates (hunger, thirst, sleep) without leaving Survival difficulty.

## Status

v0.1 — scaffolding in place; unlocks and need-rate sliders coming next.

## Requirements

- Fallout 4 with F4SE
- Address Library for F4SE Plugins
- Mod Configuration Menu (MCM)

## Build

Requires [xmake](https://xmake.io) and a local checkout of CommonLibF4 (default path `C:/Utilities/CommonLibF4`, override with the `COMMONLIBF4_PATH` env var).

```sh
xmake config -m releasedbg
xmake build
```

## License

MIT — see [LICENSE](LICENSE).
