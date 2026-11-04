# Releasevorbereitung 1.1.1

- SAPI bereitet Sprache parallel zur Wiedergabe vor und reagiert schneller auf Unterbrechungen.
- Gedankenstriche im Text werden in den geprüften europäischen Sprachen als Pausenzeichen ausgewertet; bei Zahlenbereichen bleibt die bisherige Behandlung erhalten.
- Japanische und chinesische Wortmarkierungen unterbrechen die native Textanalyse nicht mehr und erzeugen keine zusätzlichen Pausen. Koreanisch verwendet denselben nicht eingreifenden Markierungsweg.
- Bei ausdrücklicher Zeichenlesung wird der deutsche Unterstrich als Nomen betont. Die Satzanalyse unterscheidet Nomen und Verb weiterhin nach Kontext.

# Releasevorbereitung 1.1.0

- Der bestätigte deutsche E-I-Gleitlaut verwendet Öffnung 85, Rücklage 45 und Weichheit 45. Die drei vorübergehenden NVDA-Regler sind entfernt; Einstellungen aus rc27 werden nicht mehr an den Sprachkern übertragen.
- Die Android-Version verwendet einen größeren Versionscode, damit auch 1.1.0-rc27 ohne Deinstallation aktualisiert werden kann. Der Update-Prüfer älterer rc-Versionen zeigt 1.1.0 möglicherweise nicht an; die APK kann einmalig direkt von GitHub installiert werden.
- Der Dateinamenfehler aus Issue #10 ist automatisch nachgestellt und korrigiert; die Bestätigung durch den ursprünglichen Melder nach Installation bleibt offen. Bei einem einzeln zitierten Wort können ECI-Positionsmarken weiterhin erst am Ende ausgegeben werden; das trat auch in rc26 auf.

# Testkandidat 1.1.0-rc27: Weichheit des deutschen E-I-Gleitlauts

- Der Gleitlaut in „drei“ verwendet im nativen Plan F3 2450 Hz, in „Homepage“ waren es bei den mittleren Reglern 2850 Hz. Ein neuer Weichheitsregler senkt F3 stufenweise um bis zu 400 Hz, unabhängig von Öffnung und F2. Er beginnt bei 50; mit 0 lässt sich der Klang von rc26 wiederherstellen. Ein Hörvergleich ist weiterhin nötig.
- Die englische Morphologie darf bei einem bereits abgetrennten Präfix nicht den einzigen verbleibenden Vokal als weiteres Präfix verbrauchen: „recosp“, „uncosp“ und „noncosp“ behalten so den vollständigen Stamm. Eine T-Regel gibt einen alleinigen Schlussvokal wieder an die Lautplanung zurück, statt ihn mit einer Affrikate zu verschlucken, wie bei „tzsche“ und „ttzsche“. Ein Hörtest bleibt nötig.
- Bei vorbereitenden Vergleichen mit 3013 englischen Wörtern mit möglichen Präfixen und 3016 Wörtern mit T änderte sich nur die Lautschrift der jeweils betroffenen Testformen. Das ist keine vollständige Sprachabdeckung.
- Deutsche Bindestrichfälle: Bei „dane-ben“ und „dage-gen“ darf die da-Partikelregel nicht auf einen Bestandteil angewendet werden, dessen eigene Morphologie keinen da-Präfix enthält. So entsteht derselbe Betonungsablauf wie bei vergleichbaren Bindestrichwörtern. Ein vorläufiger Vergleich von 153 da-Zusammensetzungen änderte nur diese beiden Fälle.
- Die Analyse darf einen inneren Schnitt nicht hinter das Ende des aktuellen Bestandteils setzen. Meldet die anschließende Zusammensetzungsanalyse Erfolg, ohne für den Bestandteil ein Morphem anzulegen, greift die gewöhnliche Stammbehandlung. Bei „audio-encoder“, „video-encoder“ und „macro-encoder“ werden dadurch beide Bestandteile mit derselben Betonungsstruktur wie bei „radio-encoder“ analysiert. Ein vorläufiger Vergleich von 1.667 Bindestrichwörtern änderte nur acht betroffene Formen. Klang und Betonung benötigen noch einen Hörvergleich.
- PR 11 für den nativen Windows-Build ist gemergt und in den Teststand übernommen.

# Testkandidat 1.1.0-rc26: Vokallose Silben und einstellbarer Gleitlaut

- Die frühere Aussage für rc25 wird korrigiert: „tzsche“ erzeugte dort nur praktisch stumme Samples, und bei „recosp“ fehlte der S-Rauschanteil. Der Test hatte lediglich auf Daten ungleich Null geprüft. Ursache war die Einordnung der gesamten vokallosen Silbe als Vokalkern. Der gültige Tonhöhen-Anker liegt nun hinter der Silbe, damit die ursprüngliche Konsonantenplanung alle Laute behandelt. US- und britisches Englisch sowie der englische Pfad in Mandarin werden auf hörbaren Pegel und S-Rauschenergie geprüft; ein Hörtest bleibt notwendig.
- Zwei NVDA-Regler im Einstellungsring erlauben die unabhängige Änderung der Öffnung und der Lage des deutschen E-I-Gleitlauts. 50/50 ist die rc25-Ausgangsstellung; andere Werte ändern nur diesen Kontext. Per-Instanz-ECI-Einstellungen werden während einer Synthese threadlokal gesetzt und anschließend wiederhergestellt. Deutsche Standardsignale wurden mit rc25 für beide Modelle und alle sieben Raten verglichen.
- Für Issue #10 gibt es eine [Nachstellanleitung](ISSUE-10-REPRO.de.md) und ein Skript für lange Dateien. Ein Windows-Stresstest stellte den gemeldeten AddText-Fehler nach; zuvor schlug innerhalb von `eciSynthesize` ein Zugriff auf einen bereits freigegebenen Audioformat-Eintrag fehl. Ein erzwungener Konverterfehler reproduziert denselben Absturz. Die Formatverwaltung entfernt den Zeiger jetzt sofort und beendet die fehlgeschlagene Einrichtung, bevor erneut darauf zugegriffen wird. Tests erzwingen den Fehler und prüfen die Weiterarbeit zweier Handles; zusätzlich folgen 1.280 schnelle Dateinamenwechsel bei 11 und 48 kHz, Deutsch und US-Englisch. Die Bestätigung unter Windows steht noch aus.
- Die rc25-Windows-DLL umfasst 54.848.512 Bytes. Ihr Code und die konstanten Sprachtabellen belegen den überwiegenden Teil; die alte deutsche Planerimplementierung wird in den produktiven Sprachpfad nicht mehr eingebunden. Ein experimenteller verlustfreier Blockkompressor für die lokalen Unicode-Namen spart nur etwa 1,37 MB Rohdaten und wurde wegen zusätzlicher Laufzeitkomplexität nicht für diesen Hörkandidaten übernommen. Ein deutlich kleineres Binärpaket ohne Funktionsverzicht ist bisher nicht nachgewiesen.

# Testkandidat 1.1.0-rc25: Stumme und unvollständige Fälle

- Die gemeinsame native Zeitplanung behandelt englische Silben ohne Vokalanker, unter anderem bei „tzsche“ und „recosp“, auch im chinesisch-englischen Pfad. Der nachfolgende Text wird mitgeprüft.
- Französisches „quil“ erhält wie „kil“ einen Vokal und ein konsonantisches L; „quille“ behält seinen Gleitlaut. Die Regel gilt für beide französischen Varianten.
- Spanische Beträge wie „€123 456.78“ lesen alle Tausendergruppen und Nachkommastellen. Sehr lange Zahlen mit „ª“ werden auch innerhalb eines Satzes wiedergegeben. Prüfungen vergleichen die vollständige Lautfolge mit äquivalenten Schreibweisen.
- Die deutschen Anpassungen aus rc24 bleiben enthalten. Die Färbung von „Homepage“ und die S-Anregung bei „sm“/„sn“ benötigen weiterhin einen NVDA-Hörvergleich. Issue #10 zu schneller Dateinavigation bleibt offen, weil der gemeldete Fehler bisher nicht reproduziert wurde. Auch die verspäteten Wortmarken bei chinesisch-englischem und koreanischem Text sind weiter offen.

# Testkandidat 1.1.0-rc24: Deutscher Gleitlaut und stille Wörter

- Der J-Gleitlaut in „Homepage“ geht vom unveränderten nativen Eloquence-Pfad aus und wird behutsam nach hinten verschoben. Ein spektraler Vergleich mit der proprietären Original-DLL stützt den Ausgangspunkt; die neue Färbung bleibt ein Hörkandidat. Messbedingungen: [Originalvergleich](REFERENCE-ELOQUENCE-GLIDE.md).
- Unbetontes E vor vokalischem R, wie in „Kann er“, verwendet die Formantfärbung des früheren ReLoq-Planers; die Regel berücksichtigt Text und Lautschrift.
- Bei wortanfänglichem „sm“ und „sn“ soll das S wieder die Anregung des früheren ReLoq-Planers verwenden; Timing und Übergang bleiben im nativen Pfad. Die Klangwirkung benötigt einen Hörtest.
- Die nativen Zeitmarken- und Bereichsprüfungen lassen mehrere bislang stumme Wörter sprechen, darunter „dane-ben“, „dage-gen“, „audio-encoder“, „video-encoder“, „macro-encoder“ und englisches „bad'hhesiron“. Regressionstests prüfen auch die Sprachausgabe nach diesen Wörtern.
- Die Untersuchung der weiteren Fehlermuster aus Eloquence 64 ist noch nicht abgeschlossen: Französisches „quil.“, englisches „tzsche“, chinesisch-englisches „recosp“ und spanische Beträge wie „€123 456.78“ bleiben in diesen Sprachvarianten stumm oder unvollständig. Sie sind noch kein Release-fähiger Stand. Issue #10 zu schneller Dateinavigation ist bislang nicht reproduziert.

# Testkandidat 1.1.0-rc23: Zahlwörter und weitere deutsche Aussprache

- Bei 42 und 43 erhält der Übergang von „zwei“ beziehungsweise „drei“ zu „und“ wieder die Zeitpunkte des alten Planers; die verbindende Aussprache ohne Glottisschlag bleibt erhalten. Vor „achtzig“ wird der Glottisschlag in den zusammengesetzten Achtzigern wieder gesetzt.
- Der J-Gleitlaut in „Homepage“ geht vom Ei-Gleitlaut aus und wird leicht in Richtung langes I verschoben. Die Klangwirkung benötigt einen Hörtest.
- Die unbetonte Silbe in „veröffentlicht“ und seinen Beugungsformen entspricht „veröffentlichen“. Das unabhängige Wort „samt“ erhält stimmhaftes S; ausdrückliche Wörterbucheinträge haben weiterhin Vorrang.
- Nutzerwörterbucheinträge in Bindestrichzusammensetzungen werden im nativen Pfad berücksichtigt. Der alte Satz- und Akustikplaner ist aus dem ausgelieferten ECI-Kern entfernt; sein Quelltext bleibt für Entwicklervergleiche erhalten.

# Testkandidat 1.1.0-rc22: Deutsches R, finales A und Zahlübergänge

