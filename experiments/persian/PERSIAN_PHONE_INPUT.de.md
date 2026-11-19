# Persische Lautschrifteingabe: bestehende Zeichen

Diese Übersicht beschreibt die explizite persische Lautschrifteingabe und
ihre aktuelle Übergabe an die Eloquence-C-Klangerzeugung. Die persischen
Akustikanpassungen müssen aktiv sein. Groß- und Kleinschreibung sind
entscheidend. Die gewählten Klänge und Übergänge bleiben unverändert. Die
gesamte Wörterbuch- und Plattformintegration ist noch nicht abgeschlossen.

## Vokale

| Zeichen | Gegenwärtiger persischer Laut | Hinweis |
|---|---|---|
| `A` (groß) | kurzes, helles Ä | in `سلام` der erste Vokal; finnischer Ä-Platz, persisch abgestimmt |
| `a` (klein) | hinteres Iran-A | in `سلام` der zweite Vokal; finnischer A-Platz, gesondert abgestimmt |
| `e` | E | vorhandener, persisch abgestimmter Vokal |
| `i` | I | vorhandener, persisch abgestimmter Vokal |
| `o` | O | vorhandener, persisch abgestimmter Vokal |
| `u` | U | vorhandener, persisch abgestimmter Vokal |

Ein `i` oder `a` bezeichnet hier die Lautkategorie. Die konkrete Dauer hängt
von Betonung, Position und Klangplanung ab. Ein explizites Längenzeichen wie
`'a:'` ist eine zusätzliche Angabe und darf nicht bei jedem Vorkommen des
Iran-A automatisch gesetzt werden.

Der unveränderte finnische Lautschriftpfad enthält darüber hinaus beispielsweise
`O` für Ö und `y` für Ü; `I`, `U` und `Y` treten als Bestandteile finnischer
Doppellaute auf, etwa in `kaikki`, `sauna` und `pöytä`. Diese zusätzlichen
finnischen Zeichen gehören nicht zur gegenwärtigen persischen Grundbelegung.
Ohne aktivierten persischen Akustikbereich steht `G` weiterhin für finnisches
Ng statt für den persischen Ach-Laut.

## Konsonanten

| Zeichen | Gegenwärtiger persischer Laut | Hinweis |
|---|---|---|
| `b d f g k l m n p s t v z` (klein) | die entsprechenden B-, D-, F-, G-, K-, L-, M-, N-, P-, S-, T-, W- und stimmhaften S-Laute | Die bereits gewählten Klänge bleiben erhalten; mehrere sind an andere Eloquence-Sprachkerne angelehnt. `v` entspricht deutschem W, `z` stimmhaftem S. |
| `r` (klein) | persisches R | vorhandene R-Behandlung einschließlich Kontextregeln |
| `j` (klein) | J wie in „ja“ | am Wortanfang an den deutschen J-Laut angelehnt |
| `h` (klein) | H | am Wortende wird derzeit ein zusätzliches internes Trägerzeichen benötigt, siehe unten |
| `S` (groß) | stimmloses Sch | wie in „Schule“ |
| `Z` (groß) | stimmhaftes Sch | etwa wie das J in „Journal“ |
| `C` (groß) | Tsch | ein Laut mit Verschluss und anschließendem Reibelaut |
| `J` (groß) | Dsch | stimmhaftes Gegenstück zu `C`; nicht mit kleinem `j` verwechseln |
| `x` (klein) | Ach-Laut | analog zum deutschen Lautschriftzeichen; die Übergabe verwendet vorerst den internen Platz `G` |
| `q` (klein) | persischer Zäpfchenlaut | gesondert von `k` und `r`; die vorhandene Realisierung ist eine abgestimmte Annäherung und wird intern vorerst als `R` übergeben |
| `?` | Glottisschlag | nur innerhalb einer persischen Lautschriftgruppe; wird intern vorerst als `w` übergeben |

`h`, `C` und `J` akzeptiert der tatsächliche Lautschriftparser, obwohl sie in
der statischen Liste `fifi.settings` nicht als einzelne `Phoneme`-Einträge
stehen. Diese Metadatenliste ist daher nicht mit dem vollständigen
Eingabewortschatz des Parsers gleichzusetzen. Die `C`- und `J`-Laute werden im
Kern in ihre Verschluss- und Reibanteile umgesetzt; ihre Klangregeln wurden
für den persischen Pfad bereits angepasst.

