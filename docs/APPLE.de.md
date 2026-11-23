# ReLoq Apple: Installation auf iPhone, iPad und Mac

[English](APPLE.en.md)

[ReLoq Apple](https://github.com/Quinton1110/ReLoq-Apple) ist eine unabhängig betreute App, die ReLoq-C-Stimmen für VoiceOver und gesprochene Inhalte bereitstellt. Downloads und Updates kommen vom ReLoq-Apple-Projekt; sie gehören nicht zu den Installationsdateien von ReLoq C 2.0.

Vorausgesetzt werden iOS beziehungsweise iPadOS 18 oder neuer oder ein Mac mit Apple Silicon und macOS 15 oder neuer. Der angebotene Mac-Build unterstützt keine Intel-Macs. Die aktuelle Version und die Dateien stehen auf der [Release-Seite von ReLoq Apple](https://github.com/Quinton1110/ReLoq-Apple/releases/latest); das Projekt bietet auch eine [Download-Seite](https://quintonwilliams.me/reloq-apple/).

**Unterschied zwischen den Versionen:** Am 23. November 2026 ist ReLoq Apple 0.1.5 aktuell. Der darin enthaltene Sprachkern basiert auf ReLoq C 1.4.1 mit den Apple-Korrekturen, noch nicht auf ReLoq C 2.0. Aragonesisch ist daher dort noch nicht enthalten. Eine Installation von ReLoq C 2.0 auf einer anderen Plattform aktualisiert den Kern in der Apple-App nicht. Änderungen stehen in den Release-Hinweisen des Apple-Projekts.

## iPhone und iPad: AltStore Classic

Eine IPA-Datei lässt sich nicht einfach durch Öffnen in „Dateien“ installieren. Für diesen Weg werden AltStore Classic, ein Mac oder Windows-PC zur Einrichtung und ein Apple-Account benötigt.

1. Installiere AltServer auf dem Computer und damit AltStore Classic auf dem Gerät. Folge der offiziellen Anleitung für [Mac](https://faq.altstore.io/altstore-classic/how-to-install-altstore-macos) oder [Windows](https://faq.altstore.io/altstore-classic/how-to-install-altstore-windows), einschließlich der Bestätigungen zum Koppeln und Vertrauen.
2. Falls verlangt, aktiviere unter **Einstellungen → Datenschutz & Sicherheit → Entwicklermodus** den Entwicklermodus, starte das Gerät neu und bestätige die Nachfrage. Vertraue dem Entwicklerprofil unter **Einstellungen → Allgemein → VPN & Geräteverwaltung**, falls du dazu aufgefordert wirst.
3. Öffne AltStore, gehe zu **Sources**, wähle die Schaltfläche zum Hinzufügen und gib diese Quellenadresse ein:

   `https://quintonwilliams.me/reloq-apple/source.json`

4. Öffne die ReLoq-Apple-Quelle und installiere ReLoq Apple. Falls AltStore fragt, ob App-Erweiterungen beibehalten werden sollen, behalte sie: Die Sprachausgabe wird durch eine Erweiterung bereitgestellt.
5. Öffne ReLoq Apple einmal. Wähle anschließend unter **Einstellungen → Bedienungshilfen → VoiceOver → Sprache → Stimme** eine Stimme mit dem Zusatz **(ReLoq)**. Die Stimmen stehen auch für gesprochene Inhalte zur Verfügung.

Bei einem kostenlosen Apple-Account läuft die Signierung nach sieben Tagen ab, wenn sie nicht erneuert wird. AltStore kann Apps erneuern, während AltServer auf dem Computer läuft und das Gerät im selben WLAN oder per USB verbunden ist. Kontrolliere **My Apps** und verwende bei Bedarf **Refresh All**; die Erneuerung im Hintergrund ist nicht garantiert. Nach Ablauf stehen die Stimmen möglicherweise erst nach der Erneuerung wieder zur Verfügung. AltStore begrenzt außerdem die Zahl aktiver Apps und App-IDs; siehe die [Hinweise zu Erneuerung und Account](https://faq.altstore.io/altstore-classic/your-altstore).

Das Apple-Projekt beschreibt in seiner [Anleitung](https://github.com/Quinton1110/ReLoq-Apple#just-want-to-use-it) auch die Installation mit [SideStore](https://sidestore.io/) und [Sideloadly](https://sideloadly.io/). Die IPA dafür steht auf der Release-Seite.

## Mac

1. Lade auf einem Mac mit Apple Silicon die Datei mit der Endung `-mac.zip` von der [Release-Seite von ReLoq Apple](https://github.com/Quinton1110/ReLoq-Apple/releases/latest) herunter.
2. Entpacke die Datei, verschiebe **ReLoq Apple** nach **Programme** und öffne die App einmal. Lass sie in „Programme“ liegen: Ihre Spracherweiterung stellt die Stimmen bereit.
3. Öffne das **VoiceOver-Dienstprogramm → Sprache → Stimmen**, wähle die primäre Stimme und dann eine Stimme mit dem Zusatz **(ReLoq)**. Bei eingeschaltetem VoiceOver öffnet **VO+Fn+F8** das Dienstprogramm; VO bedeutet normalerweise Control und Option zusammen.

Stimmeinstellungen und das optionale Aussprachewörterbuch werden in ReLoq Apple verwaltet. Laut Anleitung der App übernimmt VoiceOver eine Änderung der Abtastrate nach einem VoiceOver-Neustart. Weitere Hinweise und Updates findest du in der [Anleitung von ReLoq Apple](https://github.com/Quinton1110/ReLoq-Apple#readme).
