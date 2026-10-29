# Technischer Arbeitsstand: 0.2.188

Die Projektbeschreibung und Bedienungsanleitungen stehen jetzt getrennt von diesen Entwicklungsnotizen. Ausgangspunkt für die Änderungen ist der unveränderte GitHub-Import von 0.2.186. Der eigene deutsche Akustikplaner, Chinesisch und die Entscheidungen in [GEWUENSCHTE-ABWEICHUNGEN-DE.md](../GEWUENSCHTE-ABWEICHUNGEN-DE.md) bleiben erhalten.

## Korrigiert: Main-Textersetzungen

Deutsche Main-Schlüssel werden beim Zusammenführen und im C-Lexikon nicht mehr mit Root-Schlüsseln zusammengefaltet. Main unterscheidet die Schreibweise; das deutsche Root-Wörterbuch behält seine bisherige Suche ohne Unterscheidung der Groß- und Kleinschreibung.

Gewöhnliche deutsche Main-Ersetzungen werden nicht mehr beim Laden als isolierte Wörter zu festen Lautfolgen kompiliert. Sie werden innerhalb des C-Textplaners einmal aufgelöst und durchlaufen anschließend die gewöhnliche Wort- und Satzanalyse. Dadurch behält etwa eine Ersetzung durch ein schwaches Wort wie „bei“ ihre Rolle im Satz. Bindestrichglieder werden bei derselben Wörterbuchsuche berücksichtigt. Das ist keine Sonderregel für ETI-Eloquence. Explizite Ausspracheangaben und Root-Einträge behalten ihre eigenen Lexikonstufen.

NVDA-Indizes werden von der ursprünglichen Eingabe auf den analysierten Text abgebildet. Bei einem erneuten Analyseversuch bleiben die ursprünglichen Grenzen gültig. Ersetzungen werden innerhalb eines Analyseversuchs nicht rekursiv wieder auf sich selbst angewendet.

`nvda/test_eci_dictionary_semantics.py` vergleicht die tatsächliche native PCM-Ausgabe und Indexpositionen von Ersetzungen mit der normalen Texteingabe. Die 25 Prüfungen umfassen schwache Wörter, verschiedene Schreibweisen, Zahlen, Umlaute, Mehrwort-Ersetzungen, Bindestriche, explizite Ausspracheangaben und zyklische Ersetzungen. PCM bleibt im RAM. Die bereits vorher nicht unterstützte Eingabe `Test-504` gehört nicht zu den behaupteten Korrekturen.

## Wörterbuchordner und Aktualisierung

Der Bearbeitungsort ist wieder ausschließlich der vorhandene Ordner `synthDrivers/reloqC/communityDictionaries` im Add-on. **Wörterbuchordner öffnen** öffnet diesen Ordner. Der zusätzliche Bearbeitungsordner aus 0.2.187 wird nicht mehr angelegt oder automatisch ausgewertet.

Bei aktivierten Gemeinschaftswörterbüchern erkennt der Monitor geänderte, neu angelegte, entfernte und beim Speichern ersetzte Dic-Dateien im Add-on. Ausdrücklich ausgewählte eigene Quelldateien werden weiterhin beobachtet. Die 18 Verwaltungsprüfungen umfassen auch das Aktualisieren der installierten Dateien aus dem aktuellen GitHub-Master und den Erhalt lokaler Änderungen.

Downloads und vollständige unveränderliche Worker-Snapshots liegen intern unter `.cache` im selben Add-on-Ordner. Der Sprach-Worker liest solche Snapshots statt der gerade im Editor bearbeiteten Dateien. Eine spätere Änderung kann damit keine bereits übergebenen Daten verändern. Die Auswahl eigener Quelldateien wird separat als `reloqCDictionaries.json` im NVDA-Profil gespeichert und bleibt bei einer Add-on-Neuinstallation erhalten. Bestehende Auswahlen aus dem früheren Speicherort werden übernommen. Vorhandene alte Dateien werden nicht gelöscht.

Community-Updates aktualisieren unveränderte installierte Volumes. Lokal bearbeitete oder entfernte Volumes bleiben als lokale Entscheidung erhalten und haben Vorrang. Eigene Quelldateien behalten ihre bisherige Reihenfolge und Priorität.

## US-Englisch: Einfrieren statt Absturz

Der Nutzer hat präzisiert, dass NVDA einfriert. Ein Engpass wurde jetzt mit dem vollständigen US-Wörterbuchsatz nachgewiesen: Beim Ersetzen oder Abschalten des Wörterbuchs wurden sehr viele kleine Speicherblöcke in Hash-Bucket-Reihenfolge freigegeben. Das Einfügen dieser verteilten Blöcke in die geordnete Freiliste führte zu quadratischem Aufwand.

Große Hash-Tabellen erfassen ihre eigenen Blöcke jetzt vor dem Freigeben und geben sie nach Adresse sortiert zurück. Benachbarte freie Blöcke können sofort zusammengeführt werden. Besitzregeln für Schlüssel und Werte bleiben erhalten; die Sortierliste existiert nur während der Freigabe. Kleine Tabellen und ein fehlgeschlagener temporärer Speicherzugriff behalten den bisherigen Freigabepfad.

