# Eloquence C 0.2.7 – experimentelles NVDA-Test-Addon

Der weiterhin unabhängige C-Kern ist als x64-ECI-DLL im auswählbaren NVDA-Add-on enthalten. Die Stimme unterstützt derzeit Deutsch bei 11.025 Hz. Die Warteschlange, Chunk-Pausen und die Anhebung der Stimme bei Großbuchstaben aus 0.2.6 bleiben erhalten.

## Änderungen

- Ein interner `V.CCV`-Übergang mit zwei stimmlosen Verschlusslauten `/p t k/` wird anhand der Lautklasse, Betonung und angrenzenden Vokale geplant. Die Regel arbeitet nur im belegten A/E/U-zu-A/I-Bereich mit expliziter Betonung; sie enthält keine Sonderbehandlung einzelner Wörter.
- In 27 kontrollierten Apple/C-Fällen stimmen Coda-Dauer, zweiter Burst, Aspirationsbeginn und folgender Vokaleinsatz an den gemessenen Zielpunkten, einschließlich neun unabhängiger U/I-Fälle. „Papptipp“ und „Papptipps“ treffen die betreffenden Apple-Zeitpunkte. Die PCM-Aktivdauer nähert sich nur mäßig an; daraus folgt noch keine allgemeine Klanggleichheit.
- Verbundene Wortgrenzen bleiben unverändert und trafen in 18 von 18 Stop-zu-Stop-Kontrollen bereits die Apple-Zeitpunkte. 52 weitere Sätze und sechs andere Stop- beziehungsweise Satzgrenzfälle blieben im C-PCM bitidentisch.

## Weitere Abweichungen

Der Nutzer bestätigt die anfänglichen `/p k t/` direkt vor Vokalen als gut. Eine isolierte Anhebung dieser Anfänge wird daher nicht ausgeliefert. Andere Konsonantenkontexte, Reibelaute, Lautübergänge und Ausklänge weichen weiterhin ab. Eine breite Apple/C-Matrix mit 570 CV-, 729 VC- und 405 CC-Fällen sowie 270 frühen Pegelmessungen liegt mit reproduzierbaren Skripten unter `reference-apple/`. Naheliegende reine Pegel- oder Einpunktänderungen wurden nach Gegenproben verworfen, weil sie die Coda-Spektren oder unabhängige Sätze verschlechterten.

Zum Hörvergleich eignen sich „Papptipp“, „Papptipps“, „Abtippen“ sowie fortlaufende Sätze mit p/t/k und f/s. „Abtippen“ fällt derzeit wegen einer anderen Betonung noch nicht unter die neue V.CCV-Regel. Die unveränderten Lautanfänge `/p k t/` und Wortgrenzen dienen als Gegenprobe.
