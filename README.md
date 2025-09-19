# ft_irc

An IRC server implementation in **C++98**, compatible with both **Linux** (epoll) and **macOS** (poll).  
It supports multiple clients, authentication, nick/user registration, channel management, private messaging, and channel operator commands.
Written in a group together with https://github.com/michaela811 and https://github.com/denizcaglarcingoz.

*Project is compliant with the subject requirements for **42school ft_irc** and with IRC Protocol 1993*

---

## Features

- Non-blocking event loop:
  - **Linux** → uses `epoll`
  - **macOS** → uses `poll`
- Authentication with `PASS`, `NICK`, `USER`
- Channel system with:
  - `JOIN`
  - `PRIVMSG` (private or channel messages)
  - `INVITE`
  - `KICK`
  - `TOPIC`
  - `MODE` (`+i`, `+t`, `+k`, `+o`, `+l`)
- Proper numeric replies for protocol compliance
- Handles edge cases (buffer limits, malformed input, rapid connects, etc.)

---

## Build

```bash
make
```

---

## Run

```bash
./ircserv <port> <password>

# Example:
./ircserv 6667 mypass
```

---

## Event loop differences

- **poll** (macOS):
  - Scans the full file descriptor set each time.
  - Complexity: `O(n)` per call.
- **epoll** (Linux):
  - Kernel stores the set of watched FDs and only reports the ready ones.
  - Amortized complexity: `O(ready)`.
  - Much more scalable for many clients.

---

## Testing

### With HexChat (reference client)

Download: [HexChat](https://hexchat.github.io/)

1. Start server:
   ```bash
   ./ircserv 6667 mypass
   ```
2. In HexChat → *Network List* → Add a network:
   - Server: `127.0.0.1/6667`
   - Password: `mypass`
   - TLS: **disabled**
   - Choose a nickname and username
3. Connect and test:
   ```
   /join #chan
   /topic #chan :Hello World
   /msg #chan hi all
   /invite othernick #chan
   /kick #chan othernick :bye
   ```

### With `nc` (netcat)

**Terminal A**
```bash
nc -C 127.0.0.1 6667
PASS mypass
NICK alice
USER alice 0 * :Alice
JOIN #chan
```

**Terminal B**
```bash
nc -C 127.0.0.1 6667
PASS mypass
NICK bob
USER bob 0 * :Bob
JOIN #chan
PRIVMSG #chan :hello!
```

#### Partial packet test (Ctrl+D style)

```bash
{ printf 'com'; sleep 1; printf 'man'; sleep 1; printf 'd\r\n'; } | nc -C 127.0.0.1 6667
```

This simulates sending fragmented input (`com` + `man` + `d\n`).
Or while running nc when you have at least one character in your command line just press Ctrl+D whenever you want.

---

## Job control with `nc`

Control that the server is still running when one of the clients is suspended.
In previously created 'alice' and 'bob':
- on the terminal of bob press `Ctrl+Z`
- send PRIVMSG from alice to bob
- type `fg` in bob's terminal to bring him to foreground
- all messages sent by alice should arrive to bob

---

## Automated tests

From repo root:

```bash
./run_all_edge_tests.sh
```

Or run individually:

```bash
./test_buffer_limits.sh
./test_malformed.sh
./test_protocol_violations.sh
./test_rapid_connections.sh
./test_special_chars.sh
```

These scripts send crafted input via `nc` to test edge cases.

---

## Credits
- **Authors:** grignetta, https://github.com/michaela811 and https://github.com/denizcaglarcingoz
