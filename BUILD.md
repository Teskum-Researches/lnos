# Building LNOS

This document describes how to build LNOS from source.

## Requirements

Install the required dependencies:

```bash
sudo apt install cmake ninja-build libsodium-dev gettext
```

Required tools:

* CMake 3.16 or newer
* C++20 compatible compiler
* libsodium
* GNU gettext

---

## Debug build

Configure the project:

```bash
cmake -B cmake-build-debug \
      -DCMAKE_BUILD_TYPE=Debug
```

Build:

```bash
cmake --build cmake-build-debug
```

The resulting binaries will be located in:

```
cmake-build-debug/
├── lnosd
└── lnosctl
```

---

## Release build

Configure the project:

```bash
cmake -B cmake-build-release \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr
```

Build:

```bash
cmake --build cmake-build-release
```

Release binaries will be located in:

```
cmake-build-release/
├── lnosd
└── lnosctl
```

---

## Building translations

LNOS uses GNU gettext for localization.

Translation files are compiled automatically during the CMake build process.

Source files:

```
po/
├── lnos.pot
└── <language>.po
```

Generated files:

```
locale/
└── <language>/
    └── LC_MESSAGES/
        └── lnos.mo
```

---

## Clean build

Remove the build directory:

```bash
rm -rf cmake-build-debug
rm -rf cmake-build-release
```

Then configure the project again.
