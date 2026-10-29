# Technischer Arbeitsstand: 0.2.187

Die Projektbeschreibung und Bedienungsanleitungen stehen jetzt getrennt von diesen Entwicklungsnotizen. Ausgangspunkt für die Änderungen ist der unveränderte GitHub-Import von 0.2.186. Der eigene deutsche Akustikplaner, Chinesisch und die Entscheidungen in [GEWUENSCHTE-ABWEICHUNGEN-DE.md](../GEWUENSCHTE-ABWEICHUNGEN-DE.md) bleiben erhalten.

## Korrigiert: Main-Textersetzungen

Deutsche Main-Schlüssel werden beim Zusammenführen und im C-Lexikon nicht mehr mit Root-Schlüsseln zusammengefaltet. Main unterscheidet die Schreibweise; das deutsche Root-Wörterbuch behält seine bisherige Suche ohne Unterscheidung der Groß- und Kleinschreibung.

Gewöhnliche deutsche Main-Ersetzungen werden nicht mehr beim Laden als isolierte Wörter zu festen Lautfolgen kompiliert. Sie werden innerhalb des C-Textplaners einmal aufgelöst und durchlaufen anschließend die gewöhnliche Wort- und Satzanalyse. Dadurch behält etwa eine Ersetzung durch ein schwaches Wort wie „bei“ ihre Rolle im Satz. Bindestrichglieder werden bei derselben Wörterbuchsuche berücksichtigt. Das ist keine Sonderregel für ETI-Eloquence. Explizite Ausspracheangaben und Root-Einträge behalten ihre eigenen Lexikonstufen.

NVDA-Indizes werden von der ursprünglichen Eingabe auf den analysierten Text abgebildet. Bei einem erneuten Analyseversuch bleiben die ursprünglichen Grenzen gültig. Ersetzungen werden innerhalb eines Analyseversuchs nicht rekursiv wieder auf sich selbst angewendet.

`nvda/test_eci_dictionary_semantics.py` vergleicht die tatsächliche native PCM-Ausgabe und Indexpositionen von Ersetzungen mit der normalen Texteingabe. Die 25 Prüfungen umfassen schwache Wörter, verschiedene Schreibweisen, Zahlen, Umlaute, Mehrwort-Ersetzungen, Bindestriche, explizite Ausspracheangaben und zyklische Ersetzungen. PCM bleibt im RAM. Die bereits vorher nicht unterstützte Eingabe `Test-504` gehört nicht zu den behaupteten Korrekturen.

## Korrigiert: geänderte Dic-Dateien übernehmen

Die Wörterbuchverwaltung beobachtet neben ausdrücklich ausgewählten Dateien jetzt einen eigenen Wörterbuchordner. In der NVDA-Einstellungskategorie **Eloquence C** öffnet **Wörterbuchordner öffnen** diesen Ordner. Dort werden die kanonischen Dateinamen aus der Bedienungsanleitung automatisch erkannt, einschließlich neu angelegter, entfernter und beim Speichern ersetzter Dateien.

Bei aktivierten Gemeinschaftswörterbüchern werden auch lokale Änderungen an den mitgelieferten Dic-Dateien berücksichtigt. Ein bearbeitetes Volume ersetzt dieses mitgelieferte Volume, sodass auch entfernte Einträge verschwinden. Ausdrücklich ausgewählte eigene Dateien haben weiterhin Vorrang. Die Herkunftsprüfung heruntergeladener GitHub-Snapshots bleibt bestehen; bearbeitete Dateien werden als lokale Daten behandelt. Erzeugte Cache-Snapshots sind keine Bearbeitungsordner.

Die Prüfung und das Zusammenführen laufen im Hintergrund. Der Sprach-Worker erhält vollständige Datenverzeichnisse. Die 15 Prüfungen in `nvda/test_dictionary_manager.py` decken unter anderem Änderungen, neue Dateien, atomisches Ersetzen, Entfernen, Fehlerbehandlung und das Zusammenwirken mit Community-Updates ab.

## Weiter offen: US-Englisch mit Wörterbüchern

