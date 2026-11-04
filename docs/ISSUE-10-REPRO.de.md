# Dateinamen-Test für Issue #10

Die Meldung betrifft Version 1.0.1 beim schnellen Navigieren in xplorer2.
Der Fehler wurde inzwischen mit dem automatischen Windows-Stresstest bei
48 kHz nachgestellt. Die folgenden Schritte schaffen eine ähnliche Situation
in xplorer2; sie garantieren keine Reproduktion auf jedem Gerät.

1. Das Skript `tools/repro_issue_10.ps1` in PowerShell ausführen. Es erstellt
   einen neuen Ordner im temporären Verzeichnis mit 60 langen Dateinamen und
   gibt dessen Pfad aus. Es löscht keine vorhandenen Dateien.
2. Den ausgegebenen Ordner in xplorer2 öffnen. Wenn möglich in der Detailansicht
   Dateiname, Erweiterung, Größe, Attribute und Änderungsdatum anzeigen lassen.
3. NVDA mit Eloquence C aktivieren. Den NVDA-Protokollpegel auf „Debugwarnung“
   oder „Debug“ setzen und wiederholt schnell mit Pfeil auf und Pfeil ab durch
   die Liste gehen. Auch bei laufender Sprachausgabe weiter navigieren.
4. Falls Eloquence C ausfällt, **NVDA+F1** für das Protokoll drücken und den
   Fehler mit Uhrzeit, NVDA- und Add-on-Version, Sprache, Stimme, Abtastrate
   sowie Ansicht und Spalten in xplorer2 festhalten. Prüfen, ob ein Neustart
   von NVDA die Sprachausgabe wiederherstellt.

Anschließend kann der ausgegebene Testordner von Hand gelöscht werden. Ein
ausbleibender Fehler bestätigt nicht, dass das Problem behoben ist. Die Tests
im Projekt simulieren zusätzlich viele Abbrüche und prüfen Folgeausgaben auf
demselben Sprachworker, ersetzen aber keinen Test im echten xplorer2.
