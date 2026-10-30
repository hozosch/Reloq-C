# Technischer Arbeitsstand: 0.2.198

## 0.2.198: Ganze Quellkennungen und messbare DSP-/Heap-Entlastung

Der Nutzer bestätigt, dass Play Protect die APK 0.2.197 akzeptiert und die deutsche Analyse besser ist. Der neue Dateiname und der Satz „SAPI5 berücksichtige ich bei der Vorbereitung des ersten Releases.“ zeigen dieselbe verbliebene Ursache: Das vorzeitige Trennen von Buchstaben und Ziffern verändert die native Akronym-/Zahlenanalyse und sogar die Silbengrenze im späteren Wort Vorbereitung. Isoliert wird Vorbereitung bereits richtig aus dem normalen Kern gelesen. Gemischte Kennungen gehen jetzt vollständig samt echten numerisch relevanten Satzzeichen in die native Analyse. Deutsche Buchstaben in Kennungen bleiben erhalten. Eine zusätzliche Lexikondatei oder ein Ausspracheeintrag für das Beispiel wird nicht eingeführt. 184 vollständige native Gruppen-/Akzentvergleiche und 1.664 Akzentvergleiche um variable Quellspannen bestehen; 160 öffentliche UTF-8-/Tag-/Fragment-/Indexprüfungen umfassen jetzt auch die neuen Beispiele.

Ein Linux-Samplingprofil bei 48 kHz weist auf Filterkorrektur und deren Phasenberechnung. Beide bekommen auf unterstützten nativen x64-Prozessoren einen AVX2-Zweig mit vier unabhängigen Double-Lanes. Die Reihenfolge jedes Summenpfads, Float-Rampen und die Horner-Rechnung bleiben erhalten; es gibt keine neue Näherung oder FMA-Zusammenziehung. 19.404 Filterzustandsvergleiche, 20.000 Phasenvergleiche und der ausdrücklich kompilierte bisherige Pfad bestehen bitgleich. Unter Windows auf ARM deaktiviert eine echte Hostarchitektur-Abfrage den neuen Zweig; native ARM64-Ziele behalten ihren bisherigen Rechenpfad. Feste 65-Tap-Fenster werden einmal wie die bisherigen Transformationskonstanten vorbereitet, log(1) entfällt. Die Pakete erhalten keine zusätzliche Laufzeitbibliothek.

Die vollständige Gegenüberstellung zu 0.2.197 hält alle 196 PCM-Ausgaben (14 Sprachen, sieben Abtastraten, zwei Modelle) bitgleich. Bei 48 kHz liegt der Median der CPU-Zeitverhältnisse auf dem AVX2-Linux-Host bei 0,9036; alle 14 Sprachen verbessern sich um etwa 6,4 bis 12,2 Prozent. Deutsch liegt bei 0,9355, US-Englisch bei 0,8937. Bei 11 kHz liegt der Median bei 0,9990, also praktisch unverändert. Gemessen werden Prozess-CPU-Zeiten, keine Reaktionszeit auf Dell/ARM/Android.

Der deutsche Quellpuffer wächst aus tatsächlichen Stützpunkten statt aus sechs maximalen Zielpuffern. Im 4.000-Wörter-Test sinkt die Spitze angeforderter Heap-Bytes von 100.477.682 auf 91.652.850 (minus 8.824.832 bzw. 8,78 Prozent), bei identischem PCM. Die große getrennte Zielpunktreservierung bleibt bestehen. Die Windows-DLL wächst für den zusätzlichen Code um 10.752 Bytes auf 55.223.296 Bytes; das Add-on wird 1.721 Bytes kleiner, die APK-Größe bleibt mit 17.708.438 Bytes gleich. Es wird kein Sprach-/Audioergebnis zwischengespeichert.

Vollständige ECI-Prüfung, 1.680 asiatische/griechische Ausgaben, 105 Pausenvergleiche, 366 CJK-Mischtextprüfungen, 35 JNI-/C-Paare sowie die NVDA-Verträge auf Deutsch und Englisch bestehen. Die APK aktualisiert 20197 → 20198 mit derselben Signatur, INTERNET bleibt die einzige Berechtigung. Der Browser-Updateweg bleibt bestehen. Die 46 geprüften eigenen deutschen Wortlautungen bleiben erhalten. Details: `docs/validation/0.2.198.json`.

Der neue englische `docs/RELEASE-PLAN.md` beschreibt überprüfbare Kriterien für das erste öffentliche Release und den nächsten SAPI5-Adapter. COM-Aktivierung, Fragment-/Steuerungsabbildung, PCM-Streaming und automatische Quellpositionsereignisse sind dort ausdrücklich noch als Implementierungsarbeit erfasst; SAPI5 ist in diesem Paket nicht enthalten.

