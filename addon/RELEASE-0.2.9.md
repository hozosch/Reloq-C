# Eloquence C 0.2.9 – deutscher NVDA-Teststand

Die auswählbare x64-Stimme bleibt bei 11.025 Hz und verwendet den unabhängigen C-Kern.

## Änderungen

- Gültige Zeichen wie `§`, `©`, `°`, `²` und `\` brechen den ganzen Satz nicht mehr ab. Gebräuchliche Zeichen haben deutsche Namen; andere gültige Zeichen werden als „Zeichen“ ausgesprochen. 217 zulässige CP1252-Zeichen wurden jeweils im Satz mit PCM und Endindex geprüft.
- Ein bisher ungedecktes Wort blockiert nicht mehr alle Wörter desselben Auftrags. Für eindeutige deutsche Komposita verbindet der C-Textpfad zwei bereits sprechbare Teile zu einer Wortgruppe; eine eng geprüfte Fugen-s-Klasse ist enthalten. „Tonqualität“, „Klangqualität“, „Bildqualität“ und „Lebensqualität“ erzeugen nun Audio. Die Aussprache dieser Gruppen bleibt angenähert.
- Nach einem tatsächlichen Stillstand des Audioplayers meldet der Treiber den verworfenen Puffer als Fehler und räumt ihn vor dem nächsten Auftrag ab. Der normale schnelle Abbruch und die Chunk-Warteschlange bleiben erhalten.

## Prüfung und Grenze

Die native ECI-Matrix spricht 122/122 vollständige CP1252-Texte und 8/8 zusätzliche Unicode-Fälle nach der Add-on-Zulassung. 80/80 Komposita bilden je eine Wortgruppe und liefern Audio mit Endindex; 16/16 Kontrollwörter bleiben unverändert. In der gepaarten Prüfung sind 112 zuvor sprechende Sätze samplegenau gleich. Windows-x64-DLL und 19 ECI-Exporte wurden gebaut und geprüft; ein Hörtest in einem echten Windows-NVDA-Prozess steht aus.

Die Abdeckung und Fehlerbehandlung sind verbessert. Die Betonung, Lautübergänge und der Gesamtklang sind weiterhin nicht allgemein Apple/Eloquence-getreu; insbesondere sind Komponenten des Kompositums noch vom experimentellen G2P abhängig.