- Die konsonantische R-Anregung folgt den unterschiedlichen Kontexten des alten Planers, unter anderem bei „herunter“ und an Wortgrenzen; „vierund“ behält den kontinuierlichen R-Verlauf. Die Wirkung benötigt einen Hörtest.
- Das finale Text-A nach R wird auch bei „Kamera“ erkannt. Der J-Gleitlaut in „Homepage“ liegt als Hörtest zwischen dem Original und RC21.
- Der Er-Laut nach langem Ö, wie in „höher“, erhält die Formanten des alten Planers. Bei vokalischem R nach langem Ö wird das Ende der gemeinsamen Stimmkurve behutsam abgesenkt.
- „zweiund“ und „dreiund“ werden mit den alten Übergängen verglichen; Zahlwörter bleiben ohne den zuvor unerwünschten Glottisschlag. Lautschrift und Texteingabe sollen dieselbe Verbindung verwenden.
- Der alte deutsche Satz- und Akustikplaner bleibt erhalten, bis die gewünschten Fälle vollständig übertragen und im Hörtest bestätigt sind. Android und SAPI5 werden vom Nutzer erst nach der deutschen Aussprache getestet.

# Testkandidat 1.1.0-rc21: J-Gleitlaut und 32-Bit-Wortmarken

- Der J-Gleitlaut nach E in derselben Silbe, wie in „Homepage“, verwendet die Formantziele des Ei-Lautes in „drei“. Text und entsprechende Lautschrift werden zusammen geprüft; der Klangeindruck bleibt im Hörtest zu beurteilen.
- Eine falsche Aufrufkonvention bei 32-Bit-Wortmarken wurde korrigiert. Die plattformübergreifende Validierung läuft.
- Bei Bindestrichzusammensetzungen bleibt die ursprüngliche Schreibweise für die Satzplanung erhalten, wenn die native Aussprache den Nutzerwörterbucheintrag bereits enthält. Muss ein Bestandteil nachträglich ersetzt werden und besteht die Zusammensetzung aus mehreren gesprochenen Teilen, wird nur dieser Bestandteil ersetzt. So bleiben in den geprüften Fällen der Eintrag und der Satzakzent erhalten, ohne den Bindestrich vorzulesen. Weitere gemeldete Fälle können zur genauen Nachstellung noch ein konkretes Beispiel erfordern.

# Testkandidat 1.1.0-rc20: R-Anregung, US-Fragen und Wörterbuchquelle

- Konsonantisches R verwendet wieder die zum Kontext passende Anregung des alten deutschen Pfades; vokalisches R bleibt unverändert.
- US-amerikanische Fragen behalten eine ansteigende Endkontur, auch bei hoher Sprechgeschwindigkeit; null Betonung bleibt flach.
- NVDA und Android bieten die alternative IBM-TTS-Wörterbuchquelle zusätzlich zur bisherigen Quelle an; eigene Einträge behalten Vorrang. Der erste Bezug erfolgt im Hintergrund.
- SAPI5 bietet dieselbe Quelle mit Hintergrund-Download. Ein eigener SAPI-Wörterbuchordner hat Vorrang vor der ausgewählten Quelle.
- Die alternative Quelle enthält derzeit US-englische und kastilisch-spanische Dateien, keine deutschen. Das ist ein unveröffentlichter Hörtestkandidat; es erfolgt keine automatische Veröffentlichung.
- Der zusätzliche SAPI-/ECI-Vergleich meldet bei chinesisch-englischem Mischtext mit Wortmarken abweichendes Audio. Derselbe Befund besteht bereits mit RC19; die übrigen 13 verglichenen Sprachvarianten stimmten überein. Die Marke wird dort teilweise erst am Textende gemeldet. Das bleibt als eigener Fehler offen.

# Testkandidat 1.1.0-rc19: Deutsches R, finales A, Fragekontur und Wörterbuch

Die bestätigte Anregung am Wortanfang wird für konsonantisches R auch in
anderen Wortpositionen verwendet. Das vokalische R behält seine Behandlung.
Für finales A wird der Formantübergang an den früheren Planer angeglichen,
unter anderem bei „Komma“. Die bewährte Auswahl am Satzende bleibt bestehen.
Fragen verwenden einen tiefen Satzakzent und den Schlussanstieg des früheren
deutschen Planers. Die Klangwirkung dieser Änderungen ist im NVDA-Hörtest zu
beurteilen.

Vollständige Bindestricheinträge im Stammwörterbuch haben vor Bestandteilen
Vorrang. Ausspracheeinträge bleiben auch bei ausgeschaltetem
Abkürzungswörterbuch und bei `da0` aktiv. Der Test durch den gepackten
NVDA-Treiber verwendet eine ausgewählte eigene Dic-Datei.

# Testkandidat 1.1.0-rc18: Deutsches R, finales A, Zahlwort, Wörterbuch und Satzmelodie

Die Anregung des konsonantischen R wird mit dem früheren deutschen Planer
abgeglichen: Das anlautende R erhält dessen mehrteilige Schwingung; „vierund“
verwendet den dortigen kontinuierlichen Übergang. Am Satzende erhält das
unbetonte Text-A die frühere Formantmischung, auch vor einem folgenden Satz;
vor einem folgenden Wort bleibt die kurze Variante. Das N vor „und“ in
„neunund“ wird an den früheren Zeitwert angeglichen.

Wenn die native Analyse einen Wörterbucheintrag in einem Bindestrichwort nicht
in der vollständigen Aussprache berücksichtigt, wird dieses einzelne
Wort mit den vorhandenen Zusammensetzungsregeln aufgelöst. Der umgebende Satz
bleibt in der nativen Analyse. Geklammerte Bindestrichwörter werden bei
passenden typografischen Anführungszeichen wie bei geraden Anführungszeichen
analysiert. Bei schwach betonten Verben nach dem Satzakzent entfällt in
Aussagesätzen eine zusätzliche Anhebung der Satzmelodie. Verschiedene deutsche
Fragetypen erhalten am Ende wieder eine steigende Kontur, auch vor passenden
schließenden typografischen Anführungszeichen. Die Wirkung von R,
A und Satzmelodie muss im NVDA-Hörvergleich beurteilt werden.

# Testkandidat 1.1.0-rc17: R, N, A und deutsche Tonhöhe

Die in rc16 ergänzten zusätzlichen Formantziele für konsonantisches R sind
zurückgenommen; seine Schwingungsquelle wird für den Hörvergleich weicher
abgestimmt, einschließlich des R in „vierund“. Der N-Anschluss in „siebenund“
wird verkürzt. Die E-Formanten nach I werden an den früheren Pfad angenähert,
unter anderem bei „Barriere“ und „wie eben“.

Das unbetonte finale Text-A verwendet eine Mischung aus den nativen kurzen
und den früheren langen Formantzielen. Sie gilt auch vor einem folgenden Satz
nach Punkt, Frage- oder Ausrufezeichen; vor dem nächsten Wort im selben Satz
bleibt die kurze Textvariante erhalten. Explizite Lautschrift behält ihre
Vorgaben. Die Wirksamkeit bei Sonderfällen und die subjektive Klangwirkung
müssen weiter im Hörvergleich geprüft werden.

Die deutsche F0-Zielberechnung verwendet die breiten Zwischenrechnungen des
englischen Pfades. Zuvor konnten bei hoher Tonhöhe und großer Satzmelodie
Zwischenwerte überlaufen und die Kontur einbrechen lassen. Die Korrektur und
die neuen Lautziele werden direkt an der nativen Planung geprüft.

# Testkandidat 1.1.0-rc16: Deutsche Zahl- und Lautübergänge

Der native deutsche Pfad liest die Zahlbestandteile „zweiund“, „dreiund“,
„vierund“, „siebenund“ und „neunund“ ohne den zusätzlichen Glottisschlag;
bei „vierund“ wird das R als Konsonant geformt. Die gewählte Behandlung bei
„fünfund“, „sechsund“ und „achtund“ bleibt erhalten. Der Hörvergleich ist
noch offen.

Die Analyse korrigiert „Sprachauswahl“ und „Suchauswahl“ einschließlich ihrer
geprüften Plurale. Eigene Main- und RLEX-Einträge behalten in den geprüften
Fällen Vorrang. Lange O-Laute in geschlossenen -os-Silben behalten ihre Länge;
„famos“ und „rigoros“ erhalten die Betonung des alten Pfades. Für erfasste
Hauptwörter mit „Über-“ und „Unter-“ wird die bisherige Großschreibungs- und
Betonungsregel wieder angewandt. Die Auswahl dieser Wortstämme deckt nicht
notwendigerweise alle denkbaren Zusammensetzungen ab.

Die native Lautplanung übernimmt weitere Formantziele für konsonantisches R,
passt /pf/ vor einer folgenden Konsonantensilbe und /s/ nach /n/ vor /f/ an und
verwendet für normales unbetontes A am Ende einer Äußerung den bisherigen
helleren Vokalansatz. Die endgültige Klangwirkung kann nur ein Hörvergleich
beurteilen. Der C-Klang und die Hochratenfilter wurden nicht neu abgestimmt.
Ein ausdrücklich als kurzes A vorgegebener Lautschrifteintrag behält seine
eigene Vorgabe; die End-A-Vorliebe gilt für normalen Text.

# Testkandidat 1.1.0-rc15: Deutsche Lautplanung und Zahlen

Die native deutsche Satz- und Lautplanung bleibt der Produktpfad. Die
französische Nasalfärbung in „Chance“ und „Restaurant“, die R-Quelle, der
R-Übergang nach langem ö und das reduzierte R nach langem ü oder o verwenden
gezielte Werte aus dem vorherigen Komponisten. Ab 22 kHz erhält der deutsche
labiodentale Reibelaut /f/ beziehungsweise /v/ eine Quellenanhebung; die
C-Hochratenfilter bleiben unverändert. Die subjektive Färbung und der F-Pegel
müssen im Hörvergleich geprüft werden.

Die native Analyse korrigiert die Betonung und den Vokal im Wortstamm
„konzentrier“. Uhrzeiten einschließlich Sekunden sowie Tausendergruppen mit
mehreren oder geschützten Leerzeichen werden auf die native Zahlenanalyse
abgebildet, mit Quellenabbildung für Indizes und Steuerungen. Bei expliziter
deutscher Lautschrift wird die Vokallänge anhand der nativen Silbenstruktur
an die Textanalyse angeglichen; kurze Vokale bleiben kurz. Automatische
Tests vergleichen unter anderem Text mit erzeugter Lautschrift, Wörterbuch-
Vorrang, Indizes und Kontrollwörter. Die Ergebnisse gelten für die geprüften
Fälle und ersetzen keinen Hörtest.

Eine breitere Stichprobe zeigt weiterhin Unterschiede zwischen Text und daraus
erzeugter Lautschrift, unter anderem bei „obenan“ und „Ruin“. Der native
Textpfad besitzt Wort- und Glottalgrenzen, die in der ausgegebenen Lautschrift
nicht immer gleichermaßen erhalten sind. Die Gleichheit ist daher für die
geprüften Beispiele verbessert, aber nicht für alle Wörter nachgewiesen.

# Testkandidat 1.1.0-rc14: Gemeinsame deutsche Satzanalyse