## 0.2.197: Analysegrenzen, Regelaufrufe und Android-Updates

Weitere Fälle wurden ohne wortbezogene Korrekturen reproduziert: Zahlen nach Bindestrichen (etwa NVDA-2026) wurden als ausgesetzte Wortbestandteile behandelt; bei der Kontextrekonstruktion gingen echte Satzzeichen verloren oder wurden durch interne Akustikmarkierungen ersetzt. Die native Satzanalyse erhält nun zulässige ursprüngliche Trennzeichen einschließlich Anfang und Ende. Eingeklammerte Auslassungspunkte bleiben Prosodie. Die Referenz ist der bereits vorliegende vollständige native C-Regelkern; ein Apple-Originalmodul wurde hier nicht ausgeführt. Der vom Nutzer noch nicht konkret benannte Restfall ist damit nicht direkt geprüft.

Die Prüfung auf Regelprotokollierung und das dazugehörige Argument-Umpacken entfallen zur Übersetzungszeit aus den Release-Sprachaufrufen. Veränderlicher Diagnosezustand und die laufende Pflege des aktuellen Diagnoseregelzeigers entfallen ebenfalls. Entwicklungstracing bleibt mit `make -f Makefile.native BUILD=build/rule-diagnostics RULE_DIAGNOSTICS=1` in einem separaten Bauordner verfügbar. Ausspracheergebnisse werden weiterhin nicht zwischengespeichert.

Der Nutzer hat eine ausdrückliche Schädlich-Einstufung durch Play Protect für 0.2.196 bestätigt. Die neu eingeführte Android-Installationsberechtigung und der APK-Provider werden entfernt. Die Updateansicht öffnet auf Nutzeraktion einen geprüften GitHub-Downloadlink im Browser; Android übernimmt seine normale Installation. Ein ursächlicher Zusammenhang mit der Warnung oder eine Freigabe durch Google ist nicht belegt. Paketkennung und Signatur bleiben erhalten. Automatische Updateprüfungen gibt es nicht. Die Abhängigkeiten, tatsächlichen APK-Berechtigungen, Größen, Regressionstests und Grenzen stehen in `docs/validation/0.2.197.json` und den zugehörigen Detailberichten.

Die Windows-DLL sinkt von 55.982.592 auf 55.212.544 Bytes. Das Add-on spart 369.223 Bytes, die APK 503.808 Bytes; die ungestrippte ARM64-Bibliothek spart 1.601.416 Bytes. Die Binär-/Archivprüfung bestätigt ausschließlich Systemimporte und keine Regel-Tracing-Symbole. 196 PCM-Vergleiche über alle 14 Sprachen, sieben Abtastraten und zwei Modelle bleiben bitgleich. Die 132 vollständigen Gruppen-/Akzentvergleiche sowie 1.664 gezielte Akzentvergleiche rund um variable Quellspannen bestehen. 46 eigene deutsche Wortlautungen bleiben erhalten.

Der erste Linux-CPU-Vergleich liegt im Median bei 0,971, mit gemischten Einzelfällen. Nach Ende der übrigen Builds/Tests liegt der gezielte Nachtest für Spanisch bei 11 kHz bei 0,945, für Deutsch bei 48 kHz bei 0,996. Das belegt keinen wesentlichen Gewinn im DSP der hohen Raten. Der angeforderte Heap-Spitzenwert für 4.000 Wörter bleibt bei 100.477.682 Bytes. Die vollständigen ECI-Prüfungen, 1.680 asiatische/griechische Ausgaben, 105 Pausenvergleiche, 366 CJK-Mischtextprüfungen und die direkten APK-Upgradeprüfungen von 20192 und 20196 bestehen.

## 0.2.196: URLs, Lautpuffer und Paketupdates

Der Nutzer bestätigt die verbesserte deutsche Analyse aus 0.2.195. Beim neuen vollständigen Heise-Beispiel führte die URL bisher wieder zum Verlust der nativen Satzbetonung: Ein URL-Bindestrich wurde zusätzlich als ausgesetzter Wortbestandteil geführt. ASCII-URLs durchlaufen nun die vorhandenen nativen Regeln als vollständiges Quelltoken. Bei mehrgliedriger nativer Ausgabe wird interne Zeichensetzung bei der Rekonstruktion nicht erneut eingespeist. Die Unicode-Erweiterung behält ihre Zeichentabellen. 132 Gruppen-/Akzentvergleiche einschließlich des vollständigen Texts und weiterer URLs bestehen; die bisherigen 46 deutschen Wortlautungen bleiben unverändert.

