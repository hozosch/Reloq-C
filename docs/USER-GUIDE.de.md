# Eloquence C – Bedienungsanleitung

## Version 1.4.0

Die Zahlen 12 und 19 verwenden auf Brasilianisch-Portugiesisch auch in größeren Zahlen die üblichen Vokale. ECI-Betonungsbefehle verwenden dieselbe Sprachmelodie wie der entsprechende Reglerwert.

In der NVDA-Einstellungskategorie Eloquence C kann die Zerlegung unbekannter englischer Wörter eingeschaltet werden. Sie ist zunächst deaktiviert und gilt unabhängig von der Groß- und Kleinschreibung für amerikanisches und britisches Englisch. Bei vollständig großgeschriebenen Wörtern bleibt Eloquences eigene Buchstabierung erhalten, wenn der Sprachkern das Wort buchstabiert; andere können weiterhin zerlegt werden. Erkannte Wörter und Wörterbucheinträge bleiben möglichst erhalten; die Erkennung kann sich bei unbekannten Namen und Fremdwörtern irren.

Die getrennte Option **Camel-Case-Wörter zerlegen (alle Sprachen)** setzt Wortgrenzen bei Binnen-Großbuchstaben, zum Beispiel `meinWort` und `HTTPServer`. Sie ist zunächst ausgeschaltet. NVDA kann mit seinem eigenen eingebauten Sprachwörterbuch unabhängig davon ebenfalls Camel Case aufteilen. Eigene NVDA- und native Wörterbucheinträge haben Vorrang vor der zusätzlichen Zerlegung.

Unter „Eigene Wörterbücher“ verwaltest du NVDA-Sprachwörterbücher und Eloquence-Aussprachewörterbücher gemeinsam. Mit „Datei hinzufügen“ wird das Format vorgeschlagen; bestätige oder ändere es und wähle die Sprache. Für NVDA-Dateien wählst du zusätzlich eine Stimme oder alle Stimmen, für Eloquence-Dateien den Wörterbuchtyp. Reguläre Ausdrücke, Groß-/Kleinschreibung und Kommentare der NVDA-Dateien bleiben erhalten. Eine importierte NVDA-Kopie lässt sich bearbeiten oder entfernen; die Originaldatei bleibt unverändert. NVDA-Importe und Bearbeitungen gelten sofort, Eloquence-Dateien werden mit OK oder Übernehmen gespeichert. Zuvor versehentlich als Eloquence-Dateien ausgewählte NVDA-Wörterbücher werden beim Start passend übernommen.

## Version 1.2.2

Die Grundtonhöhe lässt sich optional exakt in Hertz einstellen. Bei Deutsch wird das finale A auch vor Komma, Doppelpunkt und weiteren Pausen verwendet. Bei einer Neuinstallation in NVDA entspricht die Standardtonhöhe 65 der voreingestellten Stimme Reed.

## Version 1.2.1

Der weiche Modus wurde klanglich verfeinert: Er dämpft die stimmhaften Anteile breiter, verzichtet auf zusätzliche Richness und reduziert die Behauchung bei stimmhaften Lauten.

## Version 1.2.0

Die griechische Aussprache und Satzmelodie wurden verbessert. Die erste Sprachausgabe wartet auf die ausgewählten Wörterbücher, Lautstärke und SAPI-Reaktion wurden verbessert und ein Punkt nach einem einzelnen Buchstaben kann einen englischen Satz beenden. Der weiche Modus und die ursprüngliche Frageintonation sind als Optionen verfügbar.

## Version 1.1.3

Ein Absturz bei bestimmten langen Texten wurde behoben. HTTPS-Adressen nutzen die HTTP-Verarbeitung. Die Aussprache englischer Wörter in der griechischen Stimme wurde verbessert.

## Version 1.1.2

Ein abgesetzter Bindestrich vor einer Zahl wird als Pause gelesen. Chinesisch und Englisch wechseln flüssiger, die chinesische Betonung bleibt erhalten und deutsche Zahlwortkorrekturen gelten auch bei Tausendern.

## Version 1.1.1

Der Sprachkern berücksichtigt Gedankenstriche in den getesteten europäischen Sprachen auch ohne Leerzeichen. Bei der ausdrücklichen Zeichenlesung wird „Unterstrich“ als Nomen betont.

