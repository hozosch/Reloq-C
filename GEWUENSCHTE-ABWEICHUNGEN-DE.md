# Gewünschte Abweichungen und Erhaltungsvertrag

Stand 0.2.183: Der eigene deutsche Akustikplaner bleibt auf ausdrücklichen Wunsch erhalten. Die weitere Originalangleichung betrifft Satzanalyse, Zeichenverarbeitung und ECI-Steuerung. Eine vollständige Ablösung des Akustikplaners ist nicht mehr das aktuelle Ziel. Die folgenden Klang- und Ausspracheentscheidungen bleiben verbindlich.

Die Festlegungen wurden aus dem aktuellen Code, den Versionshinweisen und den früheren Prüffällen zusammengeführt. Später ersetzte Klangabstimmungen werden nicht wieder aktiviert. Die Liste unterscheidet die gewünschten Abweichungen und die zusätzlich zu erhaltenden Funktionskorrekturen.

| Bereich | Zu erhaltendes Verhalten | Beispiele |
| --- | --- | --- |
| consonantal_r | Konsonantisches r mit bisherigem Anregungs- und Resonanzcharakter | Raten, Rot, Richtig |
| final_a | Helleres unbetontes finales a nur am Ende der gesamten Äußerung, mit mittlerer Dauer | Amerika., Amerika ist weit., Amerika. Amerika ist weit. |
| nasal_a | Angenommener Restaurant-Nasal: F1/F2/F3 624/960/2880, B1 180, FNP/FNZ 280/350 | Restaurant, Restaurants, Chance |
| long_oe_r | Vokalisches r nach langem ö: angenommene Färbung, ohne zusätzliche Quellenabsenkung (ab 0.2.199) | hören, Hörbücher |
| ier_colour | i.e.r wie Barriere/Karriere behält frühere eigene Färbung; vorherig folgt Original | Barriere, Karriere, vorherig |
| post_e_j | Homepage-j: ein Drittel vom Original in Richtung unbetontes Germanistik-i, ohne schmalere Bandbreiten | Homepage, Germanistik |
| s_before_m_n | Normales s statt lispelnder Originalvariante vor m/n | Smile, Snob |
| s_z_balance | s/z erhalten 4 dB mehr AF, periodische Anregung unverändert | Sonne, Wasser, Reise, Zeit |
| medial_ts | Wortinneres ts in beiden Komponenten 1 dB unter angenommenem An-/Auslaut | Zeit, Katze, Satz |
| other_consonants | Übrige Konsonanten +1 dB, s/z/ts und vokalisches r ausgenommen | Paket, Raten, Faden, Laden |
| labiodental_gain | f/v/w-Pfad behält angenommene Anhebung bei hohen Raten | Faden, Wasser, Vase |
| wideband | Präsenz 3,5–4 kHz ca. 2,5 dB, Bass, s-Bandbreite und alle sieben Raten behalten | Stimme bei 8 bis 48 kHz |
| question_contour | Alle deutschen Fragen steigen spät an; frühere Wortakzente bleiben erhalten | Wie geht es dir?, Wer?, Kommst du morgen? |
| german_symbol_names | Grad-Zeichen als Grad und Unterstrich als Unterstrich, direkt in der Zeichennamenregel; Ringakzente bleiben Ringel | °, _, Å, å |
| short_u_plus | plus und Pluszeichen mit kurzem U | plus, +, ± |
| voiced_samt | samt mit stimmhaftem s | samt, Samt |
| nominal_prefix | Gewünschte Unter-/Über-Substantivbetonung, keine pauschale Verbänderung | Unterricht, Überschrift, Übersicht, Übergabe, Übernehmen, überzeugt |
| konzentrier_stem | Korrigierter konzentrier-Stamm mit Beugungen und Zusammensetzungen | konzentrieren, konzentriert, unkonzentriert |
| unicode | Unicode-Namen und Akzente nur im passenden Modus; vorhandene native Zeichentabellen werden erweitert | ā, α, 😀, Frankfurt/Main, Frankfurt/Oder |
| pause_queue | Pausen intern verkürzen, keine p1-Tags; Warteschlange und Abbruch/Index-Reihenfolge erhalten | Hallo Welt., Hallo, Welt. |
| models | Weibliche Impulsphase, kleine Kopfgrößen, Behauchtheit und hohe Tonhöhen erhalten | Kopfgrößen 22/29, Tonhöhe 98/100, alle Raten |
| decimal_digits | Gewöhnliche Nachkommastellen ziffernweise lesen; Euro-/Cent-Beträge behalten ihre eigenen Mengenregeln | 3,14, 0,05, 3,14 Euro |
| os_adjective | Vorhandene Klasse famos/rigoros einschließlich Beugung mit Endbetonung erhalten; Infos bleibt unverändert | famos, famose, rigoros, rigorosen, Infos |
| compound_readings | Vollständiger Stamm und zugelassener Substantivkopf in Sprach-/Such-Komposita; stammabschließendes ch erhalten | Sprachauswahl, Suchauswahl, Sprachauswahlen, Suchauswahlen |
| continuation_melody | Abgestimmte Konturen für Nachträge, Auslassungspunkte und z. B. nach Komma erhalten | Das denke ich, denke ich., Das ..., z. B. hier. |
| eci_tags | Vorhandene Tag-Modi und Steuerbefehle erhalten; bei deaktivierten ECI-Tags wird der Backtick als Zeichen gelesen | `ts1 abc, `ts2 abc, `0 bis `9, `v1 bis `v9, `l4, `l1.1, `vb80, `p250, Gravis bei ausgeschalteten Tags |
| eightk_sibilants | Die Korrektur des s/z-Ausfalls im dreipoligen 8-kHz-Profil und die abgestimmten kleinen/weiblichen Modelle erhalten | s und z bei 8 kHz und kleinen Kopfgrößen |

