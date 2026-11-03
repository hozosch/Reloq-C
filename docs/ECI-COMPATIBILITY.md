# Standard-ECI-Abgleich in 1.1.0-rc3

Die Referenz ist die vom Nutzer bereitgestellte `ibmeci.dll` (Windows x86,
344064 Bytes), SHA-256
`8741d4aeccf3280ea644443146194ebb81d43dee65e2ee460031b9182475a83c`.
Ihre Exporte und die Parameterprüfungen wurden statisch untersucht. Der neue
Wrapper ist eigenständiges C; die Referenz-DLL wird nicht mitgeliefert.
Die eigene Windows-x64-DLL wird gegen den deklarierten öffentlichen ABI geprüft.

Zusätzlich wurde die originale **Host-ECI.DLL aus Eloquence_64 v25** unter
Windows mit 32-Bit-Python tatsächlich ausgeführt: Version 6.1.0.0, 176128 Bytes,
SHA-256 `c1653e61e9fd599421d2f7faee21eda171d3a8ff4329343ddb24a2dd565fba5f`.
Der reproduzierbare Probeaufruf steht in `tools/observe_eci_contract.py`, die
Beobachtungen in `docs/validation/eci-original-eloquence64-v25.json`.
Diese Host-DLL exportiert **weder `EciSetVoice` noch `eciSetVoice`**.
Stimmparameter werden über `eciSetVoiceParam`, Presets über `eciCopyVoice`
gewählt. Beide Standardfunktionen sind in ReLoq vorhanden.

| Aufruf | Verhalten |
| --- | --- |
| `eciGetAvailableLanguages` | Zählabfrage mit Kapazität 0; begrenztes Füllen mit positiver Kapazität; 15 vorhandene Sprach-/Dialektkennungen; Parameterfehler 0x80. |
| `eciNew` / `eciNewEx` | Standardmäßig die normale C-Textausgabe; `eciNew` übernimmt den globalen Sprachstandard (anfangs US-Englisch), `eciNewEx` die ausdrücklich gewählte Sprache. Der alte exakte deutsche Vergleichspfad ist ausdrücklich über `VOICE_MODE=0` erreichbar. |
| `eciGetVoiceParam` / `eciSetVoiceParam` | Acht Standardparameter; aktive Stimme 0 teilt Zustand mit den bestehenden nativen ReLoq-Parametern. Geschlecht 0..1, Tempo 0..250, übrige Parameter 0..100. Setter liefert den vorherigen Wert, -1 bei Ablehnung. Mit Parameter 8 werden Hz, Wörter pro Minute und der 16-Bit-Lautstärkebereich verwendet; die vorhandene native SDK-Kalibrierung wird wiederverwendet. |
| `eciCopyVoice` | Plätze 1..8 sind die unveränderlichen Presets, 9..16 bearbeitbare Kopien; nach 0 wird die aktive Stimme atomar übernommen. |
| `eciGetVoiceName` / `eciSetVoiceName` | Namen in der gewählten Sprachkodierung bzw. UTF-16LE; maximal 29 Eingabecodeeinheiten, unabhängig von der internen UTF-8-Bytelänge. |
| `eciSetParam(h,9,language)` | Wechsel der Sprache auf demselben Handle, einschließlich Prüfung und Austausch des Sprach-Workers. Ungültige Kennungen lassen die bisherige Auswahl bestehen. |
| `eciAddText` | Windows-1252 für westliche Sprachen, Windows-1253 für Griechisch, CP932/936/949/950 für Japanisch/vereinfachtes Chinesisch/Koreanisch/traditionelles Chinesisch. Bit 0x800 im Sprachwort wählt UTF-16LE. `eciAddTextUtf8` bleibt eine zusätzliche Schnittstelle. |
| `eciSetParam(h,1,1)` | Aktiviert die Annotationen, wie beim Standardparameter `eciInputType`. |
| `eciNewDict`, `eciLoadDict`, `eciSetDict`, `eciGetDict`, `eciDeleteDict` | Eigene Wörterbuch-Handles mit einzelnen Main-/Root-/Abbreviation-Volumes. `eciSetDict` liefert wie das Original `eciDictNoError` (0) bei Erfolg, keinen booleschen Wert; rc3 korrigiert den zuvor falschen Erfolgswert 1. Beliebige Dateinamen; geladene Daten gehören dem Handle und überstehen das Entfernen der Quelldatei. Fehlgeschlagene Ladevorgänge behalten vorhandene Daten. Volumes werden mit dem vorhandenen nativen Parser verarbeitet. |