Der deutsche Lautpuffer wird aus der validierten Lautzahl statt aus der Zahl aller Lautschriftbytes angelegt. Der bestehende Parser schreibt direkt in diesen privaten Puffer. Die zusätzliche Kopie zur atomaren Übergabe entfällt, während Abbruch und Fehler die Aufruferausgaben unverändert lassen. Im 4.000-Wörter-Vergleich gegen 0.2.195 sinkt die Spitze der angeforderten Heap-Bytes von 106.957.538 auf 100.477.682 (rund 6,06 Prozent). PCM bleibt bitgleich. Das sind erfasste Heap-Zuweisungen, keine RSS- oder Gerätemessung. Der große Akustik-Zielpuffer bleibt ein nächster Optimierungsansatz.

NVDA erhält in der Einstellungskategorie Eloquence C eine Paketupdate-Prüfung; Android erhält App-Updates. Beide prüfen veröffentlichte GitHub-Releases nur auf Anforderung, streamen Downloads und prüfen Größe/SHA-256. Die Installation geht an den jeweiligen normalen Installationsdialog. NVDA prüft Add-on-Name und Version, Android zusätzlich Paketkennung und Signatur. Der direkte signierte APK-Vergleich 20192 → 20196 besteht. Im Repository sind noch keine Download-Releases veröffentlicht; der neue Bereich meldet dies korrekt. Der eigentliche Installer-Dialog bleibt ein Geräte-/NVDA-Oberflächentest für die erste veröffentlichte Aktualisierung. Die Test-Pakete werden weiterhin unmittelbar bereitgestellt.

## 0.2.195: Kompakter Regelcode

Der Nutzer bestätigt bessere Satzanalyse und teilweise stärkere Übereinstimmung zum Original. Die DLL wurde durch größenorientiertes Übersetzen der Sprachregeln von 61.280.256 auf 55.981.568 Bytes verkleinert. Der DSP behält seine Optimierungseinstellungen. Die eigenen Klangabweichungen bleiben erhalten.

## 0.2.194: Einheitlicher Wörterbuchpfad

Deutsch nutzt regulär das eingebaute native C-Wörterbuch. Die zusätzliche Datei mit 16.500 Einträgen und 712.343 Bytes entfällt aus beiden Paketen und den regulären Treibern. Historische RLEX-Dateien bleiben Quell-Testdaten. Native Abkürzungsauflösung ersetzt die doppelten SPR-Tabellen. Haupt-/Stamm-Benutzerwörterbücher bleiben aktiv.

Für die Satzanalyse bleiben einfache Kardinalzahlen als ursprüngliche Ziffern erhalten, während die eigene Zahlenlautung weiter verwendet wird. Dadurch greifen native Betonung und Phrasengrenzen auch bei Mengen-/Zeitangaben. Die Analyse erhält die tatsächlichen Leerzeichen zwischen zugeordneten Quellwörtern; künstliche SPR-Formatierungsabstände werden dort nicht zurückgeführt. Die zugelassenen ch-Komposita lesen ihre Bestandteile bei Bedarf aus dem eingebauten Wörterbuch, ohne Ausspracheergebnisse aufzubewahren. Der eigene Akustikplaner bleibt erhalten.

Als Nutzerreferenz für spätere Geschwindigkeitsangleichung sind Deutsch 38 sowie Spanisch/Griechisch 18 in NVDA erfasst. Es gibt noch keine automatische sprachübergreifende Umrechnung. Die bisherige native Skala bleibt erhalten.

Die vollständige ECI-Prüfung, 227 gezielte Steuerungs-/Fragmentvergleiche, 35 JNI-/C-Paare, 866 Android-Einstellungs-/Datenprüfungen, beide NVDA-Oberflächen und die Wörterbuchprüfungen bestehen. Die Pakete sind um 147.620 bzw. 151.616 Bytes kleiner. Im kleinen Linux-Vergleich sinken Initialisierungszeit und Spitzen-RSS; die Mediane laufender Synthese/erster PCM liegen geringfügig höher. Ein allgemeiner CPU-Gewinn ist daraus nicht belegt. Der Hörtest auf Windows/Android für diesen Umbau steht aus. Hashes und Grenzen: `docs/validation/0.2.194.json`.

Die folgenden älteren Abschnitte beschreiben jeweils den damaligen Stand.

Die Projektbeschreibung und Bedienungsanleitungen stehen jetzt getrennt von diesen Entwicklungsnotizen. Ausgangspunkt für die Änderungen ist der unveränderte GitHub-Import von 0.2.186. Der eigene deutsche Akustikplaner, Chinesisch und die Entscheidungen in [GEWUENSCHTE-ABWEICHUNGEN-DE.md](../GEWUENSCHTE-ABWEICHUNGEN-DE.md) bleiben erhalten.

