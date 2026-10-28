# Eloquence C 0.2.8 – experimentelles NVDA-Test-Add-on

Die auswählbare deutsche x64-Stimme enthält weiterhin den unabhängigen C-Kern bei 11.025 Hz. Warteschlange, Chunk-Pausen, Abbrechen und Großbuchstaben-Tonhöhe bleiben erhalten.

## Hörbare Änderung

- Eine allgemeine Regel setzt 14 trennbare Partikeln mit eingeschobenem `zu` und einem sicher analysierten Infinitiv zusammen. Ein Graphem- und Phonemabgleich schützt unter anderem den Diphthong `eu/äu` vor einer falschen Zerlegung. Auf 2.840 frischen, zuvor nicht im Lexikon stehenden Formen gab es drei vollständige Referenztreffer mehr und keine beobachtete phonetische Regression. Über alle 13.280 untersuchten Formen stimmten 29 durch die jüngste Erweiterung geänderte Aussprachen mit der öffentlichen Apple-Ausgabe überein, auch im Satz.
- Das produktive Suffix `-los` erhält im freien deutschen Aussprachestrang einen langen Endvokal. In 2.960 weiteren frischen, lexikonfreien Wörtern änderten sich zwölf Lesungen; sechs ganze Wortgruppen wurden referenzgleich, alle zwölf Phonemfolgen rückten näher an die Referenz, ohne beobachtete Regression. In acht natürlichen Sätzen hatte das Suffix nun achtmal die passende Vokallänge; fünf Zielwörter stimmten vollständig.

## Grundlegung für die weitere Synthese

- Vokaltrakt, Stimmquelle und Rauschquellen werden im generischen Satzpfad separat geplant. Die reine Umstellung und die zusätzliche Zielpunktreserve ließen 40 Vergleichsausgaben samplegenau unverändert; sie erlauben künftige gekoppelte Übergangsregeln.
- Ein vorhandener `0`- oder `00`-Kontrollmarker vor einer lexikalisch betonten Silbe bekommt den passenden tieferen F0-Verlauf. Der freie Textweg erzeugt solche Marker noch nicht zuverlässig. Deshalb ist dies im normalen NVDA-Text derzeit kaum hörbar.
- Ein neuer Korpus mit unabhängigen Wortfamilien und natürlichen Sätzen prüft Konsonantenübergänge gegen die öffentlichen Apple-ECI-Rückrufe. Die bisher getesteten akustischen Kandidaten wurden wegen einzelner Verschlechterungen nicht ausgeliefert.

Die allgemeine Stimme bleibt akustisch experimentell. Das Paket ist ein neuer Aussprache- und Architekturstand; es behebt die breiten Konsonantenübergänge und Lautausklänge noch nicht.
