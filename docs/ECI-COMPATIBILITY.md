# Standard-ECI-Abgleich in 1.0.2-dev8

Die Referenz ist die vom Nutzer bereitgestellte `ibmeci.dll` (Windows x86,
344064 Bytes), SHA-256
`8741d4aeccf3280ea644443146194ebb81d43dee65e2ee460031b9182475a83c`.
Ihre Exporte und die Parameterprüfungen wurden statisch untersucht. Der neue
Wrapper ist eigenständiges C; die Referenz-DLL wird nicht mitgeliefert.
Die eigene Windows-x64-DLL wird gegen den deklarierten öffentlichen ABI geprüft.

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
| `eciNewDict`, `eciLoadDict`, `eciSetDict`, `eciGetDict`, `eciDeleteDict` | Eigene Wörterbuch-Handles mit einzelnen Main-/Root-/Abbreviation-Volumes. Beliebige Dateinamen; geladene Daten gehören dem Handle und überstehen das Entfernen der Quelldatei. Fehlgeschlagene Ladevorgänge behalten vorhandene Daten. Volumes werden mit dem vorhandenen nativen Parser verarbeitet. |

## Ergänzungen in dev8

`eciGetDefaultParam` und `eciSetDefaultParam` verwalten die implementierten
Standardparameter 0, 1, 2, 3, 5, 8 und 9 atomar. Ein bestehendes Handle bleibt
unverändert; neue Handles übernehmen die aktuellen Werte. Ungültige Werte
werden vor der Änderung abgelehnt. Die ReLoq-Ratencodes 3..6 bleiben eine
bewusste Erweiterung gegenüber dem alten Windows-ECI.

`eciReset` setzt bei Clients, die ausschließlich Standardaufrufe verwenden,
Parameter, aktive Stimme und bearbeitbare Stimmenplätze auf diese Defaults
zurück und verwirft Text sowie Synthesehistorien. Callback, Ausgabepuffer und
eigene Wörterbuch-Handles bleiben beim Host. Clients mit ausdrücklich
verwendeten ReLoq-Steuerparametern behalten den bisherigen Resetvertrag,
damit vorhandene NVDA-/Android-/SAPI-Adapter ihre Einstellungen nicht verlieren.
Diese beiden Ausnahmen (Ausgaberouting und ReLoq-Reset) sind dokumentierte
Kompatibilitätsentscheidungen, keine behauptete 1:1-Nachbildung des Originals.

`eciGetIndex` liefert den zuletzt gemeldeten Index; er wird vor dem zugehörigen
Callback atomar veröffentlicht und ist auch während der Ausgabe abfragbar.
Ohne vorherigen Index ist der Wert 0. Reset verwirft diesen zuletzt gemeldeten
Wert nicht, entsprechend dem beobachteten getrennten Zustand des Originals.

Bei Real-World-Units bleiben angeforderte Hz/WPM/Volume-Werte beim Zurücklesen
und beim Kopieren zwischen Stimmenplätzen erhalten; der Renderer verwendet
den nächstliegenden nativen Wert. Die privaten nativen ReLoq-Aliasse arbeiten
weiter in ihren dokumentierten Rohwerten.

## Konkrete Referenzbeobachtungen

`eciSetVoiceParam` liegt bei 0x100052f7. Die Prüfung ab 0x100053bf lässt
nur Platz 0 und die bearbeitbaren Plätze 9..16 zu; die Parameter werden in
der Standardreihenfolge geprüft. `eciCopyVoice` bei 0x1000454d prüft die
Zielplätze ebenfalls. `eciLoadDict` bei 0x1000938d liest den Sprachparameter
für die Unterscheidung von Codesätzen und erweiterten asiatischen Volumes.
`eciGetAvailableLanguages` bei 0x10001b1b reicht Ausgabeadresse und Zähladresse
an den installierten Sprachmanager weiter; die eigene Liste stammt deshalb
aus den tatsächlich eingebauten C-Sprachmodulen.

## Umfang der Prüfung

