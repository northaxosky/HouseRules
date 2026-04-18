# Survival Architect

An F4SE plugin that curates and configures Fallout 4's Survival mode through a single MCM menu. A clean-room reimplementation combining features from Unlimited Survival Mode, Survival Options, Survival Configuration Menu, and BakaSurvivalSettings.

## Status

v0.1 — scaffolding. Unlocks and need-rate sliders wired end-to-end first; SCM's larger surface (disease, adrenaline, weight-based food/drink) deferred.

## Requirements

- Fallout 4 with F4SE
- Address Library for F4SE Plugins
- Mod Configuration Menu (MCM)

## Build

Requires [xmake](https://xmake.io) and [Dear Modding's CommonLibF4](https://github.com/doodlum/CommonLibF4) checked out locally (default: `C:/Utilities/CommonLibF4`, override with `COMMONLIBF4_PATH` env var).

```sh
xmake config -m releasedbg
xmake build
```

## License

MIT — see [LICENSE](LICENSE).
