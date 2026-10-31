# Eloquence C – Bedienungsanleitung

Eloquence C ist die NVDA-Erweiterung des Projekts [ReLoq C](https://github.com/hozosch/Reloq-C). Sie entwickelt den Eloquence-Klang mit gezielten Aussprachekorrekturen, Unicode-Unterstützung und zusätzlichen Klangmöglichkeiten weiter. NVDA ist die erste Anbindung und dient derzeit zum Prüfen der Sprachausgabe. Für Android gibt es eine erste ARM64-Testversion; SAPI5 ist geplant.

## Installieren und auswählen

Benötigt wird NVDA ab 2026.1 mit einem 64-Bit-Prozess. Das Paket enthält eine Windows-x64-DLL.

1. Lade eine `.nvda-addon`-Datei aus den [Releases](https://github.com/hozosch/Reloq-C/releases), sobald dort ein Paket angeboten wird.
2. Öffne die Datei mit NVDA, bestätige die Installation und starte NVDA neu.
3. Drücke **NVDA+Strg+S**, wähle **Eloquence C (experimentell)** und bestätige.
4. Wähle in den NVDA-Einstellungen unter **Sprachausgabe** die gewünschte Sprache und Stimme.

Die ZIP-Datei des Repository-Quellcodes ist kein installierbares Add-on. Weitere Bedienungshinweise bietet das [NVDA-Benutzerhandbuch](https://download.nvaccess.org/documentation/de/userGuide.html).

## Sprachen und Stimmen

Es gibt 14 Sprachauswahlen. Die beiden chinesischen Schriftvarianten teilen sich eine Auswahl.

| Sprache | Wörterbuch-Präfix |
| --- | --- |
| Chinesisch (Mandarin) | CHS |
| Deutsch | DEU |
| Britisches Englisch | ENG |
| Amerikanisches Englisch | ENU |
| Lateinamerikanisches Spanisch | ESM |
| Spanisch aus Spanien | ESP |
| Finnisch | FIN |
| Französisch | FRA |
| Kanadisches Französisch | FRC |
| Italienisch | ITA |
| Japanisch | JPN |
| Koreanisch | KOR |
| Brasilianisches Portugiesisch | PTB |
| Griechisch | ELL |

Die Stimmprofile heißen **Reed, Shelly, Bobby, Rocko, Glen, Sandy, Grandma und Grandpa**. Die Sprachwahl legt die Aussprache fest, die Stimmwahl ein Klangprofil. NVDA kann bei entsprechend ausgezeichneten Texten automatisch zwischen unterstützten Sprachen wechseln, wenn diese NVDA-Option aktiviert ist.

## Die Stimme einstellen

Die folgenden Optionen befinden sich in den Sprachausgaben-Einstellungen:

- **Geschwindigkeit und Sprachgeschwindigkeit anheben:** Lesetempo und erweiterter Geschwindigkeitsbereich.
- **Stimmhöhe, Betonung und Lautstärke:** Grundtonhöhe, Umfang der Sprachmelodie und Wiedergabepegel.
- **Kopfgröße:** verändert die Resonanzen und damit den Stimmcharakter.
- **Rauigkeit und Behauchtheit:** zusätzliche Eigenschaften der Stimmquelle.
- **Abtastrate:** 8, 11,025, 16, 22,05, 32, 44,1 oder 48 kHz. Voreingestellt sind 11,025 kHz. Höhere Raten erweitern die Klangbandbreite und benötigen mehr Rechenleistung. Bei Aussetzern hilft zum Eingrenzen ein Vergleich mit 11,025 oder 22,05 kHz.

**Pausen verkürzen** hat drei Werte: **Nie**, **Nur am Textende** und **Immer**. Die Kürzung betrifft automatisch erzeugte Pausen. Ausdrücklich angegebene Pausen behalten ihre Dauer.

**ECI-Tags verarbeiten** ist zunächst ausgeschaltet. Eingeschaltet können unterstützte Backtick-Befehle im Text Aussprache, Buchstabierung, Betonung, Stimmparameter, Stimmen und Sprachen steuern. Für gewöhnliches Lesen ist die Option nicht erforderlich. Eigene Aussprachewörterbücher können Ausspracheangaben unabhängig von dieser Einstellung enthalten.

**Abkürzungswörterbuch verwenden** schaltet die native Abkürzungsauflösung und ist zunächst eingeschaltet. Haupt- und Stammwörterbücher für die Aussprache bleiben davon unabhängig aktiv. **Satzvorhersage** ist zunächst ausgeschaltet; eingeschaltet ermittelt der Sprachkern Phrasengrenzen in Text ohne Satzzeichen. Beide Einstellungen stehen auch unter Android zur Verfügung und gelten ab der nächsten Syntheseanfrage. Gleiche Geschwindigkeitswerte behalten das Dauermodell der jeweiligen Sprache bei; sie garantieren keine gleiche Anzahl Wörter oder Silben pro Minute.

Deutsch verwendet wie die übrigen Sprachen das eingebaute C-Wörterbuch. Die zusätzliche deutsche Lexikondatei früherer Versionen wird nicht mehr benötigt; die optionalen Aussprachewörterbücher bleiben verfügbar.

Emoji-Namen werden beim normalen Lesen und bei der Zeichennavigation in der gewählten Sprache gesprochen, einschließlich Hautfarben, Flaggen, Tastensymbolen und verbundenen Emojis. Das liegt im gemeinsamen Kern und funktioniert auch unter Android. Gewöhnliche Ziffern und Satzzeichen behalten ihre bisherige Behandlung; die Auswahl der Textdarstellung wird berücksichtigt. NVDA kann vor der Übergabe an die Sprachausgabe außerdem seine eigene Symbolaussprache anwenden.

## Aussprachewörterbücher

Wähle zuerst Eloquence C als aktive Sprachausgabe. Öffne dann **NVDA-Einstellungen → Eloquence C**.

Mit **Gemeinschaftswörterbücher für die Aussprache verwenden** ergänzt du Deutsch und US-Englisch um Einträge aus [eigencrow/IBMTTSDictionaries](https://github.com/eigencrow/IBMTTSDictionaries). Die Option ist zunächst ausgeschaltet. **Wörterbücher jetzt aktualisieren** prüft den aktuellen Zweig `master`; eine neue vollständige Version wird im Hintergrund übernommen. Für diese Aktualisierung wird Internet benötigt.

Mit **Datei hinzufügen** kannst du eigene Dic-Dateien auswählen und ihnen Sprache und Wörterbuchtyp zuordnen. Die Dateien dürfen beliebig heißen und bleiben an ihrem bisherigen Ort. Spätere Dateien in der Liste haben Vorrang. **OK** oder **Übernehmen** speichert die Auswahl.

Mit **Wörterbuchordner öffnen** erreichst du den vorhandenen Ordner `synthDrivers/reloqC/communityDictionaries` im Add-on. Dort liegen die mitgelieferten Dateien; dort werden auch Änderungen in einem Editor erkannt. Bei aktivierten Gemeinschaftswörterbüchern kannst du weitere Dateien mit dem Präfix aus der Sprachtabelle und einem dieser Namen ergänzen:

- `main.dic` für das Hauptwörterbuch, zum Beispiel `DEUmain.dic` oder `ENUmain.dic`.
- `Root.dic` für das Stammwörterbuch, zum Beispiel `DEURoot.dic`.
- `abbr.dic` für Abkürzungen, zum Beispiel `DEUabbr.dic`.

Die Dateien verwenden das native Dic-Format: Windows-1252, ein Eintrag pro Zeile und ein Tabulator zwischen Suchwort und Ersetzung. Main unterscheidet Groß- und Kleinschreibung; gewöhnliche Textersetzungen durchlaufen anschließend die normale Textanalyse. Stamm- und Ausspracheangaben werden an der jeweiligen Wörterbuchstelle verarbeitet.

Änderungen an eingebundenen Dateien und Dateien im Wörterbuchordner werden im Hintergrund erkannt. Das gilt auch für neu angelegte Dateien und für Editoren, die eine Datei beim Speichern ersetzen. Bei ungültigen Daten bleibt der letzte gültige Stand aktiv. Gemeinschaftswörterbuch-Updates aktualisieren die Dateien in diesem Add-on-Ordner. Eigene eingebundene Dateien und lokal bearbeitete oder entfernte Wörterbuchdateien werden dabei erhalten. Ein lokal bearbeitetes Volume hat Vorrang vor der heruntergeladenen Fassung. Es wird kein zusätzlicher Bearbeitungsordner im NVDA-Hauptverzeichnis angelegt.

Unter **Add-on-Updates** prüft **Nach Add-on-Updates suchen** die veröffentlichten GitHub-Releases. Ist ein neueres Paket verfügbar, prüft **Update herunterladen und installieren** den Download und öffnet NVDAs normalen Installations- und Bestätigungsdialog. Die Prüfung läuft nur auf Anforderung. Wenn noch kein Paket veröffentlicht wurde, zeigt der Bereich dies an. **Wörterbücher jetzt aktualisieren** bleibt davon unabhängig.

## Fehler melden

Das Projekt ist noch experimentell. Der Nutzer hat das frühere Einfrieren und die spätere Initialisierungsverzögerung bei US-Englisch mit Wörterbüchern mit 0.2.190 als behoben bestätigt. Das Überspringen griechischer Buchstaben in finnischen und koreanischen Sätzen wurde inzwischen auf die Symbolverarbeitung von NVDA vor der Übergabe an den Synthesekern zurückgeführt. Technische Details stehen im [Arbeitsstand](https://github.com/hozosch/Reloq-C/blob/main/docs/CURRENT-WORK.md).

Für eine [Fehlermeldung](https://github.com/hozosch/Reloq-C/issues) helfen ein kurzer Beispieltext, Sprache, Stimme, Version, Abtastrate und die Angabe, ob Wörterbücher eingeschaltet sind. Beschreibe auch, ob die Ausgabe abbricht, ein Wort falsch klingt oder NVDA selbst beendet wird.