## Weitere Sprachen und Oberfläche

- Auch US-Englisch behält die gewünschte aufsteigende Endkontur bei allen erkannten Frageklassen, einschließlich W- und Alternativfragen; bei aktivierter Melodie setzt sie erst am Frageende ein. Belege und ursprüngliche Kontrollen: ARBEITSSTAND-0.2.111-DE.md und reference-apple/behavior-0.2.111.
- Chinesisch behält die kontextbezogene englische Lautanalyse, flüssige Wortgruppen, lokale Intonation 0 und die chinesische Tonhöhensteuerung. Pinyin und interne Fremdwortzuordnung bleiben im chinesischen Kern. Die beiden chinesischen Varianten haben eine gemeinsame NVDA-Auswahl.
- Japanisch verwendet für japanische Texte die ursprünglichen Eloquence-Regeln; die Behandlung lateinischer Zeichen folgt der vereinbarten OpenEVV-Art. Finnisches ü wird innerhalb der Regel als y behandelt.
- Griechisch behält seine Zeichennamen und das spanische Zeichenverhalten, die abgestimmten Chi-/J- und P-Pegel, die soy-artige Lautfarbe in voice sowie die korrekten Zahlenlesungen.
- Sprach- und Stimmennamen sowie Reihenfolge folgen Eloquence_64; Griechisch steht am Ende. Die vorhandene NVDA-Spracheinstellung wird verwendet.

## Technische Festlegungen

- Eigenständiges kompaktes C bleibt das Ziel. Kein Laufzeit-Bytecode, keine Original-SYN/DLLs oder Emulation im Add-on. Die vollständige manuelle Neufassung aller generierten C-Regeln ist weiterhin nicht abgeschlossen.
- Keine Wiederholungs-, Wortausgabe- oder PCM-Ergebniscaches. UTF-8 und die Zeichen-/Akzenttabellen liegen im C-Kern. Interne Pausenkürzung verwendet keine zusätzlich eingefügten p1-Tags.
- Warteschlange, Indizes und Abbruch behalten ihre bestehenden Regeln. Hochratenprofile, weibliche Impulsphase und Behauchtheit bleiben erhalten.
- Alle historischen Archivnamen und exakten unkomprimierten Inhalte bleiben im vollständigen Quellpaket erhalten; Größenlimit 512 MiB. Audio wird auch bei Prüfungen ausschließlich im RAM verarbeitet.

## Prüfstand

Der aktive deutsche Planer behält finales a samt Dauer, Präfix-/Wortbildungsregeln, ziffernweise Nachkommalesung und das späte Frageende. Die Tag-Behandlung wird an der Satzanalyse ergänzt: ausdrückliche Betonung bleibt erhalten; Stimmeneinstellungen unterbrechen die Satzanalyse nicht mehr. Der Arbeitsentwurf unter reference-apple/behavior-0.2.181/development/native-german-pending181 wird vom ausgelieferten Sprachpfad nicht verwendet. Die früheren 28 Aussprache-Prüffälle liegen unter reference-apple/behavior-0.2.104/check_user104.py.