## Ergänzungen in dev8

`eciGetDefaultParam` und `eciSetDefaultParam` verwalten die implementierten
Standardparameter 0, 1, 2, 3, 5, 8 und 9 atomar. Ein bestehendes Handle bleibt
unverändert; neue Handles übernehmen die aktuellen Werte. Ungültige Werte
werden vor der Änderung abgelehnt. Die ReLoq-Ratencodes 3..6 bleiben eine
bewusste Erweiterung gegenüber dem alten Windows-ECI.

`eciReset` setzt bei allen Clients Standardparameter, aktive Stimme,
bearbeitbare Stimmenplätze, Eingabe und Synthesehistorien auf die globalen
Defaults zurück. Ob vorher ReLoq-Parameter benutzt wurden, spielt seit dev9
keine Rolle mehr. Callback, Ausgabepuffer, eigene Wörterbücher und die separat
gewählte akustische Kontur bleiben erhalten. Insbesondere die erhaltene
Ausgaberoute bleibt eine dokumentierte Abweichung vom alten Geräte-Reset.
Interne numerische Tests benutzen für einen reinen Historienreset einen
eigenen, nicht exportierten Aufruf.

`eciGetIndex` liefert den zuletzt gemeldeten Index. Die Referenz aktualisiert
ihn nach dem Callback, auch bei einer Antwort von `eciDataNotProcessed`.
Darum sieht der erste Index-Callback noch den vorherigen Wert, seine
Wiederholung dagegen bereits den gemeldeten Index. Dev8 veröffentlichte den
Wert irrtümlich vorher; dev9 korrigiert das. Reset verwirft ihn nicht.

Bei Real-World-Units bleiben angeforderte Hz/WPM/Volume-Werte beim Zurücklesen
und beim Kopieren zwischen Stimmenplätzen erhalten; der Renderer verwendet
den nächstliegenden nativen Wert. Die privaten nativen ReLoq-Aliasse arbeiten
weiter in ihren dokumentierten Rohwerten.

## Allgemeine Erweiterungen in dev9

Die folgenden Regeln gelten in der DLL unabhängig vom aufrufenden Add-on:

- `eciDataNotProcessed` (0) bietet denselben PCM-Puffer oder Index nach einer
  kurzen Wartezeit erneut an. PCM, Quellphase und spätere Indizes laufen erst
  nach Verarbeitung weiter. `eciDataProcessed` (1) bestätigt; `eciDataAbort`
  (2) bricht ab. `eciStop` beendet auch eine wiederholte Nichtverarbeitung.
- `eciProgStatus`, `eciErrorMessage`, `eciClearErrors` und
  `eciIsBeingReentered` sind reguläre Exporte. Fehlerbits bleiben über
  erfolgreiche Aufrufe hinweg erhalten, bis ClearErrors oder Reset sie
  löschen. Die zuletzt gemeldete Fehlerursache bestimmt den Text; Bit 0x80
  kennzeichnet ungültige Parameter, 0x800 einen abgewiesenen Wiedereintritt.
  Die private letzte Synthesediagnose bleibt davon getrennt.
