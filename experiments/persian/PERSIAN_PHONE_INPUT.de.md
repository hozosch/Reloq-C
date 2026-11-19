# Persische Lautschrifteingabe: bestehende Zeichen

Diese Übersicht beschreibt die **gegenwärtige interne** Zeichenfolge, die der
persische Testpfad an die finnisch basierte Eloquence-C-Klangerzeugung
übergibt. Die persischen Akustikanpassungen müssen dabei aktiv sein. Sie ist
keine neue, allgemein zugesicherte Wörterbuchschnittstelle. Groß- und
Kleinschreibung sind entscheidend. Die gewählten Klänge und Übergänge bleiben
unverändert; die folgende Tabelle ist keine Neuprogrammierung der Stimmen.

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
| `G` (groß) | Ach-Laut | der finnische Platz für Ng wird im persischen Akustikbereich umbelegt |
| `R` (groß) | persischer Zäpfchenlaut | gesondert vom normalen `k` und `r`; die vorhandene Realisierung ist eine abgestimmte Annäherung |
| `w` (klein) | gesonderter Glottis-/Trägerplatz | im persischen Pfad eine technische Belegung; kein englisches W |

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

Wortfinales H erhält momentan den stillen technischen Träger `w`, etwa
`` `[.1solhw] ``. Das `w` ist hier kein zusätzlich gesprochener persischer
Laut: Es ermöglicht den normalen H-Impuls auch bei verbundenen Wörtern. An
anderer Stelle wird `w` im bisherigen internen Pfad als Marker für einen
Glottisverschluss verwendet. Deshalb lässt sich ein H unmittelbar vor einem
separaten Glottisverschluss mit dieser Zwischenform nicht eindeutig ausdrücken.
Außerdem benötigen einige Kontexte weitere Angaben außerhalb der reinen
Zeichenkette, zum Beispiel die besondere Behandlung eines gedehnten Dsch.

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
* Die bestehenden Nutzerwörterbücher ersetzen bereits Text beziehungsweise
  verarbeiten ihre jeweiligen Ausspracheangaben. Dass sie beliebige neue
  persische Lautdaten in jeder Plattform übernehmen, ist noch nicht belegt.
* Satzmelodie, Satzzeichen, Sprachwechsel, Indizes, Fallback und die echte
  native Sprachauswahl sind mit dieser Zeichentabelle allein nicht umgesetzt.
  Diese Übersicht verspricht auch keinen bereits messbaren Reaktionsgewinn.
