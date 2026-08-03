# Using LNOS

This document describes how to use LNOS after installation.

## Components

LNOS consists of two main programs:

* `lnosd` — background daemon responsible for node discovery and network communication.
* `lnosctl` — command-line utility for managing the local LNOS node.

---

# lnosd

`lnosd` is the main LNOS daemon.

It is responsible for:

* announcing the local node;
* discovering other nodes;
* maintaining the local node registry;
* providing a local control interface for `lnosctl`.

Start the daemon:

```bash
sudo lnosd
```

The daemon creates a local UNIX socket:

```text
/run/lnos/lnosd.sock
```

which is used by `lnosctl` for communication.

---

# lnosctl

`lnosctl` is a command-line control utility.

## Syntax

```bash
lnosctl <command>
```

---

## Commands

### `generatekeys`

Generate a new public and private key pair for the local node.

Example:

```bash
lnosctl generatekeys
```

---

### `init`

Create the initial LNOS configuration.

Example:

```bash
lnosctl init
```

---

### `config`

Print the current LNOS configuration.

Example:

```bash
lnosctl config
```

---

### `set`

Set a configuration property.

Syntax:

```bash
lnosctl set <property> <value>
```

Example:

```bash
lnosctl set name pc.main.home
```

---

### `get`

Get a configuration property.

Syntax:

```bash
lnosctl get <property>
```

Example:

```bash
lnosctl get name
```

---

### `nodes`

List all discovered LNOS nodes.

Example:

```bash
lnosctl nodes
```

Example output:

```text
pc.main.home - 192.168.1.20 Status: Online
Services:
  ssh:22

laptop.dev.home - 192.168.1.30 Status: Online
Services:
  http:8080
```

---

# Permissions

Some operations require root privileges.

Commands that modify system-wide files or configuration may need to be run with:

```bash
sudo lnosctl <command>
```

---

# Localization

LNOS supports GNU gettext localization.

The displayed language follows the system locale.

Example:

```bash
LANG=ru_RU.UTF-8 lnosctl nodes
```

Installed translations are stored in:

```text
/usr/share/locale/<language>/LC_MESSAGES/lnos.mo
```
