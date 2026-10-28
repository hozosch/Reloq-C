# ReLoq C

Native C speech synthesis and an experimental NVDA add-on. This repository starts from the verified **0.2.186** release; the import does not change its runtime behaviour.

## Build

A Linux build needs Python 3, GNU Make, GCC and binutils. From the repository root:

```sh
make -C eci -j4 all
python3 eci/test_public_abi.py eci/libreloq_eci.so --header include/reloq_eci.h
python3 eci/test_diagnostics.py eci/libreloq_eci.so
python3 nvda/test_dictionary_manager.py
```

The first command produces `eci/libreloq_eci.so`. The native provider is compiled as C with `EVV_NO_BYTECODE`; there is no runtime rule interpreter. The first optimized build can take several minutes.

For the Windows x64 DLL, install the MinGW-w64 x64 compiler and run:

```sh
make -C eci -j4 windows-x64
```

See `BUILD-MSYS2-ANLEITUNG.txt` for the Windows environment. The NVDA adapter and dictionary settings are in `nvda/`; packaging tools and tests are in `addon/`. A release must be packaged and tested separately; the imported source alone is not a new add-on release.

## Current work

The following user-reported problems remain open:

- American English can crash with pronunciation dictionaries enabled.
- Greek letters still fail in ordinary Finnish and Korean sentence reading in the user's NVDA setup.
- Main text replacements must use ordinary contextual analysis and preserve case-sensitive matching.
- Changes in the intended dictionary folder do not reliably become active.

Passing an isolated pronunciation test does not establish that these runtime problems are fixed. Their latest user report takes precedence over previous test results.

The German acoustic planner and agreed pronunciation departures are retained; see `GEWUENSCHTE-ABWEICHUNGEN-DE.md`. The native C migration is ongoing. No speech-result cache, original runtime DLL/SYN or runtime bytecode belongs in the add-on. Verification PCM stays in memory; tests must not write audio files.

## Source provenance

`docs/source-0.2.186.json` records the verified source archive hash and the hashes of unchanged active production files. The full historical source archive remains preserved separately. This repository contains the active implementation, data, driver and checks; it omits old build products, historical snapshots and raw rule inputs that the native build does not use. Third-party notices and license files are retained at their source locations.
