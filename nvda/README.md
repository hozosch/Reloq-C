# NVDA driver development

The ReLoq C NVDA add-on uses the native ECI-compatible engine from `../eci/`.
Its driver and settings live in `../addon/synth/`; the modules in this folder
bind the ECI interface, coordinate speech and playback, and manage voice
catalogues, dictionaries and updates.

`_reloq_eci_direct.py` owns the native handle and submits text, marks and
voice controls. `_reloq_eci_transport.py` coordinates synthesis, queued
audio, cancellation and playback-gated notifications. These modules are
packaged into the add-on by `../addon/build_synth.py`.

## Host checks

```sh
PYTHONPATH=nvda python3 nvda/test_eci_direct.py
PYTHONPATH=nvda python3 nvda/test_eci_transport.py
PYTHONPATH=nvda python3 nvda/test_updates.py
make -C eci all
PYTHONPATH=nvda python3 nvda/test_eci_languages.py
```

Native checks require the host library built in `eci/`. Packaging uses a
Windows x64 DLL; see [the add-on build instructions](../addon/README.md).
Host tests exercise API and transport behavior but do not establish audio
latency or sound quality in an actual NVDA installation.
