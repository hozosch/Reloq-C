# Eloquence C 0.2.18

Dieser Stand ersetzt mehrere allgemeine Näherungen durch unabhängig formulierte C-Regeln aus untersuchtem Eloquence-Verhalten. Er erreicht weiterhin keine vollständige Klanggleichheit.

## Änderungen

- Gemeinsame Planung von Lautausklang und Pause: Stimmton, Aspiration und Stille erhalten getrennte Zeitpunkte. Die Pause erzeugt keine eigenen F1/F2/F3-Ziele mehr. Dadurch werden Ausklänge und Übergänge über Satzzeichen nicht mehr zum bisherigen neutralen Pausenprofil gezogen. Fortsetzende Kommata und Auslassungspunkte unterscheiden sich vom endgültigen Satzende.
- n und ng nach einem Vokal vor weiteren Konsonanten verwenden die belegten Stimmkurven. Je nach Folgelaut entstehen andere Pegelpunkte oder bewusst keine zusätzlichen Punkte. Das ist keine allgemeine Pegelanhebung. Dauer und nasale Resonanz dieser Konsonantenfolgen sind damit noch nicht vollständig rekonstruiert.
- Inneres ng schließt in der freien Wortanalyse die vorherige Silbe. Die bisherige Zuordnung zum folgenden Silbenanfang verursachte zugleich falsche Vokallängen. Betroffen sind ganze Gruppen, etwa Sängerin, anfangen, angesungen, Bedingungen, Schwingungen und Zunge. Bereits zusammengesetzte Vorsilben-/Stammanalysen behalten ihre Grenzen.
- Die Entscheidung zwischen Wortlesen und Buchstabieren bei Großbuchstaben folgt den untersuchten Originalregeln für Buchstabenklassen, Wortanfang, Wortende und Ausnahmen. NATO und HALLO können als Wörter gelesen werden, EU und ÖPNV werden buchstabiert. Großbuchstabenwörter erreichen außerdem die normale Wörterbuchanalyse.

Das Ganzwortwörterbuch enthält unverändert 15.921 Einträge. Die bekannten Geldlesungen bleiben erhalten: 3,14 ziffernweise; 3,14 EUR, Euro oder € als drei Euro vierzehn. Originalprogramme und Originalmodule sind nicht enthalten.

## Prüfung

- 1.260 systematische Pausenkontexte und 720 ergänzende Kontexte stimmen im unabhängigen Ausklangplan mit den beobachteten Zeit- und Quellereignissen überein. Die ergänzenden Fälle führten zur Korrektur des m-Ausklangs und sind deshalb Entwicklungsfälle, kein unberührter Abschlusstest.
- 3.156 n/ng-Kontexte stimmen in den vollständigen AV/AH-Ereignissen des Lautkerns überein, einschließlich ausgelassener AV-Punkte. Diese Prüfung verwendet die tatsächlich im Original angekommenen Lautkontexte und Zeitgrenzen. Sie beweist keine vollständige Wortanalyse oder Gesamtsynthese.
- 25.794 erzeugte Großbuchstabenfälle stimmen in der öffentlichen Wort-/Buchstabierentscheidung überein. In einem zusätzlichen Feld mit 649 Einträgen stimmen sämtliche 646 tatsächlich aufgerufenen Original-Klassifikationen überein. Drei Einträge nehmen vorher einen Wörterbuch-/Zahlenpfad. Die vollständigen Lautgruppen steigen von 242 auf 638 passende Fälle, ohne Verlust zuvor passender Fälle.
- Im ersten ng-Wortfeld steigen die vollständigen passenden Aussprachen von 49 auf 66 bei 98 Wörtern. In 229 zusätzlich geprüften Wortformen steigen sie von 49 auf 158; kein zuvor vollständig passender Fall geht verloren. Dieses Feld enthält natürliche Wörter und systematisch gebildete Formen.
- 97 C-Testprogramme, ECI-API/ABI, 40 Sprachfälle, 36 Zahlenfälle und 15 Buchstabierprüfungen bestehen. Die Windows-x64-DLL besitzt die 19 erwarteten Exporte. Paket und NVDA-Treiber bestehen die Simulation.
- 37 vollständige Texte liefern im C-Textplan und über ECI bytegleiches PCM. Keine Übersteuerung; EUR, Euro und € erzeugen weiterhin identisches Audio.

Die 15 schon vorher verwendeten natürlichen Vergleichssätze zeigen 77 statt 93 ms mittlere absolute Abweichung der Gesamtdauer. Die aktive Sprachdauer bleibt bei 83 ms Abweichung. Die RMS-Abweichung liegt bei 0,424 statt 0,423 dB. Das sind beschreibende Messwerte und keine Aussage über einen Hördurchbruch.

## Hörvergleich

Datei: reloq-0.2.18-vergleich.wav. Pro Satz: Original mit normalem Text, 0.2.17, 0.2.18. Eine Sekunde trennt die Versionen, zwei Sekunden die Sätze. Keine Pegelnormalisierung.

1. Bitte komm, wir warten hier.
2. Die Sängerin singt, dann beginnt die nächste Schwingung.
3. Wir sehen den langen Gang, dann gehen wir weiter.

## Offen

Die allgemeine Morphologie, komplexe Zusammensetzungen, Lautdauer und zahlreiche Übergänge bleiben unvollständig. Beispiele sind Eingang/Eingänge, Längengrad, Klingelton und einige Ableitungen auf -end. Das gemeldete gelegentliche ng-zu-n wurde im ausgewählten Wortfeld nicht als einheitlicher Fehler bestätigt; gefunden wurden vor allem falsche Silbengrenzen, Vokallängen und Wortbestandteilgrenzen. Es gibt deshalb keine pauschale n-zu-ng-Ersetzung.

Die allgemeine Entstimmung von b/d/g/v/z/J wurde weiter untersucht, aber noch nicht als vollständige Regel eingebaut. Der ältere Fehler im separaten Test der wortweisen Tonhöhensteuerung bleibt offen. Unter echtem Windows/NVDA wurde dieser Stand hier nicht gehört.
