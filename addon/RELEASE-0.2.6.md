# Eloquence C 0.2.6 – experimentelles NVDA-Test-Addon

Dieses Add-on enthält weiterhin eine deutsche C-Stimme mit 11.025 Hz für x64-NVDA und ARM64EC-NVDA mit x64-DLL-Unterstützung.

## Änderungen

- NVDA kann die Tonhöhe beim Buchstabieren von Großbuchstaben per `PitchCommand` innerhalb einer Äußerung anheben und wieder zurücksetzen. Die Engine verarbeitet dafür native Tonhöhenmarken. Andere Prosodiebefehle innerhalb des Textes sind noch nicht umgesetzt.
- Eine allgemeine Regel setzt `ab-`, `auf-` und `aus-` mit vollständig analysierten Verbstämmen zusammen. In einer getrennten 952-Wörter-Probe stiegen exakte Übereinstimmungen mit der öffentlichen Apple-Phonemausgabe von **318 auf 340**; zuvor exakte Treffer gingen dort nicht verloren.
- **1.337** einzeln und in Satzkontexten bestätigte Ausspracheeinträge ergänzen die bisherigen **1.205**. Das Add-on lädt ein zusammengeführtes Wörterbuch mit **2.542** Einträgen; die neuen Einträge enthalten keine Wörter aus der genannten Probe.
- Bei `/v/` vor einem Vokal wurde der erste F1-Zielwert korrigiert. Ein Vokal vor einem Nasal im selben Wort erhält auch über eine Silbengrenze eine B1-Antizipation; der FNZ-Verlauf bei `/n/` steigt im Nasal an. Diese Änderungen betreffen Lautkontexte, keine einzelnen Wörter.

## Messgrenzen

Die größere Wörterbuchdatei erhöhte die mediane ECI-Initialisierungszeit auf dem Linux-Testrechner um **0,861 ms**. Die Zeit bis zum ersten PCM-Block nach der Initialisierung blieb dort praktisch gleich. Das ist keine NVDA-Latenzmessung unter Windows.

Die Ausspracheprobe misst Phonem-Übereinstimmung und keinen Höreindruck. Die Übergänge wurden in kontrollierten Lautkontexten geprüft; ein NVDA-Hörtest muss Stimmklang, Bass, Nasalausklang und fließende Sprache noch beurteilen. Weitere Sprachen, Abtastraten und vollständige Eloquence-Regeln sind nicht enthalten. Die bekannte native Pause an NVDA-Chunkgrenzen bleibt Teil des Treibers.

**Offener Punkt:** Die AV/AH-Ausklänge und die Endstille sollen für interne Satzgrenzen und einzelne NVDA-Äußerungen gemeinsam auf einer konsistenten Zeitachse berechnet werden. Ein Apple-näherer interner Ausklang veränderte im Versuch die bereits passende Chunk-Pause; eine kompensierte Fassung traf den Klang nicht. Deshalb enthält 0.2.6 hier keine weitere Produktänderung.