## Version 1.1.0

Der deutsche E-I-Gleitlaut ist auf Öffnung 85, Rücklage 45 und Weichheit 45 abgestimmt. Die deutsche Satz- und Lautplanung verbessert unter anderem Bindestrichwörter, Zahlwörter und Fragemelodie. In englischer Analyse werden bei „recosp“ der vollständige Stamm und bei „tzsche“ der Schlussvokal erhalten.

## Englische Uhrzeiten mit Sekunden

Englische Uhrzeiten wie `13:28:25` behalten die normale Stunden-/Minutenlesung und ergänzen „and twenty five seconds“. Bei einer Sekunde steht der Singular; Sekundenbruchteile bleiben in der gewöhnlichen Zahlenlesung. Dies gilt für US- und britisches Englisch im nativen Kern, auch für NVDA und andere ECI-Programme. Quellindizes bleiben ihren ursprünglichen Textpositionen zugeordnet.

Eloquence C ist die NVDA-Erweiterung des Projekts [ReLoq C](https://github.com/hozosch/Reloq-C). Sie entwickelt den Eloquence-Klang mit gezielten Aussprachekorrekturen, Unicode-Unterstützung und zusätzlichen Klangmöglichkeiten weiter. NVDA ist die erste Anbindung und dient derzeit zum Prüfen der Sprachausgabe. Derselbe Kern ist auch für Android mit ARM64 oder x86-64 und für Windows SAPI5 verfügbar.

## Installieren und auswählen

Benötigt wird NVDA ab 2026.1 mit einem 64-Bit-Prozess. Das Paket enthält eine Windows-x64-DLL.

1. Lade eine `.nvda-addon`-Datei aus den [Releases](https://github.com/hozosch/Reloq-C/releases), sobald dort ein Paket angeboten wird.
2. Öffne die Datei mit NVDA, bestätige die Installation und starte NVDA neu.
3. Drücke **NVDA+Strg+S**, wähle **Eloquence C** und bestätige.
4. Wähle in den NVDA-Einstellungen unter **Sprachausgabe** die gewünschte Sprache und Stimme.

Beim ersten Start übernimmt Eloquence C die NVDA-Sprache, sofern sie
unterstützt wird. Eine gespeicherte Sprachwahl hat Vorrang.

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

**Ursprüngliche Frageintonation verwenden** stellt für Deutsch und US-Englisch die Fragekontur des Sprachkerns ein. Die Option ist zunächst ausgeschaltet; dann bleibt die ansteigende Frageintonation erhalten.

**Weicher Modus** senkt ab 16 kHz die Präsenz stimmhafter Anteile bei etwa 3 bis 4 kHz und lässt ab 22 kHz die zusätzliche Richness weg. Er reduziert außerdem die Behauchung stimmhafter Laute und ist zunächst ausgeschaltet.

Unter **NVDA-Einstellungen → Eloquence C → Genaue Grundtonhöhe** kann die Grundtonhöhe zusätzlich als ganze Zahl in Hertz angegeben werden. Aktiviere dazu **Grundtonhöhe in Hertz verwenden**, gib den Wert ein und wähle **OK** oder **Übernehmen**. Die normale NVDA-Stimmhöhe behält ihre ECI-Skala von 0 bis 100. Wird dieser Regler verändert, schaltet Eloquence mit einer Ansage zurück auf die ECI-Skala; der Hertz-Wert bleibt für eine spätere erneute Aktivierung gespeichert. Stimmen- und Sprachwechsel behalten die aktivierte Hertz-Vorgabe bei. Bei Betonung 0 lässt sich die Grundtonhöhe gut überprüfen; Tonsprachen können weiterhin sprachliche Tonverläufe verwenden.

Unter **NVDA-Einstellungen → Eloquence C → Genaue Sprechgeschwindigkeit** kann die Geschwindigkeit alternativ als ganze Zahl von 1 bis 6000 Wörtern pro Minute eingegeben werden. Die normale NVDA-Geschwindigkeit bleibt als eigener Wert erhalten. Wird der normale Regler oder die Geschwindigkeitsverstärkung verändert, schaltet Eloquence mit einer Ansage auf die normale Skala zurück; der WPM-Wert bleibt gespeichert. Die tatsächliche Wortzahl hängt von Sprache, Text und Pausen ab.

Hertz und WPM ändern die laufende Sprachausgabe bereits im Dialog. **Übernehmen** oder **OK** speichert die aktuellen Werte; **Abbrechen** stellt den zuletzt übernommenen Zustand wieder her.

Unter **NVDA-Einstellungen → Eloquence C → Equalizer** kann die Klangfarbe optional mit Tiefen, Mitten und Höhen von jeweils -12 bis +12 dB verändert werden. Die Voreinstellungen „Ausgewogen“, „Warm“, „Hell“ und „Sanft“ dienen als Ausgangspunkt. Sobald ein Band geändert wird, entsteht eine benutzerdefinierte Einstellung, die auch beim Wechsel zu einer Voreinstellung erhalten bleibt. Die Änderungen sind sofort hörbar und folgen ebenfalls Übernehmen, OK und Abbrechen. Der Equalizer ist zunächst ausgeschaltet; ausgeschaltet bleibt die Ausgabe unverändert. „Ausgewogen“ setzt die drei einfachen Bereiche auf null, lässt aber erweiterte Filter aktiv. Unter „Erweiterte Filter“ lassen sich fünf Filter einzeln aktivieren und mit Frequenz (1–24000 Hz), Verstärkung (-12 bis +12 dB) und Bandbreite (10–400 Prozent einer Oktave) einstellen. 100 Prozent entspricht einer Oktave. Frequenzen ab der halben Abtastrate sind ohne Wirkung, bleiben aber gespeichert. Der Pegel wird nur begrenzt, wenn das bearbeitete Signal sonst übersteuern würde.

**ECI-Tags verarbeiten** ist zunächst ausgeschaltet. Eingeschaltet können unterstützte Backtick-Befehle im Text Aussprache, Buchstabierung, Betonung, Stimmparameter, Stimmen und Sprachen steuern. Für gewöhnliches Lesen ist die Option nicht erforderlich. Eigene Aussprachewörterbücher können Ausspracheangaben unabhängig von dieser Einstellung enthalten.

Mit `` `vwpm150 `` kann die folgende Sprache über die native WPM-Zeitsteuerung auf 150 eingestellt werden. Ganzzahlen von 1 bis 6000 werden akzeptiert; `` `vs50 `` wechselt zurück zur normalen ECI-Geschwindigkeitsskala. Bei 1 bis 4 WPM werden die Steuerdaten während der Synthese zusätzlich verlangsamt, um Überläufe in den Sprachregeln zu vermeiden; die Tonhöhe bleibt erhalten. Der Hörwert hängt von Sprache, Text und Pausen ab; sehr große Werte können an Grenzen stoßen. Die Befehle ändern keine gespeicherten Regler.

Mit `` `vbhz150 `` lässt sich die Grundtonhöhe für den folgenden Text auf 150 Hertz setzen; möglich sind ganzzahlige Werte von 1 bis 24.000 Hertz. `` `vb65 `` wechselt zurück zur üblichen ECI-Skala. Für eine Messung kann zusätzlich `` `vf0 `` die Sprachmelodie ausschalten; in Tonsprachen können sprachliche Tonverläufe dennoch erhalten bleiben. Die Abtastrate muss höher als das Doppelte der gewünschten Frequenz sein; sehr hohe Werte können durch die Stimmfilter stark gedämpft werden. Bei unzureichender Abtastrate kann das Ergebnis verfälscht sein oder die Synthese abbrechen. Die Tags ändern nicht die gespeicherten Einstellungen.

**Abkürzungswörterbuch verwenden** schaltet die native Abkürzungsauflösung und ist zunächst eingeschaltet. Haupt- und Stammwörterbücher für die Aussprache bleiben davon unabhängig aktiv. **Satzvorhersage** ist zunächst ausgeschaltet; eingeschaltet ermittelt der Sprachkern Phrasengrenzen in Text ohne Satzzeichen. Beide Einstellungen stehen auch unter Android zur Verfügung und gelten ab der nächsten Syntheseanfrage. Gleiche Geschwindigkeitswerte behalten das Dauermodell der jeweiligen Sprache bei; sie garantieren keine gleiche Anzahl Wörter oder Silben pro Minute.

Deutsch verwendet wie die übrigen Sprachen das eingebaute C-Wörterbuch. Die zusätzliche deutsche Lexikondatei früherer Versionen wird nicht mehr benötigt; die optionalen Aussprachewörterbücher bleiben verfügbar.

Emoji-Namen werden beim normalen Lesen und bei der Zeichennavigation in der gewählten Sprache gesprochen, einschließlich Hautfarben, Flaggen, Tastensymbolen und verbundenen Emojis. Das liegt im gemeinsamen Kern und funktioniert auch unter Android. Gewöhnliche Ziffern und Satzzeichen behalten ihre bisherige Behandlung; die Auswahl der Textdarstellung wird berücksichtigt. NVDA kann vor der Übergabe an die Sprachausgabe außerdem seine eigene Symbolaussprache anwenden.

## Aussprachewörterbücher

Wähle zuerst Eloquence C als aktive Sprachausgabe. Öffne dann **NVDA-Einstellungen → Eloquence C**.

Mit **Gemeinschaftswörterbücher für die Aussprache verwenden** ergänzt du Deutsch und US-Englisch um Einträge aus [eigencrow/IBMTTSDictionaries](https://github.com/eigencrow/IBMTTSDictionaries). Die Option ist zunächst ausgeschaltet. **Wörterbücher jetzt aktualisieren** prüft den aktuellen Zweig `master`; eine neue vollständige Version wird im Hintergrund übernommen. Für diese Aktualisierung wird Internet benötigt.

Unter **Wörterbuchquelle** kannst du zusätzlich **Alternative IBM-TTS-Wörterbücher** auswählen. Diese Quelle enthält derzeit US-englische und kastilisch-spanische Einträge, aber kein deutsches Wörterbuch. Beim ersten Gebrauch wird sie im Hintergrund heruntergeladen; eigene Dateien behalten Vorrang.

Nach einer Aktualisierung erscheint ein Dialog mit der Zahl neuer, unterschiedlicher Einträge und neuer Wörterbuchdateien. Änderungen oder Entfernungen zählen nicht als Zugänge. Beim ersten Download werden die Gesamtzahlen angezeigt; falls der vorherige Stand nicht vergleichbar ist, weist der Dialog darauf hin. Online-Updates übernehmen derzeit einen festen Satz unterstützter Dateinamen; andere neu hinzugefügte Dateien werden nicht automatisch heruntergeladen.

Mit **Datei hinzufügen** wählst du ein NVDA- oder Eloquence-Wörterbuch und seine Sprache. Eine NVDA-Datei wird als eigene Kopie importiert, die du über die Liste bearbeiten kannst; die Originaldatei bleibt unverändert. Ein Eloquence-Wörterbuch bleibt an seinem bisherigen Ort. Bei mehreren Eloquence-Dateien haben spätere Dateien Vorrang; **OK** oder **Übernehmen** speichert ihre Auswahl.

Mit **Wörterbuchordner öffnen** erreichst du den vorhandenen Ordner `synthDrivers/reloqC/communityDictionaries` im Add-on. Dort liegen die mitgelieferten Dateien; dort werden auch Änderungen in einem Editor erkannt. Bei aktivierten Gemeinschaftswörterbüchern kannst du weitere Dateien mit dem Präfix aus der Sprachtabelle und einem dieser Namen ergänzen:

- `main.dic` für das Hauptwörterbuch, zum Beispiel `DEUmain.dic` oder `ENUmain.dic`.
- `Root.dic` für das Stammwörterbuch, zum Beispiel `DEURoot.dic`.
- `abbr.dic` für Abkürzungen, zum Beispiel `DEUabbr.dic`.

Die Dateien verwenden das native Dic-Format: Windows-1252, ein Eintrag pro Zeile und ein Tabulator zwischen Suchwort und Ersetzung. Main unterscheidet Groß- und Kleinschreibung; gewöhnliche Textersetzungen durchlaufen anschließend die normale Textanalyse. Stamm- und Ausspracheangaben werden an der jeweiligen Wörterbuchstelle verarbeitet.

Änderungen an eingebundenen Eloquence-Dateien und Dateien im Wörterbuchordner werden im Hintergrund erkannt. Das gilt auch für neu angelegte Dateien und für Editoren, die eine Datei beim Speichern ersetzen. Bei ungültigen Daten bleibt der letzte gültige Stand aktiv. Gemeinschaftswörterbuch-Updates aktualisieren die Dateien in diesem Add-on-Ordner. Eigene eingebundene Dateien und lokal bearbeitete oder entfernte Wörterbuchdateien werden dabei erhalten. Ein lokal bearbeitetes Volume hat Vorrang vor der heruntergeladenen Fassung.

Die erste Sprachausgabe wartet, bis die ausgewählten Wörterbücher vollständig geladen sind. Wenn beim Start kein gültiger Stand verfügbar ist, erscheint eine Fehlermeldung. Prüfe dann die Wörterbucheinstellungen oder wähle vorübergehend eine andere Sprachausgabe.

Unter **Add-on-Updates** prüft **Nach Add-on-Updates suchen** die veröffentlichten GitHub-Releases. Ist ein neueres Paket verfügbar, prüft **Update herunterladen und installieren** den Download und öffnet NVDAs normalen Installations- und Bestätigungsdialog. Standardmäßig wird zusätzlich beim NVDA-Start und danach regelmäßig automatisch geprüft. Bei einer neuen Version kannst du den Download und NVDAs Installationsdialog bestätigen. Die automatische Suche lässt sich hier ausschalten. Wenn noch kein Paket veröffentlicht wurde, zeigt der Bereich dies an. **Wörterbücher jetzt aktualisieren** bleibt davon unabhängig.

## Fehler melden

NVDA kann Symbole verarbeiten, bevor der Text die Sprachausgabe erreicht; seine Symbolaussprache beeinflusst daher, welche Zeichen bei ReLoq ankommen. Technische Details stehen im [Arbeitsstand](https://github.com/hozosch/Reloq-C/blob/main/docs/CURRENT-WORK.md).

Für eine [Fehlermeldung](https://github.com/hozosch/Reloq-C/issues) helfen ein kurzer Beispieltext, Sprache, Stimme, Version, Abtastrate und die Angabe, ob Wörterbücher eingeschaltet sind. Beschreibe auch, ob die Ausgabe abbricht, ein Wort falsch klingt oder NVDA selbst beendet wird.

## SAPI5 für x64 und x86

Die gemeinsame `setup.exe` installiert auf kompatiblem 64-Bit-Windows beide
Architekturen, auf 32-Bit-Windows die x86-Stimmen. Der Inno-Setup-Dialog bietet
die unterstützten Oberflächensprachen zur Auswahl und wählt zunächst die
Windows-Anzeigesprache. Nach der Installation bereits geöffnete
Sprachanwendungen neu starten und eine **ReLoq C**-Sprachstimme auswählen.
Updates erfolgen mit demselben Installer. Über Windows „Installierte Apps“
oder die Verknüpfung im Startmenü werden beide Architekturen deinstalliert;
eigene Wörterbücher und Benutzereinstellungen bleiben erhalten.

`ReLoqSettings.exe` oder die Stimmeigenschaften öffnen die sofort wirksamen
Sprachparameter. Das Kontrollfeld für gemeinsame Einstellungen übernimmt die
Parameter für alle Sprachen; beim Ausschalten kehren die getrennten Werte
zurück. Im Dialog „Equalizer“ stehen Tiefen, Mitten und Höhen sowie fünf frei einstellbare Filter zur Verfügung. Die Einstellungen folgen der gewählten Sprache oder der gemeinsamen Einstellung. Die Eloquence-Abtastrate gilt immer sprachübergreifend. SAPI oder die
Anwendung wandelt das Signal in ihre Ausgabe-Abtastrate um. Die angeforderte
Ausgaberate verändert weder Eloquences Syntheserate noch deren Klangprofil.
Standardmäßig steuert die SAPI-Anwendung Geschwindigkeit und Lautstärke. Im Dialog
„Anwendungswerte überschreiben“ lassen sich Geschwindigkeit, Tonhöhe und Lautstärke
für ReLoq C unabhängig davon festlegen. Ohne Überschreibung oder Geschwindigkeitsschub
entspricht SAPI 0 dem ECI-Wert 60, jeder Schritt zehn ECI-Einheiten. Alle Profile starten
bei Lautstärke 100. Weitere Hinweise stehen im [SAPI-Leitfaden](../sapi/README.txt).
