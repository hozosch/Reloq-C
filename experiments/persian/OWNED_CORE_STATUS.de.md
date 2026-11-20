# Eigenständiger persischer Sprachkern: Arbeitsstand

Der neue Ereignisplaner ist ein Baustein und noch keine eigene Stimme. Die
NVDA-Teststimme verwendet für abgedeckte Eingaben bereits die native
Textprojektion aus derselben Regelbasis, aber noch nicht den Ereignisplaner
als vollständigen Syntheseweg. Der finnische Sprachkern und der englische
Melodielauf sind aus dem experimentellen Stimmpfad noch nicht entfernt.

`eciPersianAnalyzeRunsUtf8` überführt unterstützte persische Texte, Zahlen,
Zeichennamen, Pinglish und Lautschrifteingaben in persische Laut- und
Grenzereignisse. Der isolierte Test baut diesen Pfad ohne finnischen,
englischen oder eSpeak-Sprachprozessor. Die bestehende Lexik speichert ihre
Aussprachen noch in einem älteren internen Format; der neue Planer wandelt sie
selbst in die persischen Lautidentitäten um. Für Zeichen ohne eigenen Namen
verwendet er eine persische Lesung des Unicode-Codes.

Die sprachunabhängigen Lautereignisse tragen auch die bisherigen allgemeinen
Regeln für kurze unbetonte Anfangsvokale und für ein schwaches i zwischen
Vokalen. Für ein kurzes unbetontes A zwischen Vokalen vor einer betonten
Silbe gibt es einen vorsichtigen Hörtest-Kandidaten. Der Klang des langen
Vokals bleibt als eigene Eigenschaft erhalten.
Der bisherige akustische Testpfad verwendet dieselben strukturellen Regeln.
Die Ereignisplanung allein ist weiterhin keine Synthese und ersetzt den alten
Pfad noch nicht.

Die aktuelle Stimme übergibt die Entscheidung über stimmlose Lauteinsätze und
die Glättung weiterer stimmhafter Sprachwechsel inzwischen an den gemeinsamen
C-Kern. Der Referenzvergleich erhält bereits passende Übergänge und ihre
H-Impulse. Fehlt der Konsonant in einem neutralen Referenzlauf, wird ein kurzer
stetiger Übergang mit nachgewiesenem Verschluss oder Reibegeräusch erhalten.
Die bisherige Python-Berechnung ist dafür nicht mehr nötig. Die
Vorbereitung der Sprachabschnitte und weitere Klangübergänge liegen dagegen
noch im experimentellen Treiber. Auch dieser Fortschritt ist daher noch keine
vollständig eigenständige persische Stimme.

Die nächste freie Sprachnummer nach Aragonesisch (19) ist 20. Der neue
Analysepfad erkennt deshalb `` `l20.0 `` und `` `l20.1 `` als Wechsel
zwischen persischer Schrift und Pinglish. Intern entsprechen sie `0x140000`
und `0x140001`. Der Testtreiber verwendet jetzt ebenfalls die Nummer 20; die
alten Kennungen `0x7f0000` und `0x7f0001` werden dort für gespeicherte
Einstellungen als Alias akzeptiert. Die neue Nummer ist noch nicht in der
normalen ECI-Sprachauswahl registriert. Andere Sprachen muss später der
gemeinsame ECI-Router behandeln.

Lautschrift verwendet `q` für den Zäpfchenlaut, `x` für den Ach-Laut und `?`
für den Glottisschlag. `h` und ein nachfolgendes `?` bleiben getrennte
Ereignisse. Fehlerhafte Gruppen werden im neuen Analysepfad mit persischen
Zeichennamen behandelt; sie werden keinem fremden Textparser übergeben.

Noch offen für eine nutzbare eigene Stimme:

* Die persischen Aussprache- und Morphologieregeln müssen Wörter außerhalb
  der bisherigen direkten Lexik besser abdecken. Der strenge Analysemodus
  meldet solche Lücken; ein zusätzlicher Modus kann sie anhand allgemeiner
  Schrift- und Kontextregeln aussprechen. Dessen ungeschriebene kurze Vokale
  sind Näherungen und benötigen weitere sprachliche Prüfung. Ist eSpeak im
  NVDA-Testpfad verfügbar, wird es für noch nicht abgedeckte Fälle weiterhin
  als Referenz verwendet.
* Die bereits abgestimmten Klänge müssen aus den Lautereignissen mit einer
  eigenen persischen Laut- und Übergangsplanung erzeugt werden. Dabei müssen
  unter anderem der H-Impuls und der R–A-Übergang hörbar geprüft werden.
* Satzmelodie, Betonung, Pausen, Ereignismarken, Wörterbücher und
  Umschaltungen müssen in den nativen Sprachpfad eingebunden und für NVDA,
  SAPI5 und Android getestet werden. Der Treiber soll danach nur die
  Plattformanbindung übernehmen.
* Die Standard-ECI-Sprachauswahl und `l`-Umschaltung müssen mit der neuen
  Synthese verbunden werden. Erst dann sind die beiden Kennungen für eine
  echte Stimme nutzbar.

Test unter Linux: `python experiments/persian/test_persian_owned_plan.py`.
Er prüft unter anderem die persische und lateinische Eingabe, Zahlen,
Umschaltungen, fehlgeschlagene Lautschrift, Zeichennamen und alle gespeicherten
Lautgruppen. Der Test ist kein Hörtest und belegt keine Reaktionszeit im NVDA.
