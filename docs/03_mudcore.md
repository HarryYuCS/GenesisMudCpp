# mudcore Layer

The `mudcore` library sits between the network transport and the GUI. It owns session
orchestration, GMCP parsing/application, and the bridge from the IO thread to the main
thread.

## Session

[`Session`](../src/mudcore/include/mudcore/session.hpp) is the only mudcore type the GUI
interacts with directly.

| Method | Thread | Purpose |
|--------|--------|---------|
| `connect(host, port)` | Main | Post async TCP connect; phase → `Connecting` |
| `disconnect()` | Main | Close TCP; phase → `Disconnected` |
| `sendCommand(text)` | Main | Run `OutboundPipeline`, post encoded line to IO |
| `sendGmcp(body)` | Main | Post GMCP subnegotiation frame to IO |
| `sendClientSize(w, h)` | Main | Send `Core.Client` with character window size |
| `setDebugLogging(bool)` | Main | Mirror raw GMCP packages to system log lines in `poll()` |
| `poll()` | Main | Drain `EventBus`, run pipelines, return `PollResult` |
| `gameState()` | Main | Read-only vitals/room/login snapshot |

### Poll loop

Each `poll()` drains all pending inbound events and produces:

- `lines` — `DisplayLine` values tagged with `OutputSink` (`Main`, `Comms`, `System`)
- `stateChanged` — GUI should refresh vitals/map panels
- `connectionPhase` — current `ConnectionPhase` for menus/footer/title

Event types from the IO thread:

| Event | Effect |
|-------|--------|
| `Connected` | Phase → `Connected`; system line |
| `Disconnected` | Phase → `Disconnected`; `GameState::reset()`; system line |
| `Error` | System line with `Network error: …` |
| `GmcpNegotiated` | Phase → `GmcpEnabled` → handshake → `HandshakeSent` |
| `MudText` | `InboundPipeline::processMudText` → main display |
| `GmcpRaw` | `InboundPipeline::processGmcp` → comms line and/or state update |

`Char.Login` sets `playerLoggedIn` on the pipeline result; `poll()` then transitions
`HandshakeSent` → `Ready`.

## Connection phases

```
Disconnected → Connecting → Connected → GmcpEnabled → HandshakeSent → Ready
                  ↑___________________________________________|
                              disconnect resets to Disconnected
```

[`ConnectionLifeCycle`](../src/mudcore/include/mudcore/connection_life_cycle.hpp) sends
`Core.Hello` and `Core.Supports.Set` on GMCP negotiation. The GUI sends `Core.Client` when
phase reaches `HandshakeSent`.

## Pipelines

### Inbound

[`InboundPipeline`](../src/mudcore/include/mudcore/inbound_pipeline.hpp):

- `processMudText` — wraps telnet text as `OutputSink::Main` (passthrough, no transforms)
- `processGmcp` — routes `Comm.*` to comms sink; updates `GameState` for Char/Room packages;
  handles `Core.Goodbye` as a system line

### Outbound

[`OutboundPipeline`](../src/mudcore/include/mudcore/outbound_pipeline.hpp) currently passes
commands through unchanged. Alias expansion is planned.

## GameState

[`GameState`](../src/mudcore/include/mudcore/game_state.hpp) holds:

- `Char.Vitals` textual levels (health, mana, food, drink, fatigue, intoxication)
- `Room.Info` / `Room.Map` for the magic map panel
- `Char.Login` player name and `loggedIn` flag
- Arbitrary named variables (`setVariable` / `getVariable`)

`reset()` clears all fields; called on disconnect during `poll()`.

`Room.Info` replaces `exits`/`doors` arrays on each update (not append). Coordinates clear
when omitted from a subsequent broadcast.

## EventBus

Thread-safe queue from IO handlers to `poll()`. IO thread enqueues; main thread drains.
`TelnetCodec` reset happens on the IO thread in the disconnected handler, not in `poll()`.

## Testing

Unit tests in [`tests/mudcore/`](../tests/mudcore/) use mock TCP peers and scripted telnet
payloads. Session tests cover connect, GMCP handshake, login → Ready, disconnect, reconnect,
and connection-refused error paths.
