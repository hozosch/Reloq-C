# Eloquence C 0.2.10 – deutscher NVDA-Hörtest

Der x64-Teststand läuft weiterhin mit 11.025 Hz und einer unabhängigen
C-Engine. Die Warteschlange und die Chunk-Pausen des Treibers wurden für
diese Version nicht verändert.

## Neuer Übergang

Innerhalb einer Silbe gehen die deutschen `x/X`-Laute (ch) nun mit einer
getrennt geplanten Vokaltrakt- und Stimmgeste zum folgenden Vokal über. Die
Rauschdauer bleibt sieben Millisekunden nach der Lautgrenze wie zuvor. Die
Änderung betrifft zum Beispiel „Kuchen“ und „Mädchen“, aber nicht die
bereits guten wortanfänglichen p/k/t→Vokal-Übergänge.

Nach einem Verschlusslaut im selben Wort trägt das folgende `r` jetzt einen
kräftigeren Stimm- und Rauschanteil. Der Übergang wurde für p/t/k/b/d/g→r
als Lautklasse geprüft, ohne einzelne Wörter zu hinterlegen.

In 74 natürlichen Prüfsätzen traf die Änderung fünf ch→Vokal-Stellen. Jede
verbesserte den Pegel- und Spektralvergleich gegen die öffentliche
Apple-ECI-Ausgabe. Alle 52 p/k/t→Vokal-Kontrollen blieben unverändert;
Satzlängen, Wortindizes und die Clip-Zahl ebenso. In einem Satz mit
„Mädchen … lacht“ blieb der spätere Laut „lacht“ samplegenau unverändert.
In denselben drei Satzmengen lagen 23 Übergänge vom Verschlusslaut zu `r`;
die mittleren Pegel- und Spektralfehler sanken in jeder Satzmenge. Andere
Übergänge, insbesondere der bereits passende direkte p/k/t→Vokal-Anlaut,
blieben im Vergleich der Kontrollfenster weitgehend gleich.

## Einordnung

Der Test umfasst nur fünf passende Stellen. Er belegt weder einen allgemein
korrekten Übergangsplan noch die Übernahme der originalen Sprachregeln.
Andere Kandidaten für Vokal→Frikativ und n→t wurden wegen schlechter
Gegenproben zurückgehalten. Ein Hörtest unter NVDA ist der nächste Schritt.