## ECI-Analyse, deutsche Zeichennamen und gemessene Sprechdauer

Der Nutzer bestätigt Android 0.2.192 einschließlich der sofortigen Einstellungen als stabil. Die weitere Arbeit betrifft den gemeinsamen Eloquence-Kern vor dem ersten öffentlichen Release; SAPI5 ist weiterhin geplant und noch nicht implementiert.

0.2.193 ergänzt zwei unabhängige Einstellungen in NVDA und Android: Abkürzungswörterbuch verwenden (weiterhin zunächst ein) und Satzvorhersage (weiterhin zunächst aus). Die öffentliche ECI-Anbindung übernimmt Parameter 3 mit der ECI-Polarität 0=ein/1=aus sowie die instanzbezogene Satzvorhersage unter Parameter 11. Die Einstellungen wirken über die vorhandene native Umgebung; ECI-Tags müssen dafür nicht eingeschaltet sein. Haupt- und Stammwörterbücher bleiben unabhängig aktiv. Die native Maschine wird bei Bedarf neu vorbereitet; die Wörterbuchdaten bleiben geladen.

Bei Deutsch übermittelte die native Lautschrift bisher veränderte Akzente, aber keine vorhergesagten Phrasengrenzen. Die vorhandene native Einfügeregel meldet jetzt ihren gewählten Übergang und dessen Typ an die laufende Analyse. Der SPR-Schreiber ordnet ihn dem folgenden Wort zu. Diese kurzlebigen Fakten werden im eigenen Planer verwendet; es gibt keine aus der Wortzahl erratenen Grenzen. Der feste Prüfsatz mit 25 Wörtern erhält genau die native Grenze vor Wort 12. Bei ausgeschalteter Satzvorhersage bleiben die bisherigen Pläne erhalten. Die zusätzliche Grenzliste wird nur bei aktiver Vorhersage angelegt und während der Analyse sortiert durchsucht; keine Sprachergebnisse werden gespeichert.

Grad-Zeichen und Unterstrich heißen direkt in der deutschen nativen Zeichenregel und der Unicode-Zeichentabelle Grad und Unterstrich. Die Ringel-Bezeichnung von Å/å bleibt erhalten. Es werden keine Bezeichnungen vor den Text gesetzt.

Die gleichen Geschwindigkeitswerte führen messbar zu verschiedenen Sprachdauern. Bei sechs nativen ma-Silben, Tempo 55, Melodie 0, 11.025 Hz und verkürzten Pausen ergeben sich 1,594 Sekunden auf Deutsch, 1,411 auf US-Englisch und 0,964 auf kastilischem Spanisch. Das sind Dauermodelle mit den eigenen Sprachregeln, keine CPU-Zeiten oder exakte WPM-Angleichung. Der wiederholbare Vergleich steht unter `tools/check_language_rates.py`, die vollständigen Werte unter `docs/validation/0.2.193-language-rates.json`.

189 gezielte native Vergleiche, 40 Sprachvergleiche, 366 CJK-Lateinvergleiche, 35 JNI-/C-PCM-Paare, 866 Android-Einstellungs-/Datenprüfungen, die Wörterbuchprüfungen und das vollständige ECI-Testziel bestehen. Darin sind 1.680 asiatische/griechische Renderfälle und 105 Pausenvergleiche enthalten. Beide NVDA-Oberflächensprachen und die 21 Windows-Exporte sind geprüft. Der signierte APK-Vergleich bestätigt dieselbe Paketkennung und dasselbe Zertifikat sowie 20192 → 20193. Die beiden neuen Android-Schalter brauchen noch ihren praktischen Hörtest. PCM bleibt ausschließlich im RAM.

Die gebündelten Wörterbücher stammen aus dem neu aufgelösten Master f93ccc82158386f7607ed3868473b6914121d2d7. Alle sieben Dateien stimmen mit ihren Git-Blobs überein. Chinesisch bleibt in der normalen Ausgabe unverändert; die 38 traditionellen Identitäten aus 0.2.192 gelten weiterhin. Der eigene deutsche Akustikplaner und die gewünschten Klangabweichungen bleiben erhalten.

Die folgenden Abschnitte beschreiben die vorigen Entwicklungsschritte. Frühere Angaben zu noch offenen Android-Tests sind historisch; der aktuelle bestätigte Plattformstand steht oben.

## Sofortige Android-Einstellungen und Schriftidentitäten