## Form einer Lautgruppe

Das Anfangszeichen ist ein Backtick, danach folgen eine öffnende und eine
schließende eckige Klammer. Silbenmarkierungen sind `.0` für unbetont, `.1`
für Haupt- und `.2` für Nebenbetonung. Beispiel:

```text
`[.0sA.1lam]
```

Dies ist die bestehende interne Aussprache von `سلام` / „salam“: S, großes A,
L, kleines A, M. Die zweite Silbe ist betont. Diese Schreibweise ist weder
IPA noch eine beliebige Pinglish-Umschrift.

Wortfinales H erhält intern den stillen technischen Träger `w`. Die explizite
Eingabe `` `[.1solh] `` wird dafür als `` `[.1solhw] `` übergeben. Das `w`
ist hier kein zusätzlich gesprochener persischer Laut: Es erhält den normalen
H-Impuls auch bei verbundenen Wörtern. Ein Fragezeichen außerhalb einer
Lautschriftgruppe bleibt ein Satzzeichen. Die bisherigen `G`, `R` und `w`
werden für vorhandene Eingaben weiterhin akzeptiert. Da `w` in der alten
Zwischenform auch einen Glottisverschluss markiert, wird H vor einem
Glottisverschluss vor folgendem Vokal unterstützt. Ein unmittelbar nach H
endender Glottisverschluss ist dort noch nicht eindeutig auszudrücken. Der
Kern weist diesen Fall zurück, statt einen Laut zu verschlucken.
Einige Kontexte benötigen weitere Angaben außerhalb der reinen Zeichenkette,
zum Beispiel die besondere Behandlung eines gedehnten Dsch.

## Konsequenzen für den Umbau

* Die oben bezeichneten, vorhandenen Laute sind der Ausgangspunkt. Für eine
  eigene persische Sprache müssen sie nicht neu erfunden werden.
* Die gemeinsame Lautbasis muss das **persische** Lautinventar und zusätzliche
  Informationen wie Betonung, Silbengrenzen, Längen und Kontext selbst tragen.
  Die aktuelle finnisch basierte Zeichenkette ist bis zum Ersatz der Übergabe
  eine Kompatibilitätsschicht, keine Definition durch finnische Orthografie.
  Ein erster experimenteller C-Baustein beschreibt 29 Lautidentitäten mit
  Silbe, Betonung und expliziter Länge und kann die vorhandene Zeichenfolge
  verlustfrei hin und zurück umsetzen. Er ist noch nicht der normale
  Text- oder Satzplaner und führt keine eigenständige Synthese aus.
* Persische Schrift und Pinglish können dieselben Laute verwenden. Ihre
  Textanalyse bleibt unterschiedlich: Beide Schreibweisen können mehrdeutig
  sein. Insbesondere werden kurze Vokale im Persischen oft nicht geschrieben.
  Wörterbücher und Wortbildungsregeln bleiben deshalb wichtig.
* Die direkte Pinglish-Analyse für abgedeckte ASCII-Sätze, ihre Wortregeln,
  Betonung und die Projektion von Textmarken stehen im gemeinsamen C-Kern.
  Nicht abgedeckte oder nicht lateinisch-ASCII geschriebene Fälle verwenden
  vorerst weiterhin den bisherigen Weg. Der bestätigte U–A-Übergang kann im
  C-Kern geplant werden; einige gemischte Satzpfade verwenden noch den
  bisherigen Planer.
* Die bestehenden Nutzerwörterbücher ersetzen bereits Text beziehungsweise
  verarbeiten ihre jeweiligen Ausspracheangaben. Dass sie beliebige neue
  persische Lautdaten in jeder Plattform übernehmen, ist noch nicht belegt.
* Die akzeptierte Satzmelodie wird weiterhin aus einem englischen Pilotlauf
  übertragen und noch im Treiber koordiniert. Einfache explizite persische
  Lautschriftgruppen erhalten diese Kontur ebenfalls. Eine eigenständige
  persische Melodieplanung, vollständige Textanalyse ohne Fallback und echte
  native Sprachauswahl sind damit noch nicht umgesetzt. Ein großer Gewinn
  der Reaktionszeit ist damit noch nicht belegt.
