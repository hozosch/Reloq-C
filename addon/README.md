# NVDA add-on

ReLoq C 1.2.1 provides the **Eloquence C** synthesizer for NVDA 2026.1
or later in a 64-bit Windows process. The add-on includes the shared C speech
engine, voice and sample-rate controls, pronunciation dictionaries and
optional ECI tag processing. See the [release notes](../docs/releases/1.2.1.md)
and the add-on's built-in help for usage and limitations.

Install the `.nvda-addon` from GitHub Releases, restart NVDA and choose
**Eloquence C** in Speech settings. Version 1.2.1 has a new add-on identity compared with the published 1.1
and installs alongside that earlier add-on. Installations already using the
new identity update in place. Back up dictionaries edited in the
old add-on folder before disabling or removing the old add-on; restart NVDA
and check the new add-on before removing any backup.

## Build and validate

A Windows x64 build of the ECI DLL and GNU gettext's `msgfmt` are required.

```sh
make -C eci windows-x64
python3 addon/build_synth.py
python3 -B addon/test_synth.py
```

The output is `dist/reloq-c-synth-x64-1.2.1.nvda-addon`. The build and
host tests do not replace a listening test in NVDA on Windows. English and
German UI translations are maintained in `synth/locale/`; see
[TRANSLATING.md](TRANSLATING.md) for translation guidance.