- `eciDictLookup`, `eciDictLookupA`, `eciDictFindFirst`, `eciDictFindNext`,
  deren A-Formen, `eciUpdateDict`, `eciUpdateDictA` und `eciSaveDict` arbeiten
  auf den nativen Wörterbuchtabellen. Änderungen eines aktiven Sets werden
  beim nächsten Sprechen wirksam. Fehlgeschlagene Änderungen behalten den
  bisherigen Satz. NULL als Übersetzung löscht einen Eintrag. Schlüssel und
  Übersetzungen aus Lookup/Iteration gehören der Instanz. Aktive Sets werden
  pro Sprache/Dialekt gespeichert: Sprachwechsel und Reset wählen das für die
  aktuelle Sprache aktivierte Set; ein fremdsprachiges Set überschreibt nicht
  die Zuordnung der gerade gesprochenen Sprache.
- Die drei vorhandenen Volumes Main, Root und Abbreviation sowie schmale
  Sprachcodesätze und UTF-16-Ein-/Ausgabe werden unterstützt. Die einfache
  Lookup-Funktion lehnt wie die Referenz UTF-16 ab; LookupA übernimmt diesen
  Fall. MainDictExt bleibt nicht implementiert und liefert einen Volume-Fehler.
  Ungültige Ausgabezeiger werden geprüft; die alte Abhängigkeit von
  uninitialisiertem Aufruferspeicher wird nicht nachgebildet.
- `eciInsertIndex` nimmt alle 32 Bit als Nutzwert an, einschließlich negativer
  Werte. Ein abgewiesenes AddText löscht bereits akzeptierte Eingaben nicht.
  Delete und Synchronize während eines abgewiesenen Wiedereintritts melden
  den regulären ECI-Fehler, ohne das aktive Handle zu zerstören.
- Der Standard-Include `eci.h` und der erweiterte Header `reloq_eci.h`
  enthalten die gebräuchlichen ECI-Parameter-, Nachrichten-,
  Rückgabe-, Sprach- und Dictionary-Namen. Die öffentliche ABI verwendet
  weiterhin 32-Bit-Integer und die originale Windows-Aufrufkonvention.

## Allgemeine Erweiterungen in dev10

`eciGeneratePhonemes(h, room, buffer)` benötigt wie die untersuchte Referenz
einen Callback und `eciSynthMode=1`. Native Phonembytes erscheinen als
`eciPhonemeBuffer` (Nachricht 1), nicht als PCM. Der Puffer gehört dem Client;
der Callback erhält die tatsächlich belegte Bytezahl. Kleine Puffer,
eingebettete NUL-Bytes, Callback-Wiederholung, Abbruch und Stop werden
berücksichtigt. Die vorherige PCM-Ausgaberoute wird nicht ersetzt.
Textfragmente werden gemeinsam zugeführt. Die ausgewählte Sprachkodierung,
Unicode-Zulassung, aktiven Wörterbücher und nativen Sprachregeln gelten auch
hier. Die zusätzliche ReLoq-Pitch-Marke ist in dieser Ausgabe nicht erlaubt.

`eciPause(h, value)` hält weitere Callbacks an. 0 setzt fort, andere Werte
aktivieren die Pause. Pause/Resume und Stop dürfen aus dem Callback oder von
einem zweiten Thread kommen. Stop beendet eine angehaltene Ausgabe und
entfernt den Pausenzustand. Diese Implementierung betrifft den vorhandenen
Puffer-/Callback-Pfad; ein nativer Windows-Audiogerätetreiber ist weiterhin
nicht implementiert. Synthese und Phonemausgabe laufen weiterhin synchron.

## Ergänzungen in 1.1.0-rc1

Für die PCM-Synthese hält jeder nichtleere AddText-Aufruf die acht nativen
Stimmwerte fest. `eciSetVoiceParam(h,0,...)` und die entsprechenden nativen
ReLoq-Setter ändern bereits eingereihten Text nicht rückwirkend. Änderungen
zwischen Textteilen werden als interne, wortbezogene Steuerungen in denselben
Plan eingesetzt; sie benötigen keine weiteren InsertRate-/InsertVolume-
Exporte. Der zuletzt konfigurierte Handle-Zustand bleibt nach der Synthese
erhalten. Das gilt auch bei normalen Texten mit deaktivierten Annotationen.

