"""Release integration: plain text, mid-token indices and NVDA C transport."""
from pathlib import Path
import sys,tempfile
from _reloq_eci_direct import DirectEciEngine,EciIndex,GENERIC_RATE,GENERIC_VOLUME
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'addon'))
from build_synth import merged_german_lexicon

def main():
    texts=["Gesagt.","Ich habe es gesagt.","Sie hat es mir gestern gesagt.",
        "Gefragt, gelebt und geregt.","Veränderung und Verarbeitung.",
        "Das ist unauffällig.","Zwei Eier und ein Apfel.","Die ist gut.",
        "Ein alter und ein neuer Anfang.","Auch ich gehe heute nach Hause.",
        "Sechs Menschen sprechen leise.","Ich möchte das richtig aussprechen.",
        "Die Garage ist offen.","Eine Liste mit gesicherten Informationen.",
        "Tempo und Konsonantenkluster.","Hören und höher.","Hans' Haus.",
        "Wie geht’s?", "Ge'sagt.","' Gesagt '","Gesagt\u00ad und getan.",
        "Gesagt\u00a0und getan.","8:05 Uhr, 8:00 Uhr und 08:05:30 Uhr.",
        "1 234 und 1.234 Euro.","3,14 EUR und 3,14 €.","A, B und C."]
    texts += ["Chance und Chancen.", "Restaurant und Restaurants.",
        "Im Restaurant haben wir eine Chance.", "Ein neuer Anfang und ein langer Abend.",
        "Frische Luft, frischer Fisch und frisches Wasser.",
        "Die frischeren Farben auf den Tischen.", "Ein roter Stuhl neben einer Lampe.",
        "Das klingt richtig und flüssig.", "Eine kleine, klare und ruhige Stimme.",
        "Mehrere verschiedene Lautübergänge.", "Wir hören höhere Töne.",
        "Nach langen Worten folgen neue Namen."]
    texts += ["Haus-Tür und Auto-Bahn.", "U-Bahn-Tür und USB-C-Kabel.",
        "Nord-Ost-Bahn und Süd-West-Wind.", "Ein- und Ausgang.",
        "Die E-Mail-Adresse stimmt.", "Hans-Peter liest Karl-Heinz vor.",
        "Haus - Tür, Haus--Tür und Haus–Tür.", "A-B-C und 1-2-3.",
        "Eine Ampel, ein Antrag und ein Antrieb.",
        "Abbruch, Abdruck, Abgleich und Abfahrt.",
        "Anblick, Handlung, Entlastung und Aufbruch.",
        "Ausnahmen, Aufnahme und Ausflug.",
        "Es bleibt klar, glaubhaft und verständlich.",
        "Kraft, Druck, Platz, Sprache und Strömung.",
        "Atmen, Ordnung, Zeichnung und Rechnung.",
        "Offene Türen und warme Räume.",
        "Abnehmen, ablegen, Anlauf und Anmeldung.",
        "Die Äpfel liegen neben den Lampen."]
    texts += ["Er gibt es ab, sie vergibt und er bewirbt sich.",
        "Bewerfen, verwerfen und entwerfen.",
        "Gebraten, verordnet und entsprechend.",
        "Haken, hakte und hakten.",
        "Schmidt wohnt in der Stadt.",
        "Sie lebt, lebte und sagte es.",
        "Impfung, sanft, langsam und anfordern.",
        "Abfall, Abfahrt, Abschnitt und abholen.",
        "Kämpfen, dampfen und sinnvoll planen.",
        "Liste, Tempo und gesichert.",
        "Behörden, Veränderung und Verarbeitung.",
        "Wir hören höhere Töne im Restaurant."]
    texts += ["Abbruch und Abdruck, Abfall und Anlauf.",
        "Die frischeren Farben entsprechen den erwarteten Werten.",
        "Entsprechend den laufenden Änderungen wird gespeichert.",
        "Gebende, vergebende und entgebende Formen.",
        "Am Abend kommt ein Name nach dem anderen.",
        "Dem Namen folgen lange und kurze Laute.",
        "Pakt, Takt, hektisch und skeptisch.",
        "Der Kopf bleibt ruhig, und das Bild ist klar."]
    texts += ["Punkt, Punkte und Pünktlichkeit.", "Ein Punkt auf dem Desktop.",
        "Kontakte, Akten, Skripte und Objekte.", "Kampf, kämpfen und gekämpft.",
        "Tanzen, tanzt, Tänzer und Münzen.", "Bank, Klang, singen und denken.",
        "Nehmen und nennen, Namen und Männer.", "Abklemmen, abtippen und Kontakt.",
        "Eine lange Pause. Danach geht es weiter.", "Gesagt, gespeichert und geschlossen."]
    texts += ["Beachten, beobachten, beenden und überall.",
        "Am Abend wird es wieder wärmer.", "Schweben, schwimmen, schwach und schwer.",
        "Eine bequeme Quelle für frisches Wasser.", "Entwerfen, verwerfen und weiterarbeiten.",
        "Zwei offene Augen und ein neuer Anfang.", "Hören und gehören, Chance und Restaurant."]
    texts += ["Ausführen Dialogfeld", "Ausführen. Dialogfeld", "Ausführen, Dialogfeld",
        "Namen Dialogfeld", "Diesen Dialog öffnen", "Dialogfeld öffnen"]
    texts += ["Keine, keinen, kein und kleine.", "Teil, teilen, Teile und Eile.",
        "Ein Stein an einer Leine.", "Feile, Weile, Seil und Meile."]
    texts += ["81, 88, 181, 8081 und 8181.", "Version, Vision und Perspektive.",
        "Add-On, Add-ons und Check-out.", "Behandeln, behandelt, Behandlung und Behandlungen.",
        "Entwickeln, entwickelte, Entwicklung und Entwickler.",
        "Wickeln, Wicklung und Wickler.", "Handeln, Handlung, Händler und Handel.",
        "Erinnern, erinnerte, Erinnerung und Erinnerungen.",
        "Bitte einmal und auf einmal.", "Eine erste und eine andere Antwort.",
        "Nein. Name. N.", "Ende. Nein. Ende, Namen.",
        "Acht, acht Uhr, Nacht, Nächte und richtig.",
        "8:00, 8:05, 08:05:03 und 8:5:3 Uhr.",
        "81 000, 81.000, 3,14 Euro, 3,14 EUR und 3,14 €."]
    texts += ["Ausführen Knopf. Ausführen Platz. Ausführen Name.",
        "Ausführen. Name. Namen, Name. Einen. Name.",
        "Knopf, Knie, Knabe, Knecht, Knödel und Knoten.",
        "In Taste, in Anzeige und in Uhr."]
    texts += ["Samstag 10 Oktober 2026", "Ausschalten, ausstellen und aussprechen.",
        "Projekt, Projekte, Koje, Familie und bejahen.",
        "Grundlegend und grundlegende Veränderungen.",
        "Verzeihend, ausschaltend, zerlegend und bejahend.",
        "Die größten Konsonanten und die kleinsten Projekte.",
        "80, 81, 82, 83, 84, 85, 86, 87, 88 und 89.",
        "180, 181, 287, 1088, 81 000, 81.000 und 8081."]
    texts += ["1. Eins. Ein neuer Anfang.", "25, 26, 35, 36, 85, 86, 95 und 96.",
        "Fünfundzwanzig und sechsundzwanzig.", "Fünfundachtzig und sechsundachtzig.",
        "Ja. Mann. Name. Morgen. Neu. Mutter. Nehmen.",
        "Ende. Mehr. Nein. Namen. Nun. Müssen.", "Ja? Name. Ja! Mann.",
        "Ich bin es. Ich bin hier. Ich habe es. Ich habe das.",
        "Ich schalte den Computer aus. Wir schalten das Licht aus.",
        "Das Projekt wurde erfolgreich abgeschlossen.",
        "Er kennt sie und sieht ihn. Ich sehe Peter und Maria.",
        "Hektik, Achtung, Nacht, Theater und skeptisch."]
    texts += ["Mann. Name. Mutter. Nehmen.", "Warm. Arm. Schwarm. Halm.",
        "Lehrer. Computer. Neuer Lehrer.", "Haus. Name. Fertig. Name.",
        "Behalten, erhalten, Verhalten und verhindern.",
        "Hierher, vorher, Freiheit, Sicherheit und Mehrheit.",
        "Ich höre höher. Chance und Restaurant.",
        "Vierundzwanzig, vierundachtzig, 24 und 84.",
        "Fünfundzwanzig, sechsundzwanzig, 25 und 26.",
        "Fünfundachtzig, sechsundachtzig, 85 und 86.",
        "Fünfund, sechsund und vierund.",
        "Ende. Heute haben wir frei. Er hat es gesagt."]
    texts += ["Aktualisieren und Ausführen von Recheck-Skripten",
        "aktualisieren, aktualisiert, aktualisiere, qualifizieren.",
        "Skripten, Skripte, Skizze, Skonto und Recheck-Skripten.",
        "Theater, Ruine, ruinieren, ui und ea.",
        "Mann. Ein Anfang. Haben. Ein Anfang."]
    texts += ["Mithelfen, mithören und mithalten.",
        "Rathaus, Stadthalle und Stadthaus.",
        "Enthaaren, enthüllen, enthalten und Enthaltung.",
        "Hat Hilfe, mit Hilfe, mit Hans und geht hin.",
        "Geht heim, fährt heute und ein Hut hilft.",
        "Das hat heute geholfen.", "Hat. Hilfe. Geht. Heute.",
        "Hut, Hilfe. Hut; Hilfe. Hut: Hilfe.", "Theater, Methode und Mathilde."]
    checks=0
    with tempfile.TemporaryDirectory() as tmp:
        lex=Path(tmp)/'german.rlex';lex.write_bytes(merged_german_lexicon())
        engine=DirectEciEngine(ROOT/'eci/libreloq_eci.so',experimental_generic=True,lexicon_path=lex)
        try:
            for rate,volume in [(50,100),(80,100),(50,0)]:
                engine.set_parameter(GENERIC_RATE,rate);engine.set_parameter(GENERIC_VOLUME,volume)
                for text in texts:
                    a,b,indices=[],[],[]
                    assert engine.speak([text],a.append,lambda _:None),text
                    cut=min(2,len(text))
                    assert engine.speak([text[:cut],EciIndex(31),text[cut:],EciIndex(32)],b.append,indices.append),text
                    pcm=b''.join(a)
                    assert pcm and pcm==b''.join(b),(text,'indexed PCM')
                    assert indices==[31,32],(text,indices)
                    assert (not any(pcm)) if volume==0 else any(pcm),text
                    checks+=1
        finally:engine.close()
    print(f'{checks} release ECI cases: plain/indexed PCM, mid-token indices, fast speech and mute passed')
if __name__=='__main__':main()
