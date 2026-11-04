# Vergleich des Gleitlauts in „Homepage“

Als Referenz diente die proprietäre 32-Bit-ECI.DLL aus dem von Eloquence 64
verwendeten Paket `eloquence_threshold` v0.20210417.01 mit dessen deutscher
Sprachdatei. Die Messung lief unter Windows mit 11.025 Hz, Stimme 1, Kopfgröße
50, Tonhöhe 65, Schwankung 30, Geschwindigkeit 75 und Lautstärke 100. Es
wurden keine Originaldateien oder Sprachaufnahmen in dieses Repository
übernommen.

Die SHA-256-Prüfsumme der ECI.DLL war
`c1653e61e9fd599421d2f7faee21eda171d3a8ff4329343ddb24a2dd565fba5f`,
die von DEU.SYN
`97c2069b4c782b0f52d72bd9f5be7120945b1cec1eff0104e2b97ee8d56f448d`.

Für das Wort „Homepage“ wurden im Originalsignal 30-ms-Fenster mit 10 ms
Schrittweite, Vorbetonung 0,97 und LPC-Ordnung 14 ausgewertet. Im Bereich des
Gleitlauts um 309–319 ms lagen die ermittelten Resonanzspitzen ungefähr bei
275 Hz, 2.125–2.150 Hz und 2.900–2.925 Hz. Diese Signalwerte sind keine
direkt aus der DLL ausgelesenen Formantparameter. Der unveränderte native
ReLoq-C-Pfad setzt an der entsprechenden Stelle 265, 2.150 und 2.900 Hz.

Der neue Hörkandidat verwendet 285, 2.100 und 2.850 Hz und verschiebt den
nativen Ausgangspunkt damit leicht nach hinten. Die Messung belegt die Nähe
des Ausgangspunkts, ersetzt aber keinen Hörvergleich der Verschiebung.
