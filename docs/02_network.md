# Network Layer

The `network` library owns the transport: raw TCP and the telnet wire protocol. It has no
knowledge of mudcore, GMCP semantics, or the GUI. `mudcore::Session` translates network
types into application events.

## Components

| Component | Responsibility |
|-----------|----------------|
| `TcpSession` | Async resolve/connect, read loop, serialized write queue, connection state |
| `TelnetCodec` | Incremental telnet decode (IAC state machine) and encode (CRLF lines, GMCP frames) |
| `network_types.hpp` | Shared value types: `ConnectionConfig`, `ConnectionState`, `NetworkError`, `TelnetFeedResult` |

`TcpSession` moves bytes; `TelnetCodec` interprets them. Neither calls the other — `Session`
feeds `TcpSession` reads into `TelnetCodec::feed()` and sends encoded bytes back out.

## Threading contract

Everything in this layer runs on the `io_context` thread:

- `TcpSession::connect()`, `disconnect()`, and `send()` must be invoked on the io thread.
  Callers on other threads use `boost::asio::post` (see `mudcore::Session`).
- All handlers (`ReadHandler`, `ConnectedHandler`, `DisconnectedHandler`, `ErrorHandler`)
  are invoked on the io thread. They must not block and must not call back into
  `TcpSession` synchronously except via post.
- `TcpSession::getConnectionState()` is the one exception: it reads an atomic and is safe
  from any thread.
- `TelnetCodec` is not thread-safe; it is owned and touched only by the io thread.

## TcpSession callback contracts

- **`ConnectedHandler`** fires once per successful connect, before the read loop starts.
- **`DisconnectedHandler`** fires exactly once per connection, regardless of how it ended
  (client `disconnect()`, peer EOF, or error).
- **`ErrorHandler`** fires for failed resolve, connect, read, or write — always *before*
  the corresponding `DisconnectedHandler`. A clean peer EOF is **not** an error: only
  `DisconnectedHandler` fires.
- **`ReadHandler`** receives raw payload bytes. Reads may be chunked arbitrarily by TCP;
  there is no internal reassembly — `TelnetCodec::feed()` handles frames split across
  reads. The read loop only runs while a read handler is registered; without one, peer
  disconnects are not detected until a write fails.

## Write behavior

`send()` copies the bytes into a pending-write queue. Writes are serialized: one
`async_write` at a time, drained in FIFO order, so back-to-back `send()` calls cannot
interleave on the wire. A write failure is reported through `ErrorHandler` and closes
the connection; queued writes are dropped on disconnect.

## Reconnect

`connect()` is a no-op unless the state is `DISCONNECTED`. After any disconnect the
session returns to `DISCONNECTED` and may be reused for a new connection. `Session`
resets `TelnetCodec` (parser state and GMCP negotiation) in the disconnected handler,
so each connection renegotiates telnet options from scratch.

## Telnet specifics (Genesis)

- Genesis sends `IAC WILL GMCP`; the codec auto-replies `IAC DO GMCP` once per connection
  via `TelnetFeedResult::wireReplies` and sets `negotiatedNow` so `Session` can start the
  GMCP handshake.
- Other option negotiations are consumed and ignored; non-GMCP subnegotiations are
  skipped without emitting data.
- Inbound text accumulates until a CRLF boundary or the end of a `feed()` call (partial
  lines and prompts are emitted as-is).
- `IAC` bytes inside payloads are doubled on the wire in both directions.