447 Prüfungen vergleichen reale Queue-Änderungen mit Inline-Steuerungen,
Byte-/UTF-8-Eingabe, beide Setter-Routen, alle acht Parameter und aktivierte
bzw. deaktivierte Tags. Weitere Fälle prüfen `vv` innerhalb westlicher Wörter
und die Lautstärkesteuerung der CJK-Module. Diese verwenden ihre eigenen
akustischen Wortgrenzen, keine vom Host erfundenen Leerzeichenwörter.
`vb` nutzt bei den westlichen Modulen ohne zusätzliche ECI-Pitch-Marken die
kontinuierliche Wort-F0-Steuerung. Die kombinierte Legacy-Route mit zusätzlichen
Pitch-Marken bleibt erhalten und ist nicht vollständig mit allen Inline-
Tagfolgen abgeglichen. Andere globale Parameter und die Phonem-Queue haben
noch keine vollständige fragmentbezogene Snapshot-Parität.

## Konkrete Referenzbeobachtungen

Der Windows-Lauf der Host-DLL bestätigt Sprachaufzählung, Wechsel von Parameter
9 auf demselben Handle, Rückgabe des vorherigen Stimmparameterwerts,
Preset-Kopien und Wörterbuch-Handles. Preset 1 setzt auch das Tempo auf 50
zurück; es bleibt nicht bei einem zuvor eingestellten Tempo. Die Parameter
41/42 werden bereits vom Original mit -1 abgelehnt und erhalten daher keine
erfundenen Standard-Aliasse. ReLoq hat 15 eingebaute Dialekte, diese konkrete
Host-Installation meldet 13. Voreinstellungen für Lautstärke und private
ReLoq-Parameter bleiben eigene Kalibrierungen.

Die folgenden statischen Adressen beziehen sich auf die zuerst untersuchte
344064-Byte-Referenz, nicht auf die kleinere Host-DLL aus v25.

`eciSetVoiceParam` liegt bei 0x100052f7. Die Prüfung ab 0x100053bf lässt
nur Platz 0 und die bearbeitbaren Plätze 9..16 zu; die Parameter werden in
der Standardreihenfolge geprüft. `eciCopyVoice` bei 0x1000454d prüft die
Zielplätze ebenfalls. `eciLoadDict` bei 0x1000938d liest den Sprachparameter
für die Unterscheidung von Codesätzen und erweiterten asiatischen Volumes.
`eciGetAvailableLanguages` bei 0x10001b1b reicht Ausgabeadresse und Zähladresse
an den installierten Sprachmanager weiter; die eigene Liste stammt deshalb
aus den tatsächlich eingebauten C-Sprachmodulen.

`eciGeneratePhonemes` beginnt bei 0x100076dc. Die Prüfungen bei 0x100077b3
und 0x100077c0 verlangen Callback und SynthMode 1. Anschließend wird die
Ausgabe auf die übergebene Phonemroute umgestellt und die Queue verarbeitet.

## Umfang der Prüfung

`nvda/test_eci_standard_api.py` benutzt für Stimme, Sprache, Text und
Wörterbücher ausschließlich die Standardaufrufe. Es prüft reale PCM-Ausgabe
und Indexpositionen, Stimmenplätze, Fehlversuche, Wechsel der Sprachkodierung
und Gleichheit zwischen Byte- und UTF-16-Eingabe. `nvda/test_eci_extended_standard_api.py` prüft zusätzlich Einheiten, Defaults,
Reset, lange Unicode-Namen und Live-Indizes. `nvda/test_eci_contract_api.py` prüft
zudem Wiederholungen desselben Puffers, Stop während einer Wiederholung,
Fehlerakkumulation, Wiedereintritt, einheitlichen Reset und aktive
Wörterbuchänderungen mit realer PCM-/Indexausgabe. Die anderen nativen
ReLoq-Schnittstellen bleiben für vorhandene NVDA-/Android-/SAPI-Clients erhalten.