Deutsch nutzt standardmäßig die native Satzanalyse und Lautplanung wie die
anderen Sprachen. Der bisherige deutsche Komponist ist ausschließlich über
`RELOQ_DE_LEGACY=1` für Diagnosen erreichbar; eine fehlgeschlagene native
Synthese wechselt nicht automatisch auf ihn. Der gemeinsame Pfad erhält die
Nutzerwörterbücher einschließlich groß-/kleinschreibungssensitiver RLEX-Einträge
und zusammengesetzter Wörter. Geldbeträge in gleichwertiger EUR-, Euro- und
Eurozeichen-Schreibweise werden an die native Währungsregel übergeben.
Pausenverkürzung und explizite Pausen bleiben getrennt.

Der native Wortleser trennt sehr lange Einzelwörter nach 100 Buchstaben. Damit
explizite RLEX-Ausspracheeinträge bis zur zulässigen Länge von 127 Zeichen
erhalten bleiben, werden nur solche Einträge vor dieser Trennung an den
nativen Satzpfad übergeben. Separat geladene RLEX-Einträge bleiben wie bisher
auch dann aktiv, wenn der ECI-Wörterbuchschalter ausgeschaltet ist.

Die bestätigte 2-dB-Absenkung der männlichen Stimme wird an den AF-Zielwerten
für wortanfängliches /ts/ und /tsv/ angewandt. Weibliche Stimmen und interne
/ts/ ohne folgendes /v/ bleiben dabei unverändert. Stimmwechsel-Tags wählen den
Wert an der tatsächlichen Wortgrenze. Die C-Hochratenfilter und die
kastilische/griechische S-/T-Begrenzung wurden nicht neu abgestimmt.

Für „der Zahn“ stammt der Verlauf des vokalischen R aus der nativen
Lautplanung. Im bisherigen Signalvergleich liegen die Energieverhältnisse bei
1,4 und 1,6 kHz in mehreren Messfenstern näher an der originalen ECI-DLL als
beim alten Komponisten. Das belegt keine passende subjektive Färbung: Der
Hörvergleich in NVDA steht noch aus. Gleiches gilt für die übrige deutsche
Aussprache und Satzmelodie nach dem Architekturwechsel.

# Testkandidat 1.1.0-rc13: Fester C-Klang und deutsche Übergänge

Die A-/B-/C-Auswahl im NVDA-Treiber ist entfernt; die bestätigte Abstimmung C
gilt auch für ECI-Programme, die noch einen früheren Profilwert setzen. Die
acht Stimmprofile bleiben verfügbar. Frequenzabhängige Gewichte der
Hochratenfilter werden jetzt einmal pro Abtastrate berechnet. Vergleiche mit
rc12 zeigen identische PCM- und Indexdaten für 236 Fälle über alle Sprachen,
Modelle und Raten. Zwei ruhige Linux-Prozessmessungen mit deutschen, englischen
und griechischen Sätzen über 16 bis 48 kHz ergaben im Median ungefähr 3 %
weniger CPU-Zeit; einzelne Fälle lagen innerhalb der Messschwankung oder waren
langsamer. Das ist keine Windows-Messung.

Die deutsche Satzanalyse übernimmt gemischte Schrägstrich-/Bindestrichwörter
wie „A/B-Test“ als zusammengehörigen Ausdruck. Dadurch fällt der umgebende
Satz in diesen Fällen nicht wegen unterschiedlicher Wortgruppen auf die alte
Analyse zurück. 173 Vergleichsfälle mit der nativen Satzanalyse bestehen,
einschließlich der Bindestrich-/Schrägstrichkombinationen; Main-Wörterbücher
und explizite RLEX-Ausspracheeinträge bleiben in den geprüften Fällen wirksam.

Bei „der Zahn“ und ähnlichen Wortübergängen trägt der Vokal das stimmhafte R
etwas weiter in den Verschluss des folgenden Z. In der Signalmessung bleibt der
stimmhafte Anteil damit länger erhalten und nähert sich dem Verlauf der
originalen ECI-DLL an. Dauer und Indizes bleiben gleich. Andere getestete
Kontexte wie „der Mann“, „der, Zahn“, „Herz“ und „Zehn“ bleiben bitgleich.
Das ist ein technischer Vergleich; die Hörprüfung des neuen Übergangs steht aus.

Die vollständige deutsche Umstellung auf denselben nativen Ausgabepfad wie
Englisch ist noch nicht die Produktvorgabe. Der getrennte Versuchszweig zeigt
kürzere Startzeiten bei langen Einzelsätzen, aber noch Abweichungen bei
gemischt geschriebenen RLEX-Einträgen, Euro-Schreibweisen und der bereits
bestätigten männlichen Z-Lautstärke. Lange deutsche Einzelsätze und offene
Zitate behalten daher ihre bisherige Startverzögerung.

## Testkandidat 1.1.0-rc12: C als Vorgabe, griechischer Filter und CPU

Der bestätigte C-Klang ist jetzt die Vorgabe in der ECI-DLL und im NVDA-Treiber;
gespeicherte B-Vorgaben sowie die frühere C-Auswahl wechseln auf C. A und B
bleiben wählbar. Griechisch erhält dieselbe Begrenzung der zusätzlichen
S-/T-Bandbreite wie Kastilisch. Bei 16 kHz stimmen sechs griechische
Vergleichsausgaben mit rc11 überein; oberhalb von 16 kHz liegt das zusätzliche
S-Spektrum im geprüften Bereich.

Die Hochratenfilter nutzen dieselben unveränderlichen Frequenzrotationen und
Quellfilterantworten wieder, statt sie für jeden veränderten Laut erneut zu
berechnen; zwei identische Durchgangsstufen im C-Stimmfilter entfallen. 236
PCM-/Index-Vergleiche über alle Sprachen, zwei Modelle und die auswählbaren
Profile waren identisch. Im warmen Linux-Prozessvergleich sank die CPU-Zeit
der getesteten deutschen, englischen und griechischen Sätze bei 16 bis 48 kHz
um etwa 18–28 %. Das ist keine Messung auf einem Windows- oder Android-Gerät.
Ein Frequenzrastertest vergleicht alle Werte bitweise mit der direkten
Berechnung; ein separater Test initialisiert das Raster gleichzeitig aus zwölf
Threads. Ergebnisse und Vergleichscode stehen unter `validation` und `tools`.

Die Umstellung auf den gemeinsamen deutschen Satzpfad ist ausdrücklich noch
nicht die Vorgabe. Mit `RELOQ_DE_NATIVE_TRIAL=1` läuft ein Diagnoseversuch
über denselben Pfad wie Englisch. Lange Einzelsätze beginnen im lokalen
NVDA-Treibertest wesentlich früher; Tags, Zeichenmodus, weiche Trennstriche
und URLs haben eigene Vergleichstests bestanden. Es fehlen weiterhin der
Wörterbuchabgleich in zusammengesetzten Wörtern und die Übernahme der bereits
bestätigten männlichen Z-Absenkung: Bei „Zehn“ und „Zwei“ ist der Anfang im
Versuch etwa 2 dB lauter als in rc11. Die Korrektur des vokalischen r vor /ts/
ist ebenfalls noch nicht nachgewiesen. Deshalb behält rc12 den bisherigen
deutschen Produktpfad; der Diagnosepfad ist kein fertiger Ersatz.

Ein separater Linux-Stresstest mit acht gleichzeitig sprechenden ECI-Handles
löste im gemeinsamen Sprach-Frontend einen Prozessabsturz aus. Derselbe Test
bricht auch mit der unveränderten rc11-Bibliothek ab; der reine Filterrastertest mit
zwölf Threads besteht dagegen. Mehrere gleichzeitig synthetisierende Handles
sind daher weiterhin ein offener Fehler, unabhängig von dieser Optimierung.

## Testkandidat 1.1.0-rc11: Sanfte Anreicherung als Vergleich C

Der Nutzer bestätigt den A-Stimmweg von rc10 als sauber, hört aber keine
stufenweise Erweiterung bei höheren Raten. B bleibt deshalb genau die
rc10-Basis. Das neue, ausdrücklich auswählbare C formt diese Stimme ab
22,05 kHz mit einer festen, breiten periodischen Glocke bei 3,75 kHz.
Die zusätzliche Anhebung an der Mitte beträgt 0,45 / 0,8 / 1,1 / 1,25 dB
bei 22,05 / 32 / 44,1 / 48 kHz. Das Filter bleibt linear und kausal und
wird beim Start über 5 ms eingeblendet. Es verändert bestehende Energie,
erzeugt aber keine zweite Stimme und verändert weder den Glottispuls noch
Aspiration oder AF. 16 kHz und die niedrigen Raten bleiben gleich B.

