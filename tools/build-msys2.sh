#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
case "${MSYSTEM:-}" in
  UCRT64|MINGW64|CLANG64) ;;
  *) echo "ERROR: run from the supplied build-msys2.cmd or an MSYS2 MinGW shell" >&2; exit 1;;
esac
case "$MSYSTEM" in UCRT64) export PATH="/ucrt64/bin:/usr/bin:$PATH";; MINGW64) export PATH="/mingw64/bin:/usr/bin:$PATH";; CLANG64) export PATH="/clang64/bin:/usr/bin:$PATH";; esac
command -v gcc >/dev/null || { echo 'Missing gcc in this MSYS2 environment' >&2; exit 1; }
command -v objdump >/dev/null || { echo 'Missing objdump in this MSYS2 environment' >&2; exit 1; }
if command -v python3 >/dev/null; then PY=python3; elif command -v python >/dev/null; then PY=python; else echo 'Missing Python' >&2; exit 1; fi
if ! command -v make >/dev/null 2>&1; then
  echo 'ERROR: MSYS2 make is missing.' >&2
  echo 'Install it from an MSYS2 terminal using: pacman -S --needed make' >&2
  echo 'Then rerun build-msys2.cmd.' >&2
  exit 127
fi
"$PY" tools/check_word_analysis_baseline.py
make -C eci -B eci.dll WINDOWS_CC=gcc WINDOWS_OBJDUMP=objdump
"$PY" eci/verify_pe_exports.py eci/eci.dll --objdump objdump --header include/reloq_eci.h
"$PY" tools/export_replace_files.py
printf '\nSUCCESS: Files to replace are in dist/replacement-files/\n'
