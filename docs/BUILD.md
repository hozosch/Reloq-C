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

Der aktuelle Paketstand ist 0.2.193. Änderungen stehen in [den Versionsnotizen](changes/0.2.193.md), bekannte Fehler und die Grenzen der bisherigen Prüfungen im [technischen Arbeitsstand](CURRENT-WORK.md). Die C-Kernversion wird beim Laden im NVDA-Protokoll ausgegeben; damit lässt sich prüfen, welche DLL tatsächlich aktiv ist.

## Android-ARM64-Vorschau

Die [Android-Anleitung](../android/README.md) beschreibt den ersten APK-Build mit demselben C-Kern, JDK 17, SDK 35 und NDK r30. `python3 android/build.py` erzeugt die signierte ARM64-Vorschau unter `dist/`, wenn die dort beschriebenen SDK-/NDK-/Java-Pfade gesetzt sind. Android verwendet getrennte Build-Verzeichnisse. Der JNI-Transport, die Warteschlange und die ARM64-Rücksprünge haben eigene Prüfungen ohne Audiodateien; Installation und grundlegende Ausgabe der vorigen Vorschau sind vom Nutzer bestätigt. Der Nutzer bestätigt 0.2.192 einschließlich sofortiger Einstellungen als stabil. Die beiden neuen Analyse-Schalter aus 0.2.193 benötigen noch ihren praktischen Hörtest.