Das ist von DECtalks Richness-Ziel inspiriert, aber keine Umsetzung von
DECtalks ri-Parameter: [Dessen Öffnungsphase](https://dectalk.github.io/dectalk/idh_sdk_3_nopen_fixed_nf.htm)
bleibt hier unangetastet. Eine Filterung vorhandener Obertöne kann keine
zusätzlichen harmonischen Frequenzen erzeugen. Die subjektive Wahrnehmung,
auch das Verhältnis zwischen Stimmkörper und Helligkeit, bleibt offen.

Im Modellvergleich liegen 36 Vokal-/Tonhöhen-/Ratenfälle im Band 3,5–5 kHz
um 0,428 bis 1,231 dB über B und bei 5–8 kHz um 0,247 bis 0,860 dB;
der Bass weicht um höchstens 0,042 dB ab. Die 36 benachbarten Ratenstufen
haben in diesem Vergleich im Band 3,5–5 kHz mindestens 0,087 dB Zuwachs;
das ist keine Aussage über die Hörbarkeit oder alle Sprachlaute.
28 PCM-Vergleiche halten reine AF- und AH-Laute und die 16-kHz-Stimme
zwischen B und C gleich. Dazu bleiben 80 tatsächliche S-, TS-, F- und
T-Anfänge bei beiden Profilen bytegleich. In 48 verglichenen Sprach- und
Stimmeinstellungen bleibt die leisere Ausgabe für B und C linear; bei voller
Lautstärke überschreiten 9 B- und 10 C-Fälle die Schutzschwelle. Die Messungen
belegen nicht, dass alle Einstellungen hörbar verzerrungsfrei sind. Der
Hörtest in NVDA ist notwendig.
Android bleibt auf seinem bisherigen Paketstand, und die lange deutsche
Einzelsatzanalyse bleibt offen.

Die folgenden Abschnitte beschreiben frühere Teststände.

# Testkandidat 1.1.0-rc10: A-Stimmweg mit Classic-Konsonanten

Der Nutzer hört bei B auch nach rc9 scheppernde Artefakte, bei A jedoch nicht.
rc9 mischte keine zweite Obertonquelle bei; B veränderte aber Glottis und
Vokaltrakt im Klatt-Kern und formte die periodische Energie zusätzlich per EQ.
Ab 16 kHz verwendet B nun den vollständigen A-Weg für Stimme und Aspiration,
einschließlich der bisherigen A-Präsenzformung. Die Classic-Konsonanten bleiben
auf einem getrennten Rauschpfad. Der B-spezifische Stimmkörper-EQ ist nicht
aktiv. Die Ausgangsregelung und der Spitzenschutz von B bleiben bestehen.

In 300 zeitlich veränderlichen Frames über fünf Raten und zwei Referenzmodelle
sind Stimme und Aspiration vor dem Mischen mit dem Rauschen zwischen A und B
bitgleich. 45 stationäre Vokal-/Tonhöhen-/Ratenfälle stimmen dort ebenfalls mit
A überein. Das ist keine Aussage über die Gleichheit der fertigen PCM-Ausgabe
oder über den Höreindruck. Die natürliche A-Abstimmung liefert nicht für jeden
Vokal einen monoton mit der Rate ansteigenden Hochtonanteil. Ob sie den
gewünschten Stimmkörper ohne das Scheppern liefert, bleibt ein Hörtest.

Bei 32, 44,1 und 48 kHz wird S-/TS-artiges Reiben insgesamt um 1,35 dB
abgesenkt; die Form des Konsonantenspektrums bleibt erhalten. Gegenüber rc9
sind das bei 32 kHz zusätzliche 0,75 dB und bei 44,1/48 kHz 1,35 dB.
In je zwei Stimmen geprüfte S-Anfänge im US-Englischen, kastilischen und
lateinamerikanischen Spanisch sowie deutsche Z- und ZW-Anfänge bleiben bei
16 und 22 kHz bytegleich und sinken bei den höheren Raten um die genannten
Werte. Die geprüften englischen und deutschen F-Anfänge bleiben bytegleich.
Die subjektive Lautstärke und weitere Lautumgebungen müssen gehört werden.

Android bleibt auf seinem bisherigen Paketstand, bis die Klangabstimmung
bestätigt ist. Der lange deutsche Einzelsatz bleibt weiterhin offen.

Die folgenden Abschnitte beschreiben frühere Teststände.

# Testkandidat 1.1.0-rc9: Stimmkörper-EQ und Konsonantenpegel

Dieser NVDA-Hörtest setzt bei den vom Nutzer auch bei 16 kHz gehörten
Artefakten an. Der bisherige stimmhafte Hochton-Shelf, der bis zur
Nyquist-Grenze anhebt, wird ab 16 kHz durch eine feste, breite und begrenzte
Anhebung um 3,75 kHz ersetzt. Sie beträgt dort 1,3 dB bei 16 kHz und wächst
bei 22,05/32/44,1/48 kHz auf 1,66/2,02/2,26/2,38 dB. Dies formt vorhandene
periodische Energie, ohne zusätzliche Obertöne zu erzeugen. Der unabhängig
behandelte Rauschpfad der Konsonanten bei 16/22 kHz und die Aspiration
bleiben unverändert.
Bei 32 kHz wird der gesamte AF-Weg um 0,25 dB abgesenkt, bei S-artigen
Lauten und kastilischem S/T um 0,6 dB; seine relative Spektralform bleibt gleich.

Ein Vergleich gegen rc8 erhält 20 untersuchte S-/F-Anfänge bei 16 und
22 kHz bytegenau. Bei 32 kHz sind diese Anfänge je nach Laut um etwa
0,25 beziehungsweise 0,6 dB leiser; andere untersuchte Raten bleiben
gleich. In 36 stationären Vokal-/Tonhöhen-/Ratenfällen weicht der Bass
um höchstens 0,096 dB von der 16-kHz-Basis ab; die Energie im Vergleichsband
3,5–5 kHz steigt in jedem Schritt, mit maximal 2,58 dB gegenüber 16 kHz.
Diese Messungen sagen nicht voraus, ob der Klang subjektiv artefaktfrei ist.

Die Änderungen sind ein Hörtest, keine bestätigte Beseitigung der Artefakte.
Android bleibt auf seinem bisherigen Paketstand, bis die Klangabstimmung
bestätigt ist. Der lange deutsche Einzelsatz bleibt weiterhin offen.

Die folgenden Abschnitte beschreiben frühere Teststände.

# Testkandidat 1.1.0-rc8: Konsonanten und Android-Direktstart

16 kHz bleibt unverändert. Ab 22,05 kHz werden die breiteren
Konsonanten aus rc6 mit nach unten ausgeglichener Gesamtleistung verwendet.
Kastilisches S/T behält die Formung aus rc7 mit etwas schwächerer Erweiterung.
Der zusätzliche stimmhafte Anteil nutzt keine bewegten oberen Resonanzen
mehr: Ein fester, breiter Filter formt nur bereits vorhandene periodische
Energie schrittweise und zurückhaltend. Aspiration bleibt unverändert.
Der Klangeindruck und mögliche Artefakte müssen im Hörtest beurteilt werden.

Im lokalen Vergleich realer S-Anfänge liegt die 6,5–7,5-kHz-Formung bei
US-Englisch und lateinamerikanischem Spanisch wieder nahe rc6; beim
kastilischen S liegt sie in diesen Beispielen etwa 1–1,5 dB unter rc7.
40 Modellfälle prüfen die abgesenkte Gesamtleistung. 60 Vergleiche mit rc6
bestätigen A und 8/11/16 kHz bytegenau. Diese Messungen ersetzen keinen
Hörtest von laufender Sprache.

Der Android-Sprachdienst ist für Direct Boot registriert. Vor dem ersten
Entsperren werden Spracheinstellungen und Kopien der aktivierten Wörterbücher
aus dem gerätegeschützten Speicher verwendet. Die Originaldateien und ihre
Pfade bleiben im nutzergeschützten Speicher. Änderungen oder das Entfernen
aktualisieren die Direktstart-Kopie. Nach dem Entsperren verwendet der nächste
Sprachauftrag die normalen Wörterbücher und Einstellungen. Eine
Paketaktualisierung oder das Öffnen der Einstellungen übernimmt vorhandene
Stimmeinstellungen und Wörterbücher in die benötigten Kopien. Es gibt keine
zusätzlichen Android-Berechtigungen. Ein Test auf einem echten Gerät vor dem
ersten Entsperren steht aus; eine Play-Protect-Entscheidung lässt sich aus den
Paketprüfungen nicht garantieren.

Ein Hosttest durchläuft den Dienst vor und nach dem Entsperren mit gesperrtem
Nutzerspeicher sowie Austausch und Entfernen der aktiven Wörterbücher. Der
native Sprachkern liest dabei einen Eintrag aus der kopierten Direktstart-Datei;
die Android-Geräteintegration ist damit noch nicht geprüft.

Die lange Startzeit bei deutschen Einzelsätzen bleibt offen.

Die folgenden Abschnitte beschreiben den vorherigen Teststand.

# Testkandidat 1.1.0-rc7: Gestufter Stimmkörper und begrenzte S-Erweiterung

Die vollständige Classic-Abstimmung von 16 kHz bleibt auch bei höheren
Raten die Grundlage. Die zusätzliche periodische Färbung verwendet breite,
schwach gewichtete Versionen der oberen A-Resonanzen. Ihre Stärke steigt
ab 22,05/32/44,1/48 kHz mit 0,15/0,30/0,42/0,46; jede zusätzliche Resonanz
erhält höchstens das Dreifache dieses Werts in dB. Koeffizienten werden über
5 ms überführt. Die Quelle bleibt einzeln und periodisch; Aspiration und
AF erhalten diese Resonanzen nicht. Die Glottis übernimmt oberhalb von
16 kHz die stabile physikalische A-Zuordnung ohne die frühere Extrapolation.
Es gibt keine zusätzliche Sättigung oder zweite Stimme.

Das vollständige S-Hauptband bleibt am Classic-Ziel verankert. Nur die
optionale Erweiterung oberhalb von 5,5125 kHz erhält einen sanften Abfall
um 8,5 kHz und ein begrenztes Leistungsbudget. Das Budget steigt mit den
Raten auf höchstens 8/14/18/20 Prozent der Referenzleistung bis 4,6 kHz.
Kastilisches S/T erhält weiterhin nur ein Viertel der Erweiterung. Andere
Konsonanten behalten ihre bisherige Übertragung. Die Unterscheidung
zwischen Hauptresonanzen und schwächerem oberen Zusatz ist eigenständig
umgesetzt; TGSpeechBox diente als öffentlich einsehbare Vergleichsreferenz.

36 stationäre Vokal-/Tonhöhen-/Ratenfälle behalten den Bass innerhalb
0,056 dB der 16-kHz-Basis, zeigen in jedem Schritt mehr Energie im oberen
Vergleichsband (kleinster Schritt 0,030 dB, maximale Anhebung 2,394 dB)
und erreichen mindestens 55,78 dB harmonischen zu nicht-harmonischem Anteil.
180 reale Sprach-/Modell-/Ratenfälle bleiben unter der PCM-Grenze (Maximum
31.411); 60 Vergleiche mit rc6 erhalten A und 8/11/16 kHz bytegenau.
Das sind Messgrenzen für diese Fälle, keine Hörabnahme laufender Sprache.

Ein interner bedingter Trennstrich U+00AD verändert in rc6 beispielsweise
„Verschlusslaut“ und „Wortanalyse“. Er wird jetzt vor Main-Wörterbuch und
Wortanalyse entfernt. Quellindizes werden über eine Skalarabbildung erhalten;
ausdrückliche Buchstabierung und SPR bleiben unverändert. 452 Prüfungen
vergleichen PCM, Indizes, geladene Main-Wörterbücher und explizite
Buchstabierung. 48 bewegte Resonanzfolgen bleiben bei wechselnden
Blockgrenzen bytegleich; ihr Gleichspannungsgewinn bleibt eins.

Die Verzögerung bei langen deutschen Einzelsätzen ist weiter offen. Eine
separate Diagnosefassung misst bei rund 9.000 Zeichen etwa 130 ms bis zum
ersten nativen Phonemblock und rund 170 ms bis zum ersten PCM-Puffer.
Der überwiegende Teil entsteht vor der Audioausgabe in der nativen deutschen
Analyse. Englisch liefert im selben Hostvergleich nach etwa 25 ms PCM.
Ein kleinerer Audiopuffer allein löst diese Abhängigkeit nicht. Vollständige
deutsche Sätze lassen sich wie bisher nach wenigen Millisekunden beginnen.
Beliebiges Aufteilen eines Einzelsatzes würde seine Grammatik-/Akzentanalyse
verändern und wird durch diesen Teststand nicht eingeführt. Weitere konkrete
Wortanalysefehler brauchen eigene reproduzierbare Beispiele.

Die folgenden Abschnitte beschreiben historische Teststände.

# Testkandidat 1.1.0-rc6: NVDA-Transport und tatsächliche Wortgrenzen

Die bisherigen DLL-Tests und der NVDA-Vertragstest waren getrennt: der
Vertragstest verwendete eine Ersatz-Engine. Ein neuer Test führt die
unveränderten NVDA-2026.3-Methoden für Sprache, Symbole und Befehlsverarbeitung
mit dem verpackten Treiber, echter ECI-Engine und einem Audioplayer ohne Gerät
zusammen. Er ersetzt weiterhin NVDAs Oberfläche, Prioritätsplanung und das
Audiogerät; er ist kein Hörtest auf Windows ARM.

NVDA entfernt das Gravis-Zeichen oder spricht seinen Symbolnamen, bevor Text
den Synthesizertreiber erreicht. Ein nur für den aktiven ReLoq-Treiber und
die eingeschaltete Tag-Einstellung registrierter Sprachfilter transportiert
gültige Tags als Synthesizerbefehle. Gewöhnlicher Text behält NVDAs Wörterbuch-,
Symbol- und Unicode-Verarbeitung. Ein Tag zwischen Buchstaben wird vor das
ungeteilte westliche/griechische Wort gestellt. CJK behält seine Segmentierung.
NVDA-Geschwindigkeit und -Lautstärke sind zusätzlich zur Tonhöhe angebunden;
temporäre Werte werden auch nach Abbruch oder Fehlern zurückgesetzt.

Die native Meldung `synthesizingWord` läuft der Audioausgabe voraus. Mehrere
schwache Wörter können vor demselben Audioblock gemeldet werden. Dadurch
konnte eine spätere Rücksetzung den vorherigen Parameterwechsel auslöschen.
Steuerungen werden nun über private native Indizes mit akustischen Zeitmarken
geplant und erst beim entsprechenden Audioframe angewendet. Öffentliche
Wortgrenzen-Indizes und Tonhöhenmarken verwenden denselben Zeitbezug;
Innenmarken und SPR-Gruppen werden nicht geteilt. Die öffentlichen ECI-Exporte
werden dabei nicht erweitert oder umbenannt.

16 kHz bleibt bitgleich zu rc5. Ab 22,05 kHz verwendet Profil B den tatsächlichen
periodischen A-Trakt einschließlich seiner sechs, sieben bzw. acht Formanten.
Kontinuierliche Filterzustände, bandbegrenzter Glottisschluss und geglättete
Filterwechsel ersetzen die feste 20/22-dB-Anhebung aus rc5. Es bleibt eine
Stimmquelle. Die unabhängige Classic-Konsonantenbank und die auf ein Viertel
begrenzte zusätzliche kastilische S/T-Bandbreite bleiben erhalten. 36 stationäre
Vokal-/Tonhöhen-/Ratenfälle liegen in drei Spektralbändern innerhalb 0,701 dB
des A-Körpers; der Bass weicht höchstens 0,687 dB von 16 kHz ab. Das beweist
keine subjektiv passende Klangfarbe oder Verzerrungsfreiheit bei laufender Sprache.

110 kausale Tag-/Pitch-Reihenfolgetests und 725 Queue-/Wortkontrolltests
bestehen. 48 Prüfungen gegen einen ungeschützten Float-Diagnosebuild bestätigen
die lineare leise Ausgabe. Beim kleinen weiblichen Modell mit hoher Behauchung
liegen einzelne rohe Spitzen weiter über der PCM-Grenze; der vorhandene
Ausgangsschutz bleibt deshalb aktiv. Begrenzte fertige PCM-Spitzen allein
gelten nicht als Nachweis eines unverzerrten Quellsignals.

Die lange deutsche Einzelsatzanalyse ist weiterhin offen. Der durchgehende
Hosttest mit rund 9.000 Zeichen benötigt ungefähr 0,2 Sekunden bis zum ersten
Playerpuffer, gegenüber rund 0,015 Sekunden bei US-Englisch. Normale abgeschlossene
deutsche Sätze starten im selben Test deutlich früher. Diese Zeiten enthalten
die NVDA-Textverarbeitung und den Treiber, aber kein echtes Audiogerät. Das
Paket protokolliert Quellrevision, geladene DLL und bestätigte Raten-/Profil-/
Tag-Einstellungen sowie bei langen Texten erste native PCM-Ausgabe und erste
WavePlayer-Übergabe. Damit kann die Verzögerung im echten NVDA eingegrenzt werden.

Die folgenden Abschnitte beschreiben historische Teststände.

# Testkandidat 1.1.0-rc2: Classic-Basis und kastilische Bandbreite

Die 16-kHz-Abstimmung bleibt die Grundlage von Profil B. Ab 22,05 kHz
wird nur die obere Differenz des mit 0,8 kalibrierten A-Stimmkörpers
beigemischt: ein komplementärer Split bei 2,5 kHz erhält den unteren
Classic-Anteil. Die Gewichte steigen weiterhin mit 25/50/80/100 Prozent
bei 22,05/32/44,1/48 kHz. Auch bei 48 kHz bleibt der Classic-Körper aktiv;
der bisherige vollständige Wechsel zu A entfällt. Der Split führt keine
zusätzliche Ausgabeverzögerung ein und behält seinen Zustand über Frames.

Kastilisches S/T verwendet wieder die normalen Classic-Presence-Konsonanten.
Die Sprachmarkierung begrenzt ausschließlich die optionale obere AF-
Erweiterung. Bei 16 kHz hat sie keine Wirkung; darüber bleibt für diese Laute
das vollständige Classic-Zielspektrum ohne zusätzliches oberes Rauschband.
Variante A, Mexikanisch und andere Konsonanten behalten ihren normalen Pfad.
Die Erkennung mit A4F/A5F über A3F und ihr Halten über den AF-Ausklang bleibt
erhalten. Die A-/OpenEVV-Filterausnahme aus rc1 ist damit abgelöst.

Der direkte Tag-Test umfasst nun auch ein Leerzeichen nach `vv50`,
unveränderte PCM-Ausgabe vor dem folgenden Wort und identische Indexpositionen
in elf westlichen Sprachen/Dialekten, einschließlich „Das ist ein `vv50 test“.
Dafür war keine weitere Änderung am Tag-Parser erforderlich. Die vollständige
ECI-Parität bleibt ein separates, noch nicht abgeschlossenes Ziel.

Die folgenden Abschnitte beschreiben historische Teststände.

# Testkandidat 1.1.0-rc1: A-Stimmkörper, Wortparameter und Startzeit

Ab 22,05 kHz mischt Profil B den seriellen AV/AH-Stimmkörper aus Variante A
mit 25/50/80/100 Prozent bei 22,05/32/44,1/48 kHz bei. A wird dabei fest mit
0,8 kalibriert, damit die Summe mit den Classic-Konsonanten auch beim kleinen
weiblichen Modell unter der PCM-Grenze bleibt. Die frühere zusätzliche
Glottisneigung entfällt. Die 16-kHz-Stimmkontur bleibt unverändert. Bei
48 kHz wird nur der benötigte A-Körper berechnet; langsam bewegte A-Filter
werden wie Classic höchstens alle 2 ms neu angepasst und über 3 ms überführt.
Es gibt keine dynamische Lautstärkeregelung und kein PCM-Resampling.

Kastilisches S/T verwendet im bisherigen S/Z/T-Spektralbereich die vollständige
A-Rauschbank und deren Filter. Die Auswahl ist unabhängig von der bisherigen
S-Mischung: Das reale spanische S/T liegt mit A4F-A3F = 10 dB genau an der
alten, zuvor ausgeschlossenen Grenze. Der gewählte Filter bleibt im Ausklang
aktiv. Andere Classic-Konsonanten bleiben erhalten.
Die isolierte AF-Prüfung stimmt dabei über alle fünf erweiterten Raten exakt
mit A überein. Zusätzlich stimmen 20 reale S-/T-Anlaute über beide Modelle
und fünf Raten mit A überein. 30 spanische Pegelfälle bleiben unter der PCM-
Grenze (Maximum 32.275); 60 A-/Mexiko-Kontrollen bleiben unverändert. Der
Hörtest muss die passende Höhenbalance bestätigen.

`vv` wird bei den westlichen Sprachmodulen vor der Lexikalanalyse entfernt
und über die nativen Wortgrenzen angewendet. So bleibt das Wort zusammen
und sein gesamter Klang bekommt denselben Pegel. CJK verwendet den eigenen
nativen akustischen Wortplan mit dem ursprünglichen ECI-Lautstärkewert;
redundantes `vv` verändert seine Segmentierung nicht mehr. Deutsche
Stimmsteuerungen innerhalb eines Wortes werden vor diesem Wort zugelassen.
Zusätzliche Abschlusspausen bei Stimmparametern und die deutsche künstliche
50-ms-Pause entfallen. Nichtdeutsche Inline-Tonhöhe nutzt ohne zusätzliche
ECI-Pitch-Marken den bestehenden kontinuierlichen F0-Skalierungsansatz.

Alle acht nativen Stimmparameter werden beim AddText-Aufruf für die PCM-
Synthese festgehalten. Spätere Setter gelten ab dem folgenden Textteil;
Setter nach dem letzten Textteil gelten erst für die nächste Äußerung.
Aufrufer-Annotationen werden vor internen Parameterwechseln zugelassen,
auch bei ausgeschalteten öffentlichen Tags. Textteile bleiben eine Äußerung.

Lange gewöhnliche deutsche Prosa wird zunächst in begrenzten Analysefenstern
zugelassen. Nur tatsächlich erkannte vollständige Satzgrenzen mit
abgeschlossenen Zitaten/Klammern werden ausgegeben. Abkürzungen, Zahlen und
abhängige Satzenden behalten den vorhandenen Ganztextpfad. Tags, besondere
Pitch-Marken und eigene Rlex-Ersetzungen nutzen weiterhin den sicheren
bisherigen Plan. 40 Vergleiche liefern identische vollständige PCM- und
Indexausgabe. Im Hosttest sank die Zeit bis zum ersten PCM-Puffer bei
9.472 deutschen Zeichen von median 14,69 auf 3,58 ms (11,025 kHz, fünf Läufe).
Das ist keine Messung der NVDA-Audiogerätelatenz auf Windows ARM.

447 neue ECI-Queue-/Wort-/Tag-Prüfungen und die bisherigen 364 öffentlichen
API-Prüfungen bestehen. 136 reale Pegelfälle bleiben unter der PCM-Grenze
(Maximum 32.107), 36 A-/Niedrigraten-Vergleiche bleiben bitgleich zu dev10.
293 Profilprüfungen, 371 deutsche Kontextfälle, 32 Abbruchfälle und 72
Sekundenlesungsfälle bestehen ebenfalls. NVDA-Pakettests bestehen in beiden
UI-Sprachen; die Windows-x64-DLL hat unverändert 49 öffentliche Exporte.
Die vollständige ECI-Parität, die Klangbalance und ruckelfreie Wiedergabe auf
dem Zielgerät sind damit nicht behauptet. Details: `validation/1.1.0-rc1.json`.

Die Lizenztexte aus dem Entwicklungszweig werden mit dem zugehörigen
Quellverweis in die Pakete übernommen. Vor der stabilen 1.1.0 stehen der
Hörtest zu #1 und das Zusammenführen der Lizenz-/Backend-Änderungen nach
main an. #2/#5/#6 sind anhand der bestandenen Regressionen geschlossen;
#4 ist mit dem Lizenzstand vorbereitet. Es wird noch kein stabiles Release
veröffentlicht.

# Teststand 1.0.2-dev10: ECI-Phoneme und stimmhafte Quelle

`eciGeneratePhonemes` liefert native Phonembytes über den regulären
ECI-Callback, mit begrenztem Aufruferpuffer, Indizes, Wiederholung und Abbruch.
Die PCM-Route bleibt danach erhalten. `eciPause` hält die Callback-Ausgabe
an; Resume oder Stop beendet die Pause. Beide Funktionen gelten für alle
DLL-Clients. Synthese bleibt synchron; vollständige ECI-Parität ist offen.

Die Classic-Stimmquelle berücksichtigt Sample-Bruchteile beim
Glottisschluss. Perioden, Quellphasen und AH-/AF-Masken bleiben unverändert.
Damit wird ein samplegerundeter Sprung an der Quelle geglättet, bevor er den
Vokaltrakt anregt. Das ist eine Anti-Alias-Korrektur und kein Ausgangslimiter.
Die bisherigen Konturen bei 16 kHz bleiben bestehen; die Quellkorrektur gilt
auch dort. Ab 22,05 kHz öffnet eine zunehmend flachere native Glottisneigung
die periodischen Obertöne. Der bisherige 4,5-kHz-Shelf entfällt. AH und die
akzeptierte AF-Konsonantenformung bekommen diese Anhebung nicht.

Langsame Traktbewegungen passen den aktuellen Classic-Filter höchstens alle
2 ms neu an, große Sprünge sofort. Der Übergang des Restfilters dauert 3 ms.
Das spart Spektralanpassungen und entschärft ihre Wechsel. Es gibt keinen
Sprach- oder PCM-Ergebniscache. Ob Knistern, Stimmkörper und Ruckler im
alltäglichen NVDA-Betrieb ausreichend verbessert sind, bleibt ein Hörtest.

364 öffentliche ECI-API-Prüfungen und der vollständige native C-Test bestehen.
45 Quellmodellfälle reduzieren den Fehler zur überabgetasteten Referenz um
mindestens 12,8 dB. 136 reale Pegelfälle bleiben unter der PCM-Grenze; der
AF-Pfad ist in 60 Sprach-/Modell-/Tonhöhen-/Ratenfällen bitgleich zu dev9.
Die beiden DSP-Replays benötigen rund 29 bzw. 33 Prozent weniger Host-CPU.
Details, Paket-Hashes und Messgrenzen stehen in `docs/validation/1.0.2-dev10.json`.

# Teststand 1.0.2-dev9: allgemeiner ECI-Abgleich

Der Schwerpunkt liegt auf dem öffentlichen DLL-Verhalten. Die Klangabstimmung
und die Filter aus dev8 bleiben erhalten; das vom Nutzer weiterhin gehörte
Knistern und die vereinzelten Ruckler sind damit noch nicht gelöst.

13 weitere Original-Exporte ergänzen Fehlerabfragen und Wörterbuchbearbeitung.
Callback-Antwort 0 wiederholt denselben Puffer oder Index; 2 bricht ab.
Der Index wird entsprechend der Referenz erst nach dem Callback veröffentlicht.
Standardfehler akkumulieren unabhängig von der privaten Synthesediagnose.
Der ECI-Reset hängt nicht mehr von zuvor benutzten privaten Parametern ab.
Das gilt für jeden Client. SDK-Namen im öffentlichen Header sind mit C11 und
C++17 geprüft. Details, Referenzbeobachtungen und Grenzen beschreibt
ECI-COMPATIBILITY.md.

295 öffentliche ECI-API-Prüfungen, der vollständige native ECI-C-Test und die
x64-Exportprüfung mit 47 Funktionen bestehen. 56 PCM-/Indexvergleiche mit
dev8 bleiben über Deutsch/US-Englisch, beide Modelle, alle sieben Raten und
Profil A/B bitgleich. Paket-Hashes und Prüfgrenzen stehen in
`docs/validation/1.0.2-dev9.json`; die Klangprobleme sind weiterhin offen.

# Teststand 1.0.2-dev8: deutscher Satzkontext und periodisches Richness

Eloquence_64 fügt vor Textfragmenten wiederholt `pp0`/`da1` und vor
Satzzeichen Pausenbefehle ein. Bisher konnte ein einzelnes `p0` den deutschen
Ganzsatzabgleich ausschalten. Das führte unbemerkt zur isolierten
Wortanalyse. Ein Leerzeichen vor dem eingefügten Befehl trennte außerdem
Ordinale vom Punkt; wiederholte Präfixe konnten zwischen Ordinal und Monat
eine falsche Doppel-Leerraumgrenze erzeugen. Analysebefehle ohne
Zustandsaenderung und akustische Pausen werden jetzt vor der lexikalischen
Analyse normalisiert. Positive Pausen werden danach ueber dieselbe monotone
Quellabbildung wie ECI-Indizes wieder eingesetzt. SPR und echte
Zustandswechsel bleiben erhalten. Auch „Ein- / Ausgabe“ wird nicht mehr
als falsches Auslassungskompositum zerlegt und behält den Satzkontext.

Der Vergleich verwendet den tatsächlichen EciEngine-/Textbuilder-Code aus
fastfinge/eloquence_64, Commit b24a6c88446581e50867abac23807233829b367f,
mit Loader-/Callback-Aliassen unter Linux und CP1252 für den deutschen
MBCS-Korpus. Ganze Saetze und wortweise Fragmente liefern dort die gleiche
PCM-Ausgabe wie der eigene direkte Client. Das ist kein Windows-Lauf von
NVDA. Der veröffentlichte Eloquence_64-v25-Build (Run 41336133144,
Job 124936238535) lädt nachweislich ReLoq C 1.0.1 aus dem Release.
Der bisherige Entwicklungsstand wurde darin also noch nicht mitgeliefert.

Die 16-kHz-Abstimmung aus dev7 bleibt numerisch erhalten. Ab 22,05 kHz
arbeitet Richness nur auf der periodischen Quelle: eine lineare High-Shelf
mit 4,5-kHz-Knie und 1,75/3,5/5,6/7 dB Plateau bei 22,05/32/44,1/48 kHz.
Die unteren Obertöne und die kleine 0,6-dB-Anhebung bei 300 Hz bleiben
verankert. Das erste Shelf bei 2,5 kHz überlagerte den vorhandenen
Classic-Presence-Bereich und übersteuerte weibliche Vokale; es ist nicht
im Paket. Die gemeinsame AV/AH-Kurve erhält keine zusätzliche flache
Oberband-Untergrenze mehr. Der Glottispuls wird nicht verkürzt; es gibt
keinen neuen Limiter. Die bestätigte AF-Konsonantenstrecke bleibt gleich.

Die Classic-Traktberechnung fuehrt unabhängige Frequenzen in zwei
Double-SIMD-Lanes aus. Reihenfolge, Frequenzraster und Filterlänge bleiben
erhalten; für den strikten Vergleich ist RELOQ_CLASSIC_TRACT_SCALAR
verfügbar. Das senkt DSP-Arbeit, beweist aber keine ruckelfreie Wiedergabe
auf dem Windows-ARM-Geraet.

Der ECI-Abgleich ergänzt globale Defaults, Real-World-Units, SDK-Reset,
Unicode-Stimmennamen bis 29 Eingabecodeeinheiten und eciGetIndex. Details
und verbleibende Abweichungen stehen in ECI-COMPATIBILITY.md. Vollständige
ECI-Parität und alle vom Nutzer gehörten Verzerrungen sind nicht
nachgewiesen; insbesondere die 16-kHz-Meldung ließ sich mit den getesteten
Einstellungen noch nicht als Übersteuerung reproduzieren.

# Teststand 1.0.2-dev7: ruhiger Stimmkörper und geringere Rechenlast

Der ECI-Umfang aus dev6 bleibt bestehen. Der Stimmabgleich wertet die
11,025-kHz-Traktantwort jetzt nur bis zur nutzbaren Bandgrenze aus. Oberhalb
davon läuft die Rekonstruktionshüllkurve weiter; die gespiegelte tiefe
Traktantwort nahe 11,025 kHz wird nicht mehr invertiert. Ein gewöhnlicher
englischer /w/-Vokal-Übergang erzeugte zuvor bei 48 kHz eine Kernel-L1-Norm
über 600.000 und sehr große PCM-Spitzen. Der zugehörige Regressionstest
prüft jetzt 410 Varianten über fünf Raten und zwei Traktgrößen.

Der gemeinsame Classic-AV/AH-Hauptbandabgleich bleibt erhalten. Ein
periodischer Bell-Filter um 300 Hz (Q=0,8) gibt dem Stimmkörper 0,6 dB bei
16 kHz bis 1 dB bei 48 kHz zurück. AH und AF bekommen diese zusätzliche
Bassanhebung nicht. Die AF-Implementierung und die z-Kontextregel ändern
sich nicht.

Die zweite, verworfene Trakt-Zielberechnung entfällt im Classic-Pfad.
Unveränderliche Clock-/Rekonstruktions-/Faktorantworten werden atomar einmal
pro Rate vorbereitet; stimmlose Frames ohne serielle Restenergie benötigen
keinen neuen Stimmfit. Es werden keine Text- oder PCM-Ergebnisse gespeichert.

# Teststand 1.0.2-dev6: vollständiges Classic und Standard-ECI

Bei 16 kHz gilt das vollständige Classic-Presence-Ziel für AV, AH und AF.
Die zuvor nur am Hauptband beobachtete Kurve enthält nun auch den stetigen
Rekonstruktionsübergang und die Bildbandunterdrückung oberhalb 5,5125 kHz.
Die native periodische/Aspirations-Anpassung teilt sich eine kausale
Übertragungsfunktion; die bisherige A-Färbung und AH-Absenkung entfallen in B.
Ein vorab ausgeklammerter, im Restfilter kompensierter Pegelverlauf hält die
endliche FIR-Anpassung stabil. Die separate Erweiterung beginnt ab 22,05 kHz.
Es gibt weiterhin kein PCM-Resampling. Z wird nur beim männlichen Modell
am Wortanfang oder in /tsv/ um 2 dB abgesenkt.

Die Standard-ECI-Erweiterungen und Referenzbeobachtungen beschreibt
ECI-COMPATIBILITY.md. Die bisherigen ReLoq-Erweiterungen bleiben nutzbar.

# Teststand 1.0.2-dev5: z-Absenkung nur beim männlichen Modell

Die zusätzliche 2-dB-Absenkung für /ts/ und den Kontakt in zw wirkt nur
beim männlichen Modell. Beim weiblichen Modell bleibt der vom Nutzer
bestätigte Pegel aus dev3 erhalten. Entscheidend ist das jeweilige
Lautmodell einschließlich einer im Text umgeschalteten Stimme.
Die Classic-Presence-Abstimmung aus dev4 bleibt erhalten.

# Teststand 1.0.2-dev4: vollständiges Classic-Presence-Hauptband

B verwendet für alle AF-Konsonanten einschließlich P/F und direktem AB-Anteil
den vollständigen Frequenzgang des 11,025-kHz-Classic-Presence-Hauptbands.
Die vorherige Anpassung enthielt nur die EQ-Anteile; jetzt gehören der
Abfall der Rekonstruktion an der oberen Bandgrenze und der Ausgangsfaktor
0,80 zum Ziel. Die Referenz ist Classic Presence aus eloquence-contours,
Commit 24b9cb121aa3c4f55a6aaacd640f1fe8f03e020a, nicht Presence Treble.
Eigene native C-Filter passen den Rauschpfad an abgeleitete Magnitudenwerte
an; es gibt kein PCM-Resampling und keine Übernahme des Rust-Prozessors.
Die gemessene Referenzbank benutzt die tatsächlichen Quellpegel und die
Formantzahl der Stimme. Der periodische Stimmkörper bleibt bei 16 kHz
exakt A. Bei 16 kHz werden keine zusätzlichen oberen Konsonantenbänder
beigemischt. Die separate native Erweiterung wächst ab 22,05 kHz über
32 und 44,1 bis 48 kHz; das Referenzhauptband bleibt dabei verankert.

Die vom Nutzer bestätigte /ts/-Lautform und der Kontakt nach /ts/ in zw
bleiben erhalten. Ihr AF-Pegel und der /ts/-Burst sinken gegenüber dev3
um 2 dB nach der Anatomie- und Atemanpassung. Die Grundwerte bleiben
Reibung AF=65, Burst AF=66, Kontakt AF=65/AB=60; beim männlichen
Standardmodell ergeben sich AF=63, AF=64 und AF=63. Die AiF-Werte,
Frequenzen und Zeitverläufe ändern sich nicht. Die bereits bestätigte
englische Sekundenlesung aus dev3 bleibt enthalten.

# Teststand 1.0.2-dev3: A-Stimmkörper, Classic-Konsonanten und Sekunden

B kombiniert den unveränderten periodischen Stimmkörper aus A mit dem
Classic-Presence-Konsonantenpfad aus dev2. Die bisherige Classic-Färbung
im periodischen Pfad entfällt. Bei 16 kHz ist der Stimmkörper exakt A;
ab 22,05 kHz wächst eine getrennte obere Anreicherung von nominell
0,375 über 0,75 und 1,2 bis 1,5 dB bei 48 kHz. Die nativen höheren
Konsonantenbänder bleiben pro Rate erhalten. P/F behalten den bisherigen
Rauschpfad. A bleibt als Vergleich auswählbar; die alte Auswahl C wird
in NVDA auf B übernommen. Es gibt keine zusätzlichen Sprachresultat-Caches
und kein PCM-Resampling.

Die native englische Eingabeprojektion lässt HH:MM in der gewöhnlichen
Uhrzeitregel und ergänzt den Sekundenbetrag als „and … seconds“. Sekunden werden als Betrag ohne
führende Null gelesen; genau eine Sekunde erhält den Singular. Bruchteile
bleiben in der gewöhnlichen Zahlenanalyse. Die Erweiterung liegt im nativen
Sprachkern; eine monotone Abbildung der ursprünglichen Bytegrenzen erhält
Quellindizes und Parameterpositionen. Ausgesprochene Lautschrift, Buchstabier-
modus und fremdsprachige Inline-Bereiche werden nicht umgeschrieben.

Die eigene /ts/-Reibung verwendet anhand der Originalbeobachtungen 0.2.39
AF=65, mit A5F=55 am frischen Satzanfang und A5F=50 im verbundenen Sprechen. Der stimmhafte labiodentale
Kontakt nach /ts/ verwendet AF=65/AB=60 statt des unabhängigen /v/-Pegels.
Dies sind Lautklassen und Nachbarschaftsregeln, keine Wortausnahmen.
Die gesicherten Originalwerte enthalten insbesondere Zehn, Zwei, Zahl, zu,
Zunge, Zitronen, zwanzig und Zwerge. Die subjektive Übereinstimmung mit dem
Original bleibt im NVDA-Hörtest zu beurteilen.

# Teststand 1.0.2-dev2: Classic Presence und native Regeln

Der neue Hörtest orientiert B an Classic Presence des 11-kHz-Pfads von
Eloquence64RS: gemeinsame Grundfarbe für Stimmkörper und Konsonanten, ab
16 kHz ein eigener erweiterter Rauschpfad je Abtastrate. Der gemeinsame
Frequenzbereich bleibt am 11-kHz-Vorbild ausgerichtet; darüber kommen die
höheren nativen Rauschbänder hinzu. P und F behalten ihren bisherigen
Rauschpfad. C erweitert zusätzlich die oberen periodischen Anteile, von
0 dB bei 16 kHz bis höchstens 1,5 dB bei 48 kHz. B und C sind bei 16 kHz
gleich. A ist die bisherige Abstimmung. Es gibt kein PCM-Resampling, kein
Erzeugen gespiegelter Frequenzbänder und keine Sprachresultat-Caches.

Der deutsche Laut T (/ts/, etwa Z und zw) hatte seine S-Familienmarkierung
für die Hochratenfilterung nicht erhalten. Das ist korrigiert; die eingestellten
Lautpegel ändern sich dadurch nicht. Eine allgemeine Übereinstimmung des
Z-Klangs mit dem Original ist damit noch nicht durch einen Hörtest bestätigt.

Chinesische Tonhöhenziele verwenden die männliche Referenzkalibrierung bei
Tempo 75, unabhängig von Modell und tatsächlichem Tempo. Die zeitliche
Verteilung bleibt an die tatsächliche Sprechdauer gebunden. Kopfgröße und
weibliche Anatomie bleiben erhalten. 120 Vergleiche prüfen vollständige
Tonhöhenziel-Folgen bei drei Tempi, zwei Modellen, zwei Kopfgrößen und zwei
Tonhöheneinstellungen, einschließlich des englischen Pfads und gemischter Texte.

Der englische Uhrzeitkonverter hatte eine vorhandene Sekundenstelle verbraucht,
ohne sie zu analysieren. Er beendet nun die zweifeldrige Konvertierung vor
den Sekunden; die normalen Zahlenregeln verarbeiten den Rest. Dreifeldrige
Uhrzeiten behalten auch ihre Nullminuten. 370 Prüfungen erfassen US-/britisches
Englisch, alle Sekunden von 00 bis 59, Nullfelder, Bruchteile und AM/PM.
Die unveränderten originalen ENU.SYN und ENG.SYN wurden separat unter Windows
beobachtet: HH:MM und HH:MM:SS erzeugen dort identische PCM-Daten. Der Fehler
ist auch in diesen Originalmodulen vorhanden; es wird keine dort funktionierende
Sekundenregel behauptet. Das Werkzeug speichert nur Messwerte und Hashes,
keine Originalmodule oder Original-Audiodateien im Repository oder Produkt.

Die C-Filter wurden unabhängig anhand des Frequenzgangs formuliert. Kein
Rust-Modul und kein Code von Eloquence64RS wird eingebunden. Die schmale alte
periodische Präsenzanhebung wird im Kandidaten ersetzt. Die beiden übrigen
Quellen AF und AH bleiben von der zusätzlichen C-Stimmkörperanhebung getrennt.

Dies ist kein offizielles Release. Die subjektive Klangbeurteilung erfolgt
in NVDA. Die vollständige positionsbezogene Steuerung (#6) bleibt ein
Release-Prüfpunkt. 1.0.1 auf main bleibt unverändert.

# Technischer Arbeitsstand: 1.0.1

## 1.0.1: Ursprünglicher S-Pegel und Textabbruch

Die zusätzliche Anhebung von S und stimmhaftem S um 4 dB entfällt in den
elf betroffenen Sprachpfaden. Andere Konsonanten und die drei Sprachen ohne
diese Anhebung behalten ihre bisherigen Pegel.

Überholte native Textanalyse reagiert früher auf Abbruch. Der native
Rücksprung stellt auch bei verschachtelten Sprachaufrufen den Frame- und
Workerzustand wieder her. Die folgende Ausgabe kann Sprache und geladene
Wörterbücher weiterverwenden. Ein fehlgeschlagener Lautschriftabruf wird
nicht mehr endlos wiederholt. Sprachergebnisse werden nicht zwischengespeichert.

Der Erststart einer ununterbrochenen langen deutschen Anfrage bleibt von
Textanalyse und Kurvenplanung abhängig. Die generelle Tastendrucklatenz
ist deshalb weiter offen. Klangversuche für hohe Raten, S-Farbe und obere
stimmhafte Anteile bleiben außerhalb dieses Releases.

NVDA übernimmt beim ersten Start seine tatsächlich aktive Oberflächensprache,
sofern keine Standard- oder frühere Sprachauswahl gespeichert ist. Geerbte
Werte bleiben maßgeblich; reine Schemavorgaben zählen nicht als gespeicherte
Auswahl. Der Worker startet gleich mit der gewählten Sprache.

## 1.0: Offizielle Version und britisches Englisch

Das erste offizielle Release heißt 1.0. Die Paketversion 1.0.0 bleibt mit den
bisherigen Update-Prüfungen kompatibel. Produktnamen verlieren ihre sichtbaren
Testzusätze; technische Installationskennungen und das Android-Zertifikat bleiben
für Updates erhalten. Der bisher veröffentlichte Teststand wird nach erfolgreicher
Veröffentlichung der vollständigen drei Pakete wieder als interner Entwurf geführt.

Zwei britische Wörterbucheinträge verwenden nun den vorhandenen LOT-Laut
statt des US-Vokals: o’clock und der Stamm electronic. Die normale Stamm-,
Beugungs- und Kompositaverarbeitung bleibt erhalten.

## 0.2.205: Erstes Paketrelease, Wörterbuchstämme und Pausenkürzung

Das erste GitHub-Release enthält die drei installierbaren Pakete für NVDA,
Android ARM64 und SAPI5 x64/x86. Die Android-APK behält ausdrücklich ihre
Vorschaukennung und bisherige öffentliche Testsignatur für bestehende Updates.
Die SAPI-Startmenüeinträge nennen ReLoq C und umfassen Einstellungen, Anleitung
und Deinstallation. Ein Wechsel der Installersprache erneuert eigene Links.

Deutsche Root-Lautschrift bleibt bis zur Analyse des vollständigen Wortes
lexikalisch erhalten. Die frühere isolierte Synthese beim Laden hat d bereits
zu t verhärtet, bevor Reader gebildet wurde. Der allgemeine Stufenfehler ist
behoben; die normale Auslautverhärtung bei Read allein bleibt erhalten.

Die Pausenkürzung wartet nicht mehr auf exakt null gewordene PCM-Frames.
Nach vollständigem geplantem Quellausklang bleiben wie in den anderen
Sprachplanern kurze physische Ruhebudgets von 3 bzw. 1 ms. Originalkurven,
laufender Filterzustand, explizite Pausen und verkürzte Indexuhren bleiben
erhalten. Praktische Hörtests erfolgen jetzt auf dem Lenovo-ARM-Gerät; der
Dell steht dem Nutzer nicht mehr zur Verfügung.

## 0.2.204: SAPI-Transport, Inno Setup und deutsche Pausen

SAPI rendert direkt auf seinem Ausgabethread. Der erste Audioblock kommt sofort;
weitere werden in einem begrenzten 4096-Sample-Puffer gesammelt. Ereignisse und
Parameterwechsel leeren den Puffer. Die zusätzliche Thread-/Semaphore-Übergabe
entfällt; die gewählte native Rate bleibt bestehen und SAPI konvertiert zum
Anwendungsformat. Ein gemeinsamer Inno-Installer enthält beide Architekturen,
zwölf Oberflächensprachen und die gemeinsame Deinstallation. Wörterbücher
werden nur einmal installiert, Benutzereinstellungen bleiben erhalten.

Deutsch überspringt bei kurzen Pausen die geplanten stillen Parameterframes
vor dem DSP. Die vollständigen ursprünglichen Steuerkurven bleiben bestehen;
damit wandern spätere Formanten nicht rückwärts in den Ausklang. Drei vollständige stille Frames bestätigen vor dem Überspringen, dass die
Filterfahne ausgeklungen ist. Indizes und Wortparameter werden auf den
verkürzten Ausgabezeitstrahl abgebildet.
Explizite Pausen bleiben erhalten. Der Hörtest auf den Nutzergeräten steht aus.

## 0.2.201: Deutscher Pausenausklang, gemeinsame S-Pegel und SAPI5-Vorschau

Alle acht sichtbaren Stimmprofile und die neun öffentlichen ECI-Stimmenvorgaben verwenden Lautstärke 100. Die explizite Nutzerlautstärke bleibt steuerbar. Die festen, stummen deutschen Analyseabfragen behalten ihre bisherigen neutralen Parameter; deren Lautstärke ist kein Ausgabepegel.

Deutsche kurze Pausen werden nach vollständiger akustischer Planung beim Streamen verkürzt. Innerhalb der geplanten Ruheintervalle entfällt ausschließlich die abschließende Folge exakter Nullsamples. Nichtnull-Samples des Resonatorausklangs, innere Nulldurchgänge, die ursprüngliche DSP-Fortschreibung und explizite Pausen bleiben erhalten. 105 Text-/Raten-/Modell-Dreiervergleiche prüfen alle sieben Raten und auch kleine weibliche Kopfgrößen mit Behauchtheit.

Die S-Quellen der zusätzlichen Sprachen verwenden dieselbe Anhebung gegenüber ihren nativen ts-Referenzpegeln wie Deutsch/US-Englisch. Japanisch, kastilisches Spanisch und Griechisch behalten ihre bisherigen Vorgaben. Neun frühere doAF-Regelrahmen werden durch gemeinsame C-Entscheidungen ohne temporäre Arbeitsrahmen ersetzt. Bei lateinamerikanischem Spanisch und Portugiesisch liegen die Lautdatensätze in Slot 4, während Slot 2 Wörter enthält. Ihre C-Quellen lesen deshalb ausdrücklich die tatsächlichen Lautdatensätze.

Die erste SAPI5-x64-Vorschau registriert 14 Sprachstimmen. Ein nativer, per Tastatur bedienbarer Dialog bietet die acht Modelle und zusätzliche Sprachparameter ohne Speichern-Schaltfläche; gültige Änderungen werden sofort pro Sprache und Windows-Nutzer gespeichert und bei der nächsten Äußerung gelesen. Ein fauler ECI-Besitzer-Thread je aktivem COM-Objekt hält die initialisierte Sprache. Acht begrenzte 512-Sample-Pakete reichen PCM und Quellereignisse an den SAPI-Aufrufer zurück. Wortfragmentierung erzeugt keine einzelnen Synthesejobs. Text-/Ereignismetadaten werden nach jeder Äußerung freigegeben. Es gibt keinen Ergebnis-/Wiederholungscache und keine Audiodateien.

Der produktive Backend-Code wird mit einer kleinen reinen Test-Implementierung der Windows-Synchronisationsgrenze gegen den tatsächlichen C-Kern geprüft. Diese Prüfung ersetzt keine Windows-COM-Aktivierung. Die COM-DLL, Installation, SAPI-Eigenschaftenseite und praktische Bildschirmleser-Bedienung müssen auf Windows getestet werden. Die Vorschau enthält dafür zusätzlich einen nativen SAPI-Aktivierungstest mit einem RAM-Ausgabestrom. Einzelheiten und Grenzen stehen unter sapi/README.txt. Öffentliche GitHub-Binärreleases bleiben bis zu den Gerätetests ausstehend.


## 0.2.200: Kompakte C-Einstiege, entfernter Zweit-Renderer und Emoji-Lesungen

Der gemeinsame C-Zeichenleser spricht CLDR-Emoji-Namen jetzt auch im Satz, einschließlich Android. Ein Index für 3.944 RGI-Zeichen und -Sequenzen aus Unicode 17 verwendet die bestehenden übersetzten Namen. Hautfarben, ZWJ-Verbindungen, Flaggen und Tastensymbole werden als vollständige Sequenzen erkannt. Gewöhnliche Zeichenregeln, explizite Textdarstellung und Quellpositionen bleiben berücksichtigt. Die einzige gefundene fehlende Übersetzung, Französisch für U+1FA8A, ist direkt im vorhandenen Namenspool aus CLDR 48.2 ergänzt. Es gibt keine zweite Emoji-Namenssammlung und keine Android-Textersetzung.

Alle 10.896 C-Regeln erhalten statische Namens-/Funktionseinträge. Objektdateinamen, Interpreter-Offsets und Rahmenbeschreibungen entfallen aus den Release-Beschreibungen; deren Daten sinken von 435.840 auf 174.336 Bytes. Die bisherigen Stücktabellen und der beim ersten Sprachzugriff angelegte Nummernindex entfallen im ausgelieferten Binärprogramm. Die Sprachregel-Dateien selbst sind unverändert; die kompakte Bindung liegt in eigenen eingecheckten C-Dateien. Ihr Pflegegenerator liest nur vorhandene C-Schnittstellen. Die weiter vorhandenen generierten Regelabläufe und Datensatzformen sind damit noch nicht vollständig eigenständig umformuliert.

Der private Provider liefert nur Sprach-/Akustikparameter. Der zweite Oszillator-/Filter-Renderer, seine Breitbandbegleitung und seine alten Klangtabellen werden nicht mehr gebaut. Kleine gemeinsam verwendete Rechenhilfen bleiben erhalten; Windows benötigt dazu eine 40-Byte-Tabelle fester Zweierpotenzen. Die eigene ReLoq-PCM-Synthese und die dokumentierten deutschen Abweichungen bleiben erhalten. Diagnose-Dateiausgaben für Akustikframes werden im Release-Pfad entfernt. Es werden keine Sprach- oder Wiederholungs-Caches ergänzt.

Die 110.432 lokalisierten Satz-/Zeichen-/Quellpositionsprüfungen bestehen auch mit ASan/UBSan. 504 öffentliche Emoji-Prüfungen und alle 196 Sprach-/Raten-/Modell-PCM-Vergleiche mit 0.2.199 bestehen; die bisherigen Sprachfälle bleiben bitgleich. Weitere Details, Größen, Prüfungen und Messgrenzen stehen in `docs/changes/0.2.200.md` und `docs/validation/0.2.200.json`.

Die DLL sinkt um 682.496 auf 54.544.384 Bytes, das Add-on um 347.354 auf 19.508.738 Bytes und die APK um 262.144 auf 17.446.294 Bytes. Der angeforderte Heap für 4.000 deutsche Wörter bleibt bei 58.628.850 Bytes. Host-CPU-Messungen belegen keinen allgemeinen Latenzgewinn auf Nutzergeräten.

Die Gemeinschaftswörterbücher stammen vom aktuell aufgelösten Master `1ecd298e87880131dad201378ac39a80940b342e`; alle sieben Git-Blobs sind geprüft. Android behält Paketkennung, Vorschau-Zertifikat, Browser-Updateweg und INTERNET als einzige Berechtigung. Das Upgrade 20199 auf 20200 ist geprüft. SAPI5 bleibt im Releaseplan.

## 0.2.199: Zitierte Kennungen, gemeinsame Zahlenregeln und kleinere Akustikpuffer

Der vollständig ergänzte Vorbereitung-Text ist reproduziert. Ein typografisches Schlusszeichen wurde am Ende von SAPI5 als unbekannter Wortteil statt als native Zeichengrenze behandelt. Dadurch fiel der gesamte Absatz auf die ältere Ersatzanalyse zurück. Die Aufnahme gemischter Kennungen verwendet jetzt auch dort die vorhandene Zeichenklassifikation. Der Absatz bleibt in der nativen Analyse; es entsteht kein Vorbereitung-Wörterbucheintrag.

Die eigene vier-und-Verbindung steht jetzt direkt in den Kardinaldaten. Ihre bisherige nachträgliche Adapterkorrektur ist entfernt. Auch die gemeinsame native Lautschriftprojektion liefert diese Verbindung für Zahlen in Dateinamen und anderen Kennungen. Ziffernweise gewöhnliche Kommanachkommastellen werden an der nativen Zahlenkonvertierung gewählt; die Euro-Kontextprüfung erhält die Betragslesung. Die zusätzliche 8-dB-Absenkung nach langem Ö samt eigenen Absenkungsrampen ist auf Nutzerwunsch entfernt; die R-Färbung bleibt erhalten.

Die Akustikstufen reservieren Zielkurven nach wirklichem Bedarf. Erst ein erfolgreicher Plan übergibt seine Arrays ohne Kopie an den Renderer. Abbruch und Fehler geben private Arrays frei. F0 behält seine begrenzte Melodie-/Steuerreserve. Die bestehenden extern gebundenen Puffer bleiben unterstützt. Details und Messgrenzen stehen in `docs/changes/0.2.199.md` und `docs/validation/0.2.199.json`.

Die Gemeinschaftswörterbücher stammen vom frisch aufgelösten Master `80eeab8c14e869e8edce03ec4d6908b71d35be33` mit sieben geprüften Git-Blobs. Die APK behält Kennung, Vorschau-Signatur und Browser-Updateweg. SAPI5 bleibt der im Releaseplan beschriebene nächste Plattformschritt.

Im 4.000-Wörter-Test sinkt die Spitze angeforderter Heap-Zuweisungen von 91.652.850 auf 58.628.850 Bytes (minus 33.024.000, rund 36,0 Prozent) bei identischem PCM. Alle 196 Sprach-/Raten-/Modellvergleiche bleiben bitgleich; 46 eigene deutsche SPR-Lautungen bleiben erhalten. Die absichtliche Ö-R-Pegeländerung behält in 21 Raten-/Modellpaaren die Samplezahl und clippt dort nicht. 220 vollständige native Gruppen-/Akzentvergleiche, 1.664 variable Quellspannen und 208 öffentliche Fragment-/Tag-/Indexprüfungen bestehen. Der vollständige C-ECI-Test und die übrigen 26 ECI-Prüfschritte sowie NVDA/Android-Prüfungen bestehen.

Beim gemeldeten Absatz sinkt die Host-CPU-Zeit bis zum ersten PCM-Block im Median von 12,80 auf 5,89 ms (11 kHz) bzw. 14,29 auf 5,91 ms (48 kHz). Das ist ein klar abgegrenzter Effekt des vermiedenen Analyse-Rückfalls. Die allgemeine CPU-Gegenüberstellung und der Nachlauf mit fester CPU-Zuordnung bleiben gemischt. Ein allgemeiner CPU-Gewinn wird daraus nicht abgeleitet. Die DLL wächst um 3.584 Bytes, das Add-on um 3.161 Bytes; die APK behält ihre Größe. Gerätemessungen bleiben ausstehend.

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
