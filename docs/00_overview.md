# Genesis MUD C++ Client — Overview

A wxWidgets desktop GMCP client for [Genesis MUD](https://www.genesismud.org/). The
codebase is split into three libraries plus documentation:

| Layer | Path | Role |
|-------|------|------|
| Network | [`src/network/`](../src/network/) | TCP transport and telnet codec |
| mudcore | [`src/mudcore/`](../src/mudcore/) | Session orchestration, GMCP pipelines, game state |
| GUI | [`src/gui/`](../src/gui/) | wxWidgets client; owns `mudcore::Session` |

## Documentation map

- [GMCP protocol reference](01_gmcp.md) — Genesis packages and payloads
- [Network layer](02_network.md) — `TcpSession`, `TelnetCodec`, threading
- [mudcore layer](03_mudcore.md) — `Session`, pipelines, `GameState`, connection phases
- [GUI layer](04_gui.md) — panels, `GuiController`, live testing checklist

## Thread model (summary)

```
wx main thread          IO thread
     |                      |
GuiController ----post----> TcpSession
     |                      |
  wxTimer poll() <---- EventBus <---- telnet handlers
     |
  update panels
```

The GUI never touches the socket or telnet parser directly. All server data flows through
`Session::poll()` as `DisplayLine` values and `PollResult` flags.

## Building

```bash
cmake -B build
cmake --build build
ctest --test-dir build
```

The `genesis_client` executable links the `gui` object library, which depends on `mudcore`
and wxWidgets.

## Default server

`mud.genesismud.org:3011` — configurable via Settings / Connect dialog (`wxConfig` app
`GenesisMUD`).

## Roadmap

See [README.md](../README.md). Core transport, GUI wiring, and live-play hardening precede
aliases/triggers with regexp support.