Der Nutzer bestätigt die Android-Oberfläche aus 0.2.191. In 0.2.192-preview4 übernehmen und speichern Checkboxen, Listen und Regler Änderungen sofort. Der Speichern-Button entfällt. Die nächste Syntheseanfrage liest die neuen Werte; eine bereits übergebene Äußerung behält ihren Parametersatz. Beim Laden oder Zurücksetzen eines Profils werden Zwischenstände nicht gespeichert.

Im griechischen Englischpfad bleiben Umlaute und weitere lateinische Akzente Teil des vollständigen Wortes. Der US-Analyzer erhält die vorhandenen CP1252-Identitäten; die schriftbezogene Vokalausrichtung nutzt dieselben Basiszeichen wie die gemeinsame Unicode-Tabelle. Die griechische Zeichennamen- und Prosodieverarbeitung wird dabei nicht ersetzt.

Ein isoliertes kleines g im chinesischen Textpfad nutzt jetzt dieselbe Alphabetidentität wie im Buchstabiermodus. Der bisher nur dort aktive Zugriff wird gemeinsam verwendet; g als Einheit nach einer Zahl und englische Fremdwörter behalten ihre Pfade.

Die chinesische Suche prüft zuerst die originale Han-Schreibweise. Bei einer vereinfachten Ersatzform bleiben die Lesungen von 38 Zeichen aus mehrdeutigen Vereinfachungsfamilien als Bedingungen erhalten. Die kleine C-Tabelle wird aus Unicode 17.0 Unihan erzeugt. Damit wird beispielsweise 髮 nicht mehr zu 發/fa1, und 隻 bleibt von 只 unterschieden. Mehrwortlesungen wie 頭髮 mit neutraler Endsilbe sowie alternative kontextuelle Lesungen wie 甲殼 bleiben gültig. Unverwandte regionale Ausspracheunterschiede werden nicht pauschal übernommen. Es gibt keine Laufzeit-Datenbank und keine vorgeschaltete Textersetzung.

147 Komponentenprüfungen mit dem früheren Romanizer und 246 tatsächliche PCM-/Indexvergleiche prüfen die neuen Fälle. 366 vorhandene CJK-Latein-/Unicodeprüfungen und 40 Sprachvergleiche bestehen weiterhin. Die Tests verwenden PCM ausschließlich im RAM. Die gebündelten Gemeinschaftswörterbücher wurden auf Master 8e4561171d7561477023896b06593485146b86d8 aktualisiert und gegen alle sieben Git-Blobs geprüft. Der Vergleich der signierten APKs bestätigt dieselbe Paketkennung und Signatur sowie versionCode 20191 → 20192.

Die neue Android-Bedienung benötigt noch den praktischen TalkBack-Test. Der deutsche Akustikplaner, die Stimmprofile und die DSP-Pfade bleiben erhalten.

## Android: Stimmparameter, Tempo, Wörterbücher und Updates

Der Nutzer bestätigt, dass die Installation und das verzögerte US-Reagieren mit 0.2.190 behoben sind. Die folgende Arbeit richtet sich auf die gewünschte Android-Einstellungsoberfläche und weitere kleine Einsparungen; der deutsche Akustikplaner und die übrigen Sprachregeln bleiben erhalten.

`0.2.191-preview3` enthält Sprache/Stimme, Schnellsprechmodus, Tonhöhe, Satzmelodie, Lautstärke, männliches/weibliches Modell, Kopfgröße, Rauheit, Behauchtheit, alle Abtastraten, Pausenmodi und ECI-Tags. Stimmwerte werden je Profil gespeichert. Geschwindigkeit und Tonhöhe haben getrennte Schalter „über Android steuern“; bei ausgeschaltetem Schalter gilt der eigene Regler. Sprache und Stimme können ebenfalls auf die gespeicherte Auswahl festgelegt werden.

Die Android-Anfrage liefert für Tempo und Tonhöhe relative Werte mit 100 als Normalwert. Der eigene Temporegler zeigt nominelle Wörter pro Minute an, zunächst 190. Die Geschwindigkeitskurve wird beim Build direkt aus der C-Dauertabelle erzeugt und invers ausgewertet. Normal entspricht nativ 55 statt des früheren 98. Drei unterschiedlich zusammengesetzte englische Texte ergeben zusammen rund 174 WPM bei normal und 353 WPM bei 2×; das Dauerverhältnis ist 2,03. Die tatsächliche Wortzahlrate hängt stark von Wortlänge, Sprache und Satzpausen ab. Tonhöhenänderungen skalieren die physische Grundfrequenz statt den nativen Parameter einfach proportional zu setzen.