Der gemeldete Absturz in echtem NVDA ist bislang nicht reproduziert. Diese Version enthält deshalb **keine nachgewiesene Korrektur dieses Absturzes**.

Bei der Ursachensuche auf dem Stand 0.2.186 wurden die 70.900 Schlüssel der damaligen US-Main- und Root-Dateien in Satzkontexten geprüft. 859 zunächst mit falscher Eingabekodierung übergebene Schlüssel wurden erneut als korrektes UTF-8 geprüft und vollständig gesprochen. Dabei wurde kein Absturz nachgewiesen. Zusätzliche AddressSanitizer-Prüfungen erfassten Wörterbuchwechsel, Erzeugen/Freigeben, lange verbundene Texte und Abbruch; für einen Teil dieser Läufe wurde auch der private Arena-Allokator instrumentiert. Diese Entwicklungsinstrumentierung gehört nicht zum Add-on.

Der tatsächliche neue Windows-PE-Kern wurde außerdem für zwei Wörterbuchfälle in einem CPU-Test mit modellierten Windows-APIs ausgeführt und mit dem nativen Kern verglichen. Das prüft Maschinenkode und Schnittstellen, ersetzt aber weder echtes Windows noch NVDA und dessen zeitliche Abläufe. Die Linux-Tests und dieser PE-Test bestätigen keine Behebung des Nutzerfehlers.

## Weiter offen: griechische Buchstaben in finnischen und koreanischen Sätzen

Gemeint ist normales Satzlesen. Kurze native Beispiele mit α, β, γ und μ ergeben die Ausgabe der jeweiligen lokalen Buchstabennamen. Damit wurde die gemeldete Abweichung nicht reproduziert; derselbe Befund bestand bereits vor 0.2.187. Eine erneute identische Erfolgsmeldung wäre deshalb kein Nachweis einer Korrektur.

Bei sehr langen Wiederholungen können auch zwei Ausgaben derselben Eingabe geringfügig voneinander abweichen. Ein alleiniger PCM-Hashunterschied solcher langen Läufe wurde deshalb nicht als Beleg für eine falsche Buchstabenlesung gewertet. In dieser Version wurde keine neue griechische Aussprachekorrektur vorgenommen.

## Diagnose beim nächsten NVDA-Test

Beim Laden protokolliert der Treiber die tatsächliche C-Kernversion, den DLL-Pfad und die Sprache. Für diesen Stand muss die Kernversion `0.2.187.0` erscheinen. Auf Debug-Protokollstufe werden zusätzlich die Sprachbereiche und die Unicode-Codepunkte enthaltener griechischer Buchstaben erfasst. Der gesprochene Text wird durch diese neue Diagnose nicht protokolliert.

Für die beiden verbleibenden Fehler fehlen ein kurzer auslösender NVDA-Text und der tatsächliche Ablauf: ausgewählte Sprache, Stimme, Wörterbücher, automatische Sprachumschaltung sowie die Unterscheidung zwischen abgebrochener Sprachausgabe und beendetem NVDA-Prozess. Diese Angaben sind nötig, um den bisher geprüften Pfad mit dem betroffenen Pfad abzugleichen.

## Wiederholbare Prüfungen

Die Build-Schritte stehen in [BUILD.md](BUILD.md). Nach dem nativen Build sind insbesondere diese Prüfungen verfügbar:

```sh
python3 nvda/test_dictionary_manager.py
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

Alle hier beschriebenen Laufzeitprüfungen verarbeiten PCM ausschließlich im RAM. Die öffentliche ABI behält 21 ECI-Symbole. Die Linux-Verknüpfung weist jetzt wie der Windows-Build unaufgelöste Symbole zurück.

Es gelten weiterhin: kein Laufzeit-Bytecode, keine Original-DLL/SYN im Add-on, keine Wiederholungscaches und keine Audiodateien bei Prüfungen. Die historischen Quellarchive bleiben vollständig erhalten; das Größenlimit für ein vollständiges Quellpaket beträgt 512 MiB. Herkunft und Prüfsummen des unveränderten Imports sind in `docs/source-0.2.186.json` dokumentiert. Die früheren Add-on-Prüfnotizen stehen unverändert unter [history](history/README.md).