Drei lokale Linux-Läufe brauchten zuvor etwa 9,7 bis 12,6 Sekunden zum erneuten Laden und 9,0 bis 9,7 Sekunden zum Abschalten. Nach der Änderung lagen Reloads bei etwa 0,16 bis 0,19 Sekunden und das Abschalten bei etwa 0,02 bis 0,03 Sekunden. Ausgabe und Indexpositionen vor und nach dem Neuladen waren in den geprüften Fällen bytegleich. Weitere Läufe prüfen auch 48 kHz. Die Werte sind keine NVDA- oder Dell-Messungen.

Zusätzlich hält `LanguageEngine.close()` die Abbruch-Sperre nicht mehr während des nativen Aufräumens. Ein gleichzeitiger Abbruch kann den bereits abgetrennten Zustand sofort sehen. Ein gezielt blockierter nativer Close wird in den Steuerungstests geprüft. Der reale NVDA-Queue-Pfad wird außerdem mit dem tatsächlichen nativen Kern, vollständigen Wörterbüchern, wiederholtem Neuladen/Abbrechen und einer 48-kHz-Folgeausgabe geprüft; der Player wird dabei simuliert.

Die Hash-Besitzprüfungen laufen mit normalen und großen Tabellen, Zeichenketten- und Zahlenschlüsseln, geliehenen Daten und benachbarten unbeteiligten Blöcken. Zusätzliche AddressSanitizer-Prüfungen instrumentieren auch den privaten Arena-Speicher. Diese Entwicklungsinstrumentierung gehört nicht zum Add-on. LeakSanitizer ist in dieser Prüfungsumgebung deaktiviert; die Speicherzugriffsprüfung und expliziten Besitzprüfungen bleiben aktiv.

Die frühere breit angelegte Corpus-Prüfung hat keinen Absturz nachgewiesen. Die gefundene lange Blockade ist jetzt beseitigt; ob damit das gemeldete Einfrieren in echtem NVDA vollständig behoben ist, muss auf dem betroffenen System bestätigt werden. Beim Laden protokolliert der Treiber die tatsächliche C-Kernversion (`0.2.188.0`), DLL-Pfad und Sprache.

## Griechische Buchstaben in Finnisch und Koreanisch: geklärt

Der Nutzer hat den Fehler auf NVDA zurückgeführt: Dessen eigene Symbolverarbeitung übernimmt die Buchstaben, bevor sie den Synthesekern erreichen. Dieser Punkt erfordert keine Aussprachekorrektur im C-Kern. Die zusätzlichen Prüfungen des griechischen Alphabets und seiner Varianten zeigten in den nativen finnischen und koreanischen Satzkontexten ebenfalls keine ausgelassenen Namen. Die vorläufige griechische Routing-Diagnose wurde wieder entfernt.

## Weitere Plattformen und Ressourcen

NVDA ist die erste Anbindung und die derzeitige praktische Testplattform. Als Nächstes stehen geringere Paketgröße, Speicherverbrauch und CPU-Last des gemeinsamen C-Kerns an. Danach folgen SAPI5 für Windows und Android. Beide Anbindungen sind geplant, noch nicht implementiert. Ausspracheentscheidungen und der eigene deutsche Akustikplaner bleiben erhalten.

## Wiederholbare Prüfungen

Die Build-Schritte stehen in [BUILD.md](BUILD.md). Nach dem nativen Build sind insbesondere diese Prüfungen verfügbar:

```sh
python3 nvda/test_dictionary_manager.py
python3 nvda/test_dictionaries.py
python3 -X faulthandler nvda/test_eci_dictionary_transport.py
python3 nvda/test_eci_dictionary_semantics.py
python3 addon/test_synth.py
python3 addon/test_synth.py de
python3 nvda/test_eci_transport.py
python3 -X faulthandler tools/check_dictionary_lifetime.py --cycles 1
```

Der Lebensdauertest kann zusätzlich Schlüssel einer nativen Windows-1252-Dic-Datei als korrektes UTF-8 in Satzkontexten sprechen:

```sh
python3 -X faulthandler tools/check_dictionary_lifetime.py --corpus data/community/ENUmain.dic
```

Lade-/Freigabezeiten und die Ausgabeidentität nach einem Reload sind separat messbar:

```sh
python3 tools/check_dictionary_reload.py --rate 6
make -C eci test_reloq_hash_delete
./eci/test_reloq_hash_delete
```

Alle hier beschriebenen Laufzeitprüfungen verarbeiten PCM ausschließlich im RAM. Die öffentliche ABI behält 21 ECI-Symbole. Die Linux-Verknüpfung weist jetzt wie der Windows-Build unaufgelöste Symbole zurück.

Es gelten weiterhin: kein Laufzeit-Bytecode, keine Original-DLL/SYN im Add-on, keine Wiederholungscaches und keine Audiodateien bei Prüfungen. Die historischen Quellarchive bleiben vollständig erhalten; das Größenlimit für ein vollständiges Quellpaket beträgt 512 MiB. Herkunft und Prüfsummen des unveränderten Imports sind in `docs/source-0.2.186.json` dokumentiert. Die früheren Add-on-Prüfnotizen stehen unverändert unter [history](history/README.md).