Dieser Abgleich deckt die oben genannten Aufrufe ab. Weitere Original-Exporte,
asiatische MainDictExt-Volumes und das vollständige Snapshot-/Thread-Verhalten des alten ECI
sind damit noch nicht als vollständig kompatibel nachgewiesen. Der zusätzliche
Windows-Lauf der Host-DLL prüft die im Beobachtungsprotokoll aufgeführten
Aufrufe; er ersetzt keine Prüfung aller historischen ECI-Funktionen.

## Noch offene Originalfunktionen

Die Referenz exportiert 60 Funktionen. Dieser Wrapper exportiert 49, davon
45 aus dieser Referenz und vier zusätzliche ReLoq-Funktionen. Die folgenden
Original-Exporte fehlen weiterhin:

`eciActivateFilter`, `eciDeactivateFilter`, `eciDeleteFilter`, `eciGetFilteredText`, `eciNewFilter`, `eciRegisterVoice`, `eciSetFilter`, `eciSetOutputDevice`, `eciSetOutputFilename`, `eciSpeakText`, `eciSpeakTextEx`, `eciSynthesizeFile`, `eciTestPhrase`, `eciUnregisterVoice`, `eciUpdateFilter`.

Auch bei vorhandenen Exporten ist vollständige Parität nicht belegt:
`eciSynthesize` läuft hier synchron; der neue AddText-Snapshot deckt die acht
nativen Stimmparameter der PCM-Route ab. Das vollständige Queue-/Thread-
Verhalten aller Parameter ist weiterhin offen. TextMode 2/3,
NumberMode, Phonemindizes und zusätzliche asiatische Codesatzmodi werden nicht
angeboten. Eine Audio-Geräte-/Dateiausgabe und das native Filter-ABI benötigen
eigene Implementierungen. Diese Grenzen sind für einen beliebigen ECI-Client
relevant, auch wenn der untersuchte Eloquence_64-ReLoq-Adapter sie umgeht.

## Klangprofil im Eloquence_64-Adapter

Profil C ist jetzt der DLL-Standard (Parameter 0x1011 = 3) und wird auch im
eigenen NVDA-Add-on als Vorgabe verwendet. Bereits gespeicherte Einstellungen
der früheren Vorgabe B und des früheren C-Hörtests werden auf C übernommen;
Profil A und B bleiben ausdrücklich auswählbar. Der native Wert 2 bleibt zur
Kompatibilität ein Alias für B. Clients wie Eloquence_64, die den zusätzlichen
Parameter nicht setzen, verwenden damit ebenfalls C.

`integration/eloquence64-classic-profile.patch` zeigt weiterhin die frühere,
ausdrückliche Auswahl von Profil B. Ein Client, der diese Einstellung setzt,
überschreibt damit den neuen Standard. Frühere DLLs dürfen den unbekannten
Parameter ablehnen und bleiben nutzbar.
Der Patch gilt für den untersuchten Upstream-Commit und wurde lokal mit
dessen tatsächlichem Adapter bei 16, 22,05 und 48 kHz geprüft: 24 vollständige
und fragmentierte Satzfälle liefern die gleiche PCM-Ausgabe wie unser
Client mit Profil B. Der Patch ist nicht an Upstream gesendet worden.

Die Unicode-Konvertierung der neuen Dictionary-API wird lokal unter Linux
und über Windows-APIs implementiert; der JNI-UTF-8-Pfad bleibt erhalten.
Die Legacy-UTF-16-Dictionary-Konvertierung auf Android ist damit noch nicht
nachgewiesen. Der ursprüngliche C-Parser und die Sprachregeln werden hierfür
nicht durch Ersatz-Wortregeln des Wrappers ersetzt.
