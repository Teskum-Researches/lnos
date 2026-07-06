# LNOS — Local Network Overlay System

Experimental overlay networking system for local networks.

LNOS is a lightweight distributed discovery and communication layer built on top of standard IP networking. Instead of interacting with devices through IP addresses, nodes are identified using human-readable hierarchical names.

Example:

```text id="kq7m2d"
pc.main.gervaty
laptop.dev.myxa
pi.router.home
```

LNOS is not intended to replace IP, DNS, or the Internet.
It exists as a local overlay system for experimentation and learning.

---

# Features

## Current

* UDP multicast node discovery
* Automatic node registry
* TTL-based node cleanup
* Multi-threaded node agent
* Human-readable node naming

## Planned

* Name resolution (resolve API)
* Direct node-to-node messaging
* Service discovery layer
* Message routing between nodes
* Distributed registry synchronization
* Network topology visualization

---

# Architecture Overview

Each node runs a lightweight agent responsible for:

* announcing itself in the network
* discovering other nodes
* maintaining local registry state
* tracking node availability

Nodes communicate over multicast UDP and maintain a shared view of the network.

```text id="w8xk1p"
Node A  <---- multicast discovery ---->  Node B
   \                                      /
    \------------ LAN network ----------/
```

---

# Concepts

## Node Identity

Each node is identified by a human-readable name:

```text id="z2k9sa"
device.type.owner
```

Example:

```text id="q1m8fd"
pc.main.ruslan
laptop.dev.ruslan
```

## Node Lifecycle

Nodes periodically announce their presence.
If a node stops sending updates, it is considered offline and removed after a TTL period.

---

# Roadmap

## Node Metadata

* ONLINE/OFFLINE states
* last seen timestamps
* service descriptors
* node UUIDs
* improved packet format

## Name Resolution

* resolve(name) API
* query/response protocol
* local caching of resolved nodes

## Messaging Layer

* direct UDP communication
* TCP messaging support
* message routing
* basic authentication/token support

## Visualization

* network topology representation
* live updates
* optional web UI

## Distributed Registry

* peer-to-peer synchronization
* gossip-based updates
* conflict resolution strategies

---

# Status

LNOS is an early experimental system for exploring:

* distributed networking concepts
* service discovery mechanisms
* overlay network design
* systems programming in C++
