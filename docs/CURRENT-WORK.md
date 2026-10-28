# Arbeitsstand nach dem GitHub-Import

Ausgangspunkt ist 0.2.186. Der Import ändert das Laufzeitverhalten nicht und ist kein neues Add-on. Chinesisch und der eigene deutsche Akustikplaner bleiben im angenommenen Stand. Die verbindlichen Ausspracheentscheidungen stehen in `GEWUENSCHTE-ABWEICHUNGEN-DE.md`.

## Offene Fehler aus dem letzten Nutzertest

1. US-Englisch stürzt mit aktivierten Wörterbüchern weiterhin in NVDA ab. Der Fehler ist mit dem bisherigen isolierten Linux-Test nicht reproduziert. Vor einer Erfolgsmeldung müssen insbesondere Aktualisierung, Abbruch, Sprachwechsel und die Lebensdauer der nativen Wörterbuchdaten im tatsächlichen Treiberpfad geprüft werden.
2. Finnisch und Koreanisch behandeln griechische Buchstaben beim normalen Satzlesen weiterhin falsch. Gemeint ist ausdrücklich nicht die einzelne Zeichenlesung. Frühere isolierte C- und Adaptertests reichen als Nachweis nicht aus; der tatsächliche Ablauf mit NVDA-Sprachbefehlen muss geprüft werden.
3. Main-Textersetzungen müssen Groß-/Kleinschreibung beachten und die normale kontextbezogene Analyse durchlaufen. In `nvda/_reloq_dictionary_manager.py` werden deutsche Main-Schlüssel derzeit beim Zusammenführen kleingeschrieben. `vendor/openevv61/src/reloq61_dictionary.inc` kompiliert einfache Textersetzungen bereits beim Laden zu festen Lautfolgen. Der deutsche Lexikon-Resolver in `vendor/openevv61/src/eci/dict/eci_usrdct.c` faltet Main und Root ebenfalls zusammen. Diese Stufen müssen passend zu den jeweiligen Wörterbuchvolumes getrennt werden. Ein einzelner Sonderfall für ETI-Eloquence genügt nicht.
4. Änderungen von Dic-Dateien im vorgesehenen Ordner werden beim Nutzer nicht übernommen. Der bestehende Monitor beobachtet nur ausdrücklich registrierte eigene Quelldateien. Er beobachtet weder den gesamten mitgelieferten/aktiven Wörterbuchordner noch neu angelegte Dateien darin. Die Herkunftsprüfung eines heruntergeladenen Community-Snapshots muss erhalten bleiben; lokale Bearbeitung braucht einen klaren, verlässlich beobachteten Weg.

Diese Punkte sind weiterhin offen. Insbesondere ist eine bestandene Linux-Prüfung keine Bestätigung, dass ein NVDA-Absturz behoben wurde.

## Geprüfter Import

- Die vollständige historische Quelle wurde mit ihrem dokumentierten SHA-256 geprüft. `docs/source-0.2.186.json` enthält die Herkunft und Prüfsummen der unveränderten aktiven Dateien.
- Die große Unicode-Namentabelle wurde lediglich an Zeilengrenzen auf mehrere C-Includes verteilt. Zusammengesetzt ergeben sie exakt die ursprünglichen Bytes; Präprozessor-Ausgabe und erzeugte Objektdatei waren vor und nach der Aufteilung bytegleich.
- Der veröffentlichte Git-Dateibaum wurde vor dem Commit mit dem lokalen Git-Dateibaum abgeglichen.
- Frisch gebauter nativer Kern: öffentliche ABI mit 21 ECI-Symbolen, Diagnostikprüfung und 13 Tests der Wörterbuchverwaltung bestanden.
- Eine zusätzliche Linux-Serie mit 180 Äußerungen, wiederholtem Wörterbuchwechsel und Neuerzeugen/Freigeben des US-Englisch-Kerns lief durch. Sie reproduzierte den gemeldeten NVDA-Absturz nicht. PCM wurde ausschließlich im RAM verarbeitet.

Der wiederholbare Lebensdauertest liegt unter `tools/check_dictionary_lifetime.py`; zum kurzen Prüfen nach dem nativen Build:

```sh
python3 -X faulthandler tools/check_dictionary_lifetime.py --cycles 1
```

Für die nächste Version gelten weiterhin: kein Laufzeit-Bytecode, keine Original-DLL/SYN im Add-on, keine Wiederholungscaches und keine Audiodateien bei Prüfungen. Historische Quellarchive bleiben vollständig erhalten; das Größenlimit für ein vollständiges Quellpaket beträgt 512 MiB.
