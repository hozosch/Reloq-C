# Standard-ECI-Abgleich in 1.0.2-dev6

Die Referenz ist die vom Nutzer bereitgestellte `ibmeci.dll` (Windows x86,
344064 Bytes), SHA-256
`8741d4aeccf3280ea644443146194ebb81d43dee65e2ee460031b9182475a83c`.
Ihre Exporte und die Parameterprüfungen wurden statisch untersucht. Der neue
Wrapper ist eigenständiges C; die Referenz-DLL wird nicht mitgeliefert.
Die eigene Windows-x64-DLL wird gegen den deklarierten öffentlichen ABI geprüft.

| Aufruf | Verhalten |
| --- | --- |
| `eciGetAvailableLanguages` | Zählabfrage mit Kapazität 0; begrenztes Füllen mit positiver Kapazität; 15 vorhandene Sprach-/Dialektkennungen; Parameterfehler 0x80. |
| `eciNew` / `eciNewEx` | Standardmäßig die normale C-Textausgabe; `eciNew` startet US-Englisch. Der alte exakte deutsche Vergleichspfad ist ausdrücklich über `VOICE_MODE=0` erreichbar. |
| `eciGetVoiceParam` / `eciSetVoiceParam` | Acht Standardparameter; aktive Stimme 0 teilt Zustand mit den bestehenden nativen ReLoq-Parametern. Geschlecht 0..1, Tempo 0..250, übrige Parameter 0..100. Setter liefert den vorherigen Wert, -1 bei Ablehnung. |
| `eciCopyVoice` | Plätze 1..8 sind die unveränderlichen Presets, 9..16 bearbeitbare Kopien; nach 0 wird die aktive Stimme atomar übernommen. |
| `eciGetVoiceName` / `eciSetVoiceName` | Namen in der gewählten Sprachkodierung bzw. UTF-16LE. |
| `eciSetParam(h,9,language)` | Wechsel der Sprache auf demselben Handle, einschließlich Prüfung und Austausch des Sprach-Workers. Ungültige Kennungen lassen die bisherige Auswahl bestehen. |
| `eciAddText` | Windows-1252 für westliche Sprachen, Windows-1253 für Griechisch, CP932/936/949/950 für Japanisch/vereinfachtes Chinesisch/Koreanisch/traditionelles Chinesisch. Bit 0x800 im Sprachwort wählt UTF-16LE. `eciAddTextUtf8` bleibt eine zusätzliche Schnittstelle. |
| `eciSetParam(h,1,1)` | Aktiviert die Annotationen, wie beim Standardparameter `eciInputType`. |
| `eciNewDict`, `eciLoadDict`, `eciSetDict`, `eciGetDict`, `eciDeleteDict` | Eigene Wörterbuch-Handles mit einzelnen Main-/Root-/Abbreviation-Volumes. Beliebige Dateinamen; geladene Daten gehören dem Handle und überstehen das Entfernen der Quelldatei. Fehlgeschlagene Ladevorgänge behalten vorhandene Daten. Volumes werden mit dem vorhandenen nativen Parser verarbeitet. |

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
und Gleichheit zwischen Byte- und UTF-16-Eingabe. Die anderen nativen
ReLoq-Schnittstellen bleiben für vorhandene NVDA-/Android-/SAPI-Clients erhalten.

Dieser Abgleich deckt die oben genannten Aufrufe ab. Weitere Original-Exporte,
asiatische MainDictExt-Volumes, Real-World-Units und das vollständige Snapshot-/Thread-Verhalten des alten ECI
sind damit noch nicht als vollständig kompatibel nachgewiesen. Die Referenz
wurde hier statisch untersucht; ein Windows-Lauf des proprietären Originals
ist keine der aufgeführten Prüfungen.
