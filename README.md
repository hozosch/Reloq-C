# ReLoq C

[English](README.en.md)

ReLoq C entwickelt eine Sprachausgabe für NVDA, die den vertrauten Klangcharakter von Eloquence bewahrt und ihn gezielt verbessert. Im Mittelpunkt stehen verständliche Aussprache, schnelles Lesen und eine direkte Reaktion beim Navigieren mit dem Screenreader.

Die Sprachverarbeitung orientiert sich an Apple Eloquence. Wo das Original Aussprachefehler oder Einschränkungen hat, soll ReLoq C bessere Ergebnisse liefern. Dazu kommen eine breitere Unicode-Unterstützung, höhere Abtastraten und mehr Möglichkeiten, die Stimme anzupassen.

## Was ReLoq C bietet

- **14 Sprachvarianten**, darunter Deutsch, amerikanisches und britisches Englisch sowie Chinesisch, Japanisch, Koreanisch und Griechisch.
- **Acht Stimmprofile:** Reed, Shelly, Bobby, Rocko, Glen, Sandy, Grandma und Grandpa. Kopfgröße, Tonhöhe, Rauigkeit und Behauchtheit lassen sich anpassen.
- **Sieben Abtastraten von 8 bis 48 kHz** für den klassischen Klang und eine erweiterte Klangbandbreite.
- **Unicode und sprachbezogene Zeichennamen:** zusätzliche Buchstaben, Akzente und Symbole werden in die Sprachverarbeitung einbezogen.
- **Optionale Aussprachewörterbücher:** Gemeinschaftswörterbücher für Deutsch und US-Englisch sowie eigene Dic-Dateien mit Verwaltung in den NVDA-Einstellungen.
- **Verkürzbare Pausen und ECI-Steuerbefehle** als zusätzliche Optionen.

## Unsere Ziele

ReLoq C soll im Alltag mit NVDA schnell, verständlich und zuverlässig sprechen. Dafür verbessern wir Wortanalyse, Zusammensetzungen, Satzmelodie und Lautübergänge. Bewährte Klangentscheidungen bleiben erhalten, etwa das konsonantische deutsche r und die abgestimmten Nasalvokale.

Ein weiteres Ziel ist ein kompakter, sparsamer C-Kern: kurze Reaktionszeiten auch bei längeren Texten und geringe CPU-Last bei hohen Abtastraten. Verbesserungen sollen allen unterstützten Sprachen zugutekommen.

## Mit NVDA verwenden

Die Erweiterung erscheint in NVDA als **Eloquence C (experimentell)**. Sie benötigt NVDA ab 2026.1 mit einem 64-Bit-Prozess; die DLL wird derzeit für Windows x64 gebaut.

Fertige Pakete gehören in den Bereich [Releases und Downloads](https://github.com/hozosch/Reloq-C/releases). Solange dort keine Datei mit der Endung `.nvda-addon` angeboten wird, steht über GitHub noch kein installierbares Paket bereit. Der Quellcode-Download ist zum Selbstbauen gedacht.

Wenn ein Paket bereitsteht, installiere die `.nvda-addon`-Datei mit NVDA, starte NVDA neu und wähle mit **NVDA+Strg+S** die Sprachausgabe **Eloquence C (experimentell)** aus. Sprache und Stimme stellst du in den Sprachausgaben-Einstellungen ein.

Die [Bedienungsanleitung](docs/USER-GUIDE.de.md) erklärt Sprachen, Einstellungen und Wörterbücher. ReLoq C ist noch in Entwicklung; [Änderungen und bekannte Einschränkungen in 0.2.187](docs/changes/0.2.187.md) sind separat dokumentiert.

## Rückmeldungen und Mitarbeit

Aussprachefehler, abgebrochene Ausgaben und Probleme im Alltag kannst du unter [Issues](https://github.com/hozosch/Reloq-C/issues) melden. Hilfreich sind ein kurzer Beispieltext, Sprache, Stimmprofil, Version, Abtastrate und die Beschreibung, was du erwartest und was du hörst.

Für die Entwicklung:

- [Build und Paketbau](docs/BUILD.md)
- [Technischer Arbeitsstand und offene Fehler](docs/CURRENT-WORK.md)
- [Gewünschte Aussprache- und Klangentscheidungen](GEWUENSCHTE-ABWEICHUNGEN-DE.md)
- [Frühere Versionsnotizen](docs/history/README.md)

Der Synthesekern wird in C gebaut, die NVDA-Anbindung ist in Python geschrieben. Die weitere Vereinfachung und Optimierung des Kerns ist Teil des Projekts. Hinweise zu verwendeten Daten und Komponenten stehen bei den jeweiligen Quellen, unter anderem in [OpenEVV-Lizenz](vendor/openevv61/LICENSE), [OpenEVV-Hinweisen](vendor/openevv61/NOTICE) und [Wörterbuch-Lizenz](data/community/LICENSE.md).
