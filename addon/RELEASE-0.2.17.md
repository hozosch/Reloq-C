# Eloquence C 0.2.17

Neu:
- Glottisschläge als eigene Verläufe von Stimmimpuls-Unregelmäßigkeit und Öffnungsdauer. Wortanfang, innere Silbengrenze und schwache Wörter werden unterschieden.
- 528 Wort-/Lautschrift-Kombinationen bewahren zusätzliche Anstoßmerkmale. Passende Ableitungen übernehmen sie nur bei unveränderter Lautfolge des Stamms. So bleibt etwa der innere Vokal in Theater und Ruine ohne falschen Glottisschlag.
- Zusammenhängende Zahlenwörter statt einzelner Wörter für ein, und und zwanzig. Führende Nullen, große Kardinalzahlen, numerische Datumsangaben, Uhrzeiten mit Uhr und einfache Brüche sind erweitert.
- Mehrere Zeichennamen folgen dem beobachteten Original, darunter Klammeraffe und Nummernzeichen.
- Buchstabennamen werden direkt im C-Buchstabenpfad ausgegeben. C in CD bekommt dadurch keinen K-Laut mehr; F, L, M, N und R behalten ihren korrekten Vokal. Eszett erhält seine beobachtete Nebenbetonung.
- Der NVDA-Buchstabiermodus trennt auch mehrere Buchstaben und Ziffern innerhalb eines Textstücks. Umlaute, Indexmarken und Tonhöhenwechsel werden dabei berücksichtigt.

Festgelegte Zahlenlesung:
3,14: drei Komma eins vier.
3,14 EUR / 3,14 Euro / 3,14 € / 3,14€: drei Euro vierzehn.
0,50 EUR: fünfzig Cent.
Die Euro-Schreibweisen benutzen denselben Pfad und erzeugen im Test identisches Audio.

Hörvergleich:
reloq-0.2.17-glottisschlaege-vergleich.wav enthält drei Sätze.
Pro Satz ist die Reihenfolge: Original mit normalem Text, Version 0.2.16, Version 0.2.17.
1. Die Mutter öffnet das Fenster am Nachmittag.
2. Anna öffnet alle Türen.
3. Am Abend essen wir einen Apfel.
Es wird normale Texteingabe verglichen, keine Wiedergabe von Lautschrift.

Prüfung und Grenzen:
97 C-Testprogramme, ECI-ABI/Diagnostik, 40 Sprachfälle, 36 Zahlenfälle mit Indexmarken, Tempo und Stummschaltung sowie die Paket-/Treibersimulation bestehen. Windows-x64-DLL: 19 geprüfte Exporte. Vier geänderte Pfade bestehen Adress- und Undefined-Behavior-Prüfungen; die Laufzeit verhindert die separate Leak-Prüfung.
1.942 beobachtete Glottisschlag-Verläufe enthalten sämtliche erwarteten DI/OQ-Stützpunkte im eigenen C-Modul. Die Zeit- und Kontextwerte sind dabei Prüfeingaben; das beweist noch keine identische Gesamtsynthese.
Bei 1.050 vergleichbaren Wörtern/Sätzen stimmen die Anstoßentscheidungen über 3.322 Vokalkerne überein. Ein weiterer Fall hat bereits eine andere Lautfolge und wurde gesondert ausgewiesen.
1.288 von 1.289 Zahlen-/Datums-/Bruchfällen stimmen in Lautschrift und Wortgruppierung mit der Referenz überein. Der offene Fall ist ein ausgeschriebenes Datum mit flektierter Ordnungszahl. Normale Kommazahlen und vereinheitlichte Euro-Schreibweisen werden zusätzlich nach der vereinbarten Lesung geprüft.
92 zusätzliche Buchstabenfolgen stimmen in Lautnamen und Gruppierung mit dem Original überein. 15 native Buchstabierprüfungen bestehen mit Audio, Indexmarken, zwei Tempi und identischer Ausgabe von CD und C D beziehungsweise NVDA und N V D A. Im gemischten Feld aus 99 Einzelzeichen, Kürzeln und Wörtern steigt die Übereinstimmung von 75 auf 88 Fälle; kein zuvor passender Fall geht verloren.
27 vollständige Textfälle stimmen zwischen C-Textplan und ECI-Ausgabe überein; keine Übersteuerung. Drei Sätze mit bisher verbesserten stimmhaften s-Lauten bleiben gegenüber 0.2.16 bytegleich. In 15 akustischen Vergleichssätzen beträgt die mittlere absolute Abweichung der aktiven Sprachdauer 83 statt 82 ms; der gesamten Ausgabedauer 93 statt 65 ms. Diese Werte sind kein Hörurteil.
Ein älterer separater Test der wortweisen Tonhöhensteuerung scheitert vor der markierten Wortgrenze. Derselbe Fehler ist mit der eingefrorenen Version 0.2.16 reproduzierbar und bleibt offen.
Vollständige Wortbildung, Zeitplanung und Klanggleichheit sind weiterhin nicht erreicht. Unter echtem Windows/NVDA wurde dieser Stand hier nicht gehört.
Auch die Entscheidung zwischen Wortlesen und Buchstabieren bleibt unvollständig: NATO und HALLO werden noch fälschlich buchstabiert, EU wird noch als Wort gelesen. ÖPNV und die Abkürzung z.B. sind weitere offene Beispiele. Die Buchstabenkorrektur ersetzt keine vollständige Wortanalyse.

Das Add-on enthält eigenständig formuliertes C und beobachtete sprachliche Daten; keine Originalprogramme oder Originalmodule. Das Ganzwortwörterbuch mit 15.921 Einträgen und die allgemeinen Konsonantenpegel wurden nicht pauschal verändert.