`nvda/test_eci_standard_api.py` benutzt für Stimme, Sprache, Text und
Wörterbücher ausschließlich die Standardaufrufe. Es prüft reale PCM-Ausgabe
und Indexpositionen, Stimmenplätze, Fehlversuche, Wechsel der Sprachkodierung
und Gleichheit zwischen Byte- und UTF-16-Eingabe. `nvda/test_eci_extended_standard_api.py` prüft zusätzlich Einheiten, Defaults,
Reset, lange Unicode-Namen und Live-Indizes. Die anderen nativen
ReLoq-Schnittstellen bleiben für vorhandene NVDA-/Android-/SAPI-Clients erhalten.

Dieser Abgleich deckt die oben genannten Aufrufe ab. Weitere Original-Exporte,
asiatische MainDictExt-Volumes und das vollständige Snapshot-/Thread-Verhalten des alten ECI
sind damit noch nicht als vollständig kompatibel nachgewiesen. Die Referenz
wurde hier statisch untersucht; ein Windows-Lauf des proprietären Originals
ist keine der aufgeführten Prüfungen.

## Noch offene Originalfunktionen

Die Referenz exportiert 60 Funktionen. Dieser Wrapper exportiert 34, davon
30 aus dieser Referenz und vier zusätzliche ReLoq-Funktionen. Die folgenden
Original-Exporte fehlen weiterhin; sie werden nicht durch Erfolg meldende
Leerfunktionen vorgetäuscht:

`eciActivateFilter`, `eciClearErrors`, `eciDeactivateFilter`, `eciDeleteFilter`, `eciDictFindFirst`, `eciDictFindFirstA`, `eciDictFindNext`, `eciDictFindNextA`, `eciDictLookup`, `eciDictLookupA`, `eciErrorMessage`, `eciGeneratePhonemes`, `eciGetFilteredText`, `eciIsBeingReentered`, `eciNewFilter`, `eciPause`, `eciProgStatus`, `eciRegisterVoice`, `eciSaveDict`, `eciSetFilter`, `eciSetOutputDevice`, `eciSetOutputFilename`, `eciSpeakText`, `eciSpeakTextEx`, `eciSynthesizeFile`, `eciTestPhrase`, `eciUnregisterVoice`, `eciUpdateDict`, `eciUpdateDictA`, `eciUpdateFilter`.

Auch bei vorhandenen Exporten ist vollständige Parität nicht belegt:
`eciSynthesize` läuft hier synchron; `eciSynthMode=1` speichert noch keine
unterschiedlichen Parameterzustände je AddText-Fragment. TextMode 2/3,
NumberMode, Phonemindizes und zusätzliche asiatische Codesatzmodi werden nicht
angeboten. Eine Audio-Geräte-/Dateiausgabe und das native Filter-ABI benötigen
eigene Implementierungen. Diese Grenzen sind für einen beliebigen ECI-Client
relevant, auch wenn der untersuchte Eloquence_64-ReLoq-Adapter sie umgeht.

## Klangprofil im Eloquence_64-Adapter

Das eigene NVDA-Add-on wählt Profil B ausdrücklich. Für bisherige direkte
ECI-Clients bleibt der DLL-Standard Profil A (Parameter 0x1011 = 0).
Eloquence_64 b24a6c8 wählt diesen zusätzlichen Parameter bisher nicht.
Ein reiner DLL-Austausch behebt dort die Satzanalyse, aktiviert aber noch
nicht die in diesem Hörtest geprüfte Classic-/Richness-Abstimmung.

`integration/eloquence64-classic-profile.patch` ergänzt beim Vorbereiten
jedes ReLoq-Sprachhandles optional `eciSetParam(handle, 0x1011, 1)`.
Frühere DLLs dürfen den unbekannten Parameter ablehnen und bleiben nutzbar.
Der Patch gilt für den untersuchten Upstream-Commit und wurde lokal mit
dessen tatsächlichem Adapter bei 16, 22,05 und 48 kHz geprüft: 24 vollständige
und fragmentierte Satzfälle liefern die gleiche PCM-Ausgabe wie unser
Client mit Profil B. Der Patch ist nicht an Upstream gesendet worden.