Die Android-Wörterbuchverwaltung aktiviert die gebündelten Gemeinschaftsdaten, aktualisiert sie ausdrücklich aus aktuellem Master und bindet eigene Quelldateien über dauerhaften Dateizugriff ein. Quellen können sortiert, entfernt und neu geladen werden. Windows-1252-Bytes und native Main-/Root-/Abkürzungsstufen bleiben erhalten; Ersetzungen finden im C-Kern statt. Downloads, Zusammenführen und Änderungsprüfung laufen auf einem getrennten Worker. Ohne eigene Quellen läuft kein Dateimonitor; der Wörterbuch-Worker beendet sich nach 15 Sekunden ohne Arbeit. Unveränderliche Snapshots bleiben bis zum Ende ihrer aktiven Anfrage erhalten. Automatisches Erkennen von Editoränderungen benötigt Änderungsdatum oder Größe vom Android-Dateianbieter; manuelles Neuladen liest immer erneut.

Der JNI-Transport teilt weiterhin einen Kern je Sprache. Er setzt nur geänderte Steuerwerte und lädt Wörterbücher erst bei Benutzung der Sprache oder einem geänderten Snapshot. Die PCM-Puffer werden jetzt ebenfalls erst je verwendeter Sprache angelegt: anfangs 2.200 statt 30.800 Byte. Das deutsche Zusatzlexikon wird erst bei deutscher Ausgabe entpackt und anhand seines Inhalts benannt. Unveränderte 712.343 Byte können über App-Updates hinweg weiterverwendet werden. Alte ausschließlich generierte Versionskopien werden anschließend entfernt. Wiederholungscaches gibt es weiterhin nicht.

DEFLATE-Stufe 9 spart bei identischem Paketinhalt gegenüber Stufe 6 im NVDA-Paket 177.919 Byte. Der Android-Build nutzt dieselbe stärkere Kompression; wegen der neu enthaltenen optionalen Gemeinschaftswörterbücher wächst seine Gesamtgröße dennoch gegenüber der vorigen APK. Hieraus wird kein großer CPU-Gewinn abgeleitet.

Die fertigen signierten APKs werden zusätzlich auf Aktualisierbarkeit verglichen: Paketkennung `org.reloq.tts.preview`, gleiches Zertifikat und versionCode 20190 → 20191 sind bestätigt. Die APK aktualisiert damit die bisherige Vorschau und erhält deren Einstellungen. Eine Deinstallation ist nicht nötig.

Die Host-Prüfungen umfassen 865 Einstellungs-/Wörterbuchdatenprüfungen, 112 Stimmkennungen und Warteschlangensteuerung. 33 JNI-PCM-/Längenpaare stimmen mit direkten C-Aufrufen überein, einschließlich weiblicher Kopfgröße 22, 48 kHz, ECI-Tags und Wörterbuchwechseln. Die bisherigen 40 Sprachvergleiche, 25 Wörterbuch-Kontextprüfungen, 18 Retentionsprüfungen und zwölf Reload-/Abbruchzyklen mit Folgeausgabe bei 48 kHz bestehen weiter. Die neue Android-Oberfläche und Dateianbieterüberwachung wurden hier nicht auf einem echten Gerät ausgeführt. PCM bleibt ausschließlich im RAM.

Die folgenden Abschnitte dokumentieren die früheren Ursachen und Schritte; Angaben zu damals noch offenen Android-Tests sind historisch. Aktuelle Bedienung und Grenzen stehen in [android/README.md](../android/README.md).

## US-Wörterbuch nach Abbruch und Android-Paketierung

Der Nutzer konnte die erste Android-APK nicht installieren und meldet weiterhin gelegentlich langsame US-Reaktionen. Zwei konkrete Ursachen sind jetzt nachgewiesen und korrigiert.

Bei normal abgeschlossenen Äußerungen blieben die Wörterbuchtabellen bereits geladen. Ein abgebrochener interner Analysezustand wurde dagegen zusammen mit den Tabellen freigegeben. Die Folgeausgabe musste den vollständigen US-Satz erneut lesen und aufbauen. `release_handle()` trennt jetzt nur die Bezüge zur weggeworfenen Maschine und den Undo-Text. Die Tabellen und lexikalischen Daten bleiben Eigentum des Sprach-Workers. Vollständiges Freigeben erfolgt beim Schließen des Workers oder beim Aktivieren eines neuen Wörterbuchstands. Ein fehlerhaft geladener Satz wird weiterhin vollständig verworfen. Es werden keine Äußerungen oder PCM-Ergebnisse gespeichert.

