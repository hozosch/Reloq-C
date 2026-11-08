# Aktuelle Build-Anleitung

Die aktive Quelle liegt direkt im Repository. Zum Bauen wird kein historisches Archiv und kein Restore-Skript benötigt. Die alte Datei `BUILD-MSYS2-ANLEITUNG.txt` stammt aus 0.2.28; für den aktuellen Stand gilt diese Anleitung.

## Nativer Linux-Kern

Benötigt werden GCC, GNU Make, binutils und Python 3. Alle Befehle werden vom Repository-Hauptordner ausgeführt.

```sh
make -C eci -j4 all
python3 eci/test_public_abi.py eci/libreloq_eci.so --header include/reloq_eci.h
python3 eci/test_diagnostics.py eci/libreloq_eci.so
python3 nvda/test_dictionary_manager.py
python3 nvda/test_eci_analysis_options.py
python3 tools/check_language_rates.py
python3 nvda/test_eci_dictionary_semantics.py
python3 nvda/test_eci_dictionary_retention.py
python3 addon/test_synth.py
python3 addon/test_synth.py de
python3 nvda/test_eci_transport.py
python3 -X faulthandler tools/check_dictionary_lifetime.py --cycles 1
```

Das Ergebnis ist `eci/libreloq_eci.so`. Der erste optimierte Build benötigt einige Minuten. Die aufgeführten Prüfungen schreiben keine Audiodateien. Der zusätzliche Lebensdauertest verarbeitet PCM ausschließlich im RAM und prüft Wörterbuchwechsel, Äußerungsabschluss und das Freigeben/Neuerzeugen des US-Englisch-Kerns. Er ersetzt die Prüfung in NVDA auf Windows nicht.

## Windows-x64-DLL

In einer MSYS2-MINGW64- oder UCRT64-Shell mit x64-GCC, binutils, Make und Python:

```sh
make -C eci -j4 windows-x64 WINDOWS_CC=gcc WINDOWS_OBJDUMP=objdump
```

Beim Cross-Build unter Linux mit den entsprechend benannten MinGW-w64-Werkzeugen:

```sh
make -C eci -j4 windows-x64
```

Der Makefile-Standard erwartet `x86_64-w64-mingw32-gcc` und `x86_64-w64-mingw32-objdump`. Compiler und Objdump können über `WINDOWS_CC` und `WINDOWS_OBJDUMP` abweichend angegeben werden. Ergebnis ist `eci/eci.dll`, eine x64-DLL.

## NVDA-Add-on packen

Nach dem Windows-Build:

```sh
python3 addon/build_synth.py --dll eci/eci.dll
```

Der Paketbau prüft das x64-PE-Format, übernimmt Treiber und Daten und fragt die aktuelle Master-Version von `eigencrow/ibmttsdictionaries` ab. Dafür wird Netzwerkzugriff benötigt. Das Paket landet unter `dist/`; Versionsangaben und Ausgabe-Pfad stehen in `addon/synth/manifest.ini` und `addon/build_synth.py` und müssen für eine neue Veröffentlichung gemeinsam aktualisiert werden.

Die Versionshinweise beschreiben jeweils die Änderungen seit der unmittelbar vorhergehenden Veröffentlichung. Unveränderte bekannte Einschränkungen und Umstiegshinweise können separat genannt werden.

Der aktuelle Paketstand ist 1.2.2. Änderungen und bekannte Grenzen stehen in [den Versionshinweisen](releases/1.2.2.md). Die C-Kernversion wird beim Laden im NVDA-Protokoll ausgegeben; damit lässt sich prüfen, welche DLL tatsächlich aktiv ist.

## Android ARM64

Die [Android-Anleitung](../android/README.md) beschreibt den APK-Build mit demselben C-Kern, JDK 17, SDK 35 und NDK r30. `python3 android/build.py` erzeugt das signierte ARM64-Paket unter `dist/`, wenn die dort beschriebenen SDK-, NDK- und Java-Pfade gesetzt sind. Android verwendet getrennte Build-Verzeichnisse. Für JNI-Transport, Warteschlange und ARM64-Rücksprünge gibt es eigene Prüfungen. Die neue App-Kennung führt zu einer separaten Installation; Einstellungen und Wörterbuchfreigaben sind erneut einzurichten.

German uses the same built-in native dictionary path as the other languages. Historical `.rlex` files remain test/reference fixtures and are not required by either installation package.

## Windows x64 SAPI5

Build the existing `eci.dll` target first. Then compile the C COM adapter and
native settings/test programs, and package one internal architecture payload:

```sh
python3 sapi/generate_catalogue.py --check
python3 sapi/build.py --cc x86_64-w64-mingw32-gcc --objdump x86_64-w64-mingw32-objdump
```

The generated catalogue reuses NVDA's names, language ordering and anatomy
profiles, plus the existing Android sample sentences. The adapter and native
programs require only ordinary Windows system DLLs at runtime. Python and the
compiler are build tools. The package includes the same required third-party
notices as the other integrations.

`make -C sapi test` runs the production streaming backend against the host
ECI library using a test-only POSIX implementation of the small Win32 loading, atomic and UTF conversion boundary. This covers real core synthesis,
fragments, source events, partial writes, abort, priority reentrancy and model
changes. It does not test Windows COM activation or UI accessibility.
`Test-SAPI.exe`, included in the installation, tests all installed language tokens via
real Windows SAPI and an in-memory stream. See [the SAPI5 guide](../sapi/README.txt).

To build the single x86/x64 installer, build both payloads with `sapi/build.py
--arch x64` and `--arch x86`. On Windows, run `python sapi/build_installer.py
--iscc "C:\Program Files (x86)\Inno Setup 6\ISCC.exe"` with Inno Setup 6.7 or
later. The GitHub installer workflow performs this build, installs both COM
views, checks all 49 native/application rate pairs in every language, and
uninstalls both views. All synthesis diagnostics use memory streams only.
