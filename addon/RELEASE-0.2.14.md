# Version 0.2.14: gemeinsame Dauerplanung

Die zuvor separat geprüften Dauerregeln steuern jetzt unterstützte
Silbenkerne der hörbaren C-Stimme. Vokale, Lautübergänge und Diphthongbewegung
teilen sich ein Zeitbudget. Auch Übergänge nach Reibelaut plus p/t/k und
passende schwache R-Endungen werden angeschlossen.

In 62 normalen Testsätzen verändern sich alle Audioausgaben; 626 Silbenkerne
verwenden die neue Planung. Die mittlere absolute Satzdauerabweichung sinkt
von 152 auf 142 ms. Einzelne Sätze verschlechtern sich, und Klanggleichheit
ist noch nicht erreicht. Wortanalyse und Wörterbuch sind unverändert.
Mehrgliedrige Vokal-r/l-Verbindungen und weitere unvollständige Kontexte
behalten den bisherigen Ablauf.

Installation: Add-on-Datei in x64-NVDA installieren, NVDA neu starten und
unter Sprachausgabe „Eloquence C (experimentell, Deutsch)“ auswählen.
Es handelt sich um eine Aktualisierung desselben experimentellen Synthesizers.

Die begleitende Hördatei enthält drei Sätze. Pro Satz lautet die Reihenfolge:
Original mit normaler Texteingabe, C 0.2.13, C 0.2.14. Innerhalb einer Gruppe
liegt eine Sekunde Pause, zwischen Satzgruppen zwei Sekunden.

Technische Details, gemischte Messwerte und bekannte Grenzen stehen in
`reference-apple/NUCLEUS-SCHEDULE-0.2.14-DE.md`.