`LanguageEngine` ignoriert zusätzliche Meldungen für denselben unveränderlichen Snapshot-Pfad. Geänderte Editor-Dateien und neue Community-Daten ergeben weiterhin einen neuen Snapshot und führen zur bestehenden transaktionalen Neuinitialisierung. Die große US-Tabelle wird damit beim regulären Laden aufgebaut und über Abbrüche hinweg im Speicher gehalten. Das erstmalige Laden hat weiterhin einen Aufwand.

Im lokalen Vergleich von sechs Abbruch-/Folgezyklen mit vollständigen US-Wörterbüchern sank der Median bis zum ersten PCM von rund 207 auf 24 Millisekunden. Die Folgeausgaben und Indizes sind identisch. Bei 48 kHz lag der neue Median im Test bei rund 26 Millisekunden. Die Zeiten sind Linux-Kernmessungen, keine Messung des konkreten NVDA-/Windows-Geräts. Die früheren Latenzspitzen werden damit gezielt reduziert; vollständige Verzögerungsfreiheit wird nicht behauptet.

Der neue Test `nvda/test_eci_dictionary_retention.py` entfernt eine testweise angelegte Wörterbuchdatei nach dem Laden. 18 Abbruch-, Callbackfehler-, Profil-, fehlgeschlagene Aktualisierungs-, neue Eintrags- und Abschaltprüfungen behalten die jeweils erwartete Aussprache. Ein unbemerkter erneuter Dateizugriff könnte dadurch nicht als Erfolg durchgehen. Die eingefrorene 0.2.189 scheitert bereits am ersten Abbruch. Die Änderungen am Provider wurden außerdem mit AddressSanitizer und instrumentiertem privatem Arena-Speicher geprüft; LeakSanitizer bleibt in dieser Entwicklungsumgebung abgeschaltet.

Die erste APK hatte eine komprimierte `resources.arsc`. Bei der erneuten ZIP-Paketierung gingen die von aapt2 gewählten Speicherverfahren verloren. Signatur und gewöhnliche Ausrichtungsprüfung hatten diese zusätzliche Installationsanforderung nicht abgedeckt. `0.2.190-preview2` behält die Verfahren bei und prüft die fertige Ressourcentabelle gesondert auf unkomprimierte Speicherung und Vier-Byte-Ausrichtung. `android/verify_apk.py` lehnt die tatsächliche alte APK ab und akzeptiert die neue. Paketkennung und Signaturzertifikat bleiben erhalten. Ein echter Pixel-/TalkBack-Test ist weiterhin offen.

Wiederholbare Messung ohne Audiodateien:

```sh
python3 tools/check_dictionary_cancel.py --cycles 6 --rate 1
python3 tools/check_dictionary_cancel.py --cycles 6 --rate 6
python3 nvda/test_eci_dictionary_retention.py
python3 android/verify_apk.py dist/reloq-c-android-arm64-0.2.190-preview2.apk
```

## Erhalten aus 0.2.189: Ressourcen, lateinische Akzente und Android

Der Nutzer bestätigt: Das Einfrieren aus dem vorigen Stand ist vereinzeltem langsamen Reagieren gewichen. Diese verbleibenden Verzögerungen werden nicht als vollständig behoben behauptet.

`LanguageEngine` hält jetzt einen nativen Kern je verwendeter Sprache statt je Sprach-/Stimmenpaar. Die acht Profile teilen sich denselben Sprachkern und Wörterbuchsatz. Auf dem bereits vorhandenen Kern werden bei der Aktivierung nur geänderte Profilparameter gesetzt. Es werden keine Sprach- oder PCM-Ergebnisse wiederverwendet. Ein lokaler Linux-Vergleich mit derselben eingefrorenen C-Bibliothek aus 0.2.188 und vollständigen US-Wörterbüchern prüft acht Stimmen und danach die umgekehrte Reihenfolge. Vorher: acht Handles, 110.060 KiB Spitzen-RSS, etwa 1,61 Sekunden bis zum Schließen. Jetzt: ein Handle, 38.876 KiB, etwa 0,32 Sekunden. Schließen: etwa 0,20 gegenüber 0,03 Sekunden. Alle 16 PCM-Längen und Hashes sind identisch. Diese Messung betrifft Stimmenwechsel und Verwaltung, nicht die Zeit jedes einzelnen gesprochenen Wortes oder echtes NVDA auf dem Dell. Wiederholbar mit `tools/check_voice_resources.py`; `--adapter` kann eine ältere Sprachverwaltung gegen dieselbe Bibliothek laden.

