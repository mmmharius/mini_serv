# mini_serv

TCP chat server written in C

## Overview

A non-blocking multi-client chat server that listens on `127.0.0.1` and allows connected clients to communicate with each other in real time using `select()`

## Features

- Client ID assignment (incremental from 0)
- Broadcast messages to all connected clients
- Per-line message forwarding with `client %d: ` prefix
- Join/leave notifications (`server: client %d just arrived/left`)
- Partial message buffering (handles incomplete `recv`)

## Build

with flag
```bash
cc -Wall -Wextra -Werror mini_serv.c -o mini_serv
```

no flag
```bash
cc mini_serv.c -o mini_serv
```

## Usage

```bash
./mini_serv <port>
```

Then connect clients using `nc`:

```bash
nc 127.0.0.1 <port>
```

## Example

```
# Terminal 1
./mini_serv 8080

# Terminal 2
nc 127.0.0.1 8080
> server: client 1 just arrived
hello

# Terminal 3
nc 127.0.0.1 8080
> server: client 0 just arrived
> client 0: hello
```