Die chinesische und koreanische Worterkennung lässt lateinische Akzentbuchstaben nun in ihren bestehenden englischen Fremdwortpfad. Die nativen Eingabealphabete beider Module und des US-Moduls enthalten bereits dieselben 207 Zeichenwerte. Windows-1252-Akzente bleiben deshalb native Zeichen; weitere lateinische Akzente nutzen die schon vorhandenen Unicode-Basisidentitäten. Eine kleine C-Tabelle wird mit `tools/update_cjk_latin_table.py` aus den gemeinsamen Zeichendaten erzeugt. Wortgrenzen, Zahlen, Bindestriche und die koreanische Lautgruppenbildung bleiben in der normalen Analyse. Die Prüfung umfasst 366 native Ausgabe-, Unicode-, Fragment- und Indexvergleiche, einschließlich gewöhnlichem Englisch, Pinyin, NFC/NFD und gemischten Schriften. Die japanischen Akzentfälle bleiben gegenüber 0.2.188 bytegleich.

Die erste Android-ARM64-Vorschau ist implementiert und als `0.2.189-preview1` gebaut. Sie enthält denselben C-Kern, alle 14 Sprachvarianten, acht Stimmen, sieben Abtastraten und die native Pausenverkürzung. Eine feste Worker-Thread-Zuordnung, maximal 16 PCM-Blöcke und ein auf die jeweilige Anfrage begrenzter Abbruch schützen die Ausgabe vor unbegrenztem Vorlauf und verspäteten Stop-Aufrufen. Nur Androids Synthesethread benutzt den Framework-Callback. Sprachtext wird als Standard-UTF-8 übergeben. Die ARM64-Rücksprunggrenze sichert auch das Android-Plattformregister sowie SIMD-/FP-Zustand. Der Android-Build erzwingt dieselbe vorzeichenbehaftete `char`-Semantik wie Windows/Linux.

Die JNI-Anbindung wurde auf dem Host mit dem tatsächlichen Kern geprüft: 26 PCM-Längen-/Hashpaare stimmen mit direkten C-Aufrufen überein. Dazu gehören sämtliche Sprachvarianten, alle US-Profile, Akzente, gemischtes UTF-8, Abbruch, ignorierte veraltete Abbruchanfragen und Folgeausgabe. Die tatsächlichen NDK-ARM64-Instruktionen haben den Register-/Stack-/Speichergrenztest in Unicorn bestanden. APK-Signatur, Manifest, DEX, ARM64-Abhängigkeiten und 16-KiB-Segmentausrichtung wurden geprüft. Installation, Android-Diensterkennung, TalkBack und kontinuierliche Ausgabe auf dem Pixel wurden hier noch nicht ausgeführt. Die optionale Wörterbuchverwaltung ist zunächst NVDA vorbehalten. Details stehen in [android/README.md](../android/README.md).

Die bisherigen 40 PCM-/Indexvergleiche der Sprachverwaltung, die 25 deutschen Wörterbuchprüfungen, die 18 Verwaltungsprüfungen und die NVDA-Paketprüfungen in deutscher und englischer Oberfläche bleiben erfolgreich. Die reale native Wörterbuch-Warteschlange wurde mit zwölf Reload-/Abbruchzyklen und einer Folgeausgabe bei 48 kHz geprüft; der Player ist dabei simuliert. Alle Prüfungen halten PCM ausschließlich im RAM.

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

Die frühere breit angelegte Corpus-Prüfung hat keinen Absturz nachgewiesen. Die lange Blockade wurde in 0.2.188 beseitigt; der Nutzer bestätigt inzwischen den Übergang vom Einfrieren zu vereinzeltem langsamem Reagieren. Beim Laden protokolliert der Treiber die tatsächliche C-Kernversion (`0.2.191.0`), DLL-Pfad und Sprache.

## Griechische Buchstaben in Finnisch und Koreanisch: geklärt

Der Nutzer hat den Fehler auf NVDA zurückgeführt: Dessen eigene Symbolverarbeitung übernimmt die Buchstaben, bevor sie den Synthesekern erreichen. Dieser Punkt erfordert keine Aussprachekorrektur im C-Kern. Die zusätzlichen Prüfungen des griechischen Alphabets und seiner Varianten zeigten in den nativen finnischen und koreanischen Satzkontexten ebenfalls keine ausgelassenen Namen. Die vorläufige griechische Routing-Diagnose wurde wieder entfernt.

## Weitere Plattformen und Ressourcen

NVDA bleibt die erste praktische Testplattform. Geringere Paketgröße, Speicherverbrauch und CPU-Last des gemeinsamen C-Kerns bleiben Ziele. Android hat jetzt einen ersten ARM64-Testbuild; praktische Gerätetests stehen aus. SAPI5 für Windows ist weiterhin geplant. Ausspracheentscheidungen und der eigene deutsche Akustikplaner bleiben erhalten.

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
