"""Make a spoken comparison of private Aragonese acoustic refinements.

The candidate library is a disposable Castilian-based experiment, not an
installable voice. Environment settings are changed only between serial calls.
"""
from __future__ import annotations

import argparse
from array import array
import os
from pathlib import Path
import sys
import wave

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS, SAMPLE_RATE
from castilian_sh_listening import graft, millis

SH = 'RELOQ_ARAGON_PROBE_SH_PERCENT'
AU = 'RELOQ_ARAGON_PROBE_AU_PERCENT'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--refined', type=Path, required=True)
    parser.add_argument('--wav', type=Path, required=True)
    args = parser.parse_args()

    german = DirectEciEngine(args.baseline.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    castilian = DirectEciEngine(args.baseline.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
    mexican = DirectEciEngine(args.baseline.resolve(), dialect=0x20001, encoding='utf8', native_voice=True)
    refined = DirectEciEngine(args.refined.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
    engines = (german, castilian, mexican, refined)
    old_env = {key: os.environ.get(key) for key in (SH, AU)}
    samples = array('h')
    try:
        for engine in engines:
            engine.set_parameter(SAMPLE_RATE, 3)
            engine.set_parameter(PROCESS_TAGS, 1)

        def render(engine, text):
            pcm = bytearray()
            if not engine.speak([text], pcm.extend, lambda _index: None) or not pcm:
                raise RuntimeError('No audio for ' + text)
            out = array('h')
            out.frombytes(pcm)
            if sys.byteorder != 'little':
                out.byteswap()
            return out

        def add(audio):
            samples.extend(audio)
            samples.extend([0] * millis(300))

        def say(text):
            add(render(german, text))

        say('Aragonesisch. Neuer Hörvergleich. Zuerst das Sch.')
        os.environ[AU] = '0'
        examples = (('Erstes Beispiel.', '`[.1Sa.0da]', '`[.1Ca.0da]'),
                    ('Zweites Beispiel.', '`[.1Swe.0la]', '`[.1Cwe.0la]'))
        for label, source, carrier in examples:
            say(label + ' Der frühere Hörversuch.')
            add(graft(render(mexican, source), render(castilian, carrier), 30))
            say('Der bisherige native Versuch.')
            os.environ[SH] = '80'
            add(render(refined, source))
            say('Sch, Probe eins.')
            os.environ[SH] = '65'
            add(render(refined, source))
            say('Sch, Probe zwei.')
            os.environ[SH] = '50'
            add(render(refined, source))

        for label, source in (('Sch in der Wortmitte.', '`[.0a.1Si.0na]'),
                              ('Sch am Wortende.', '`[.1peS]')):
            say(label + ' Bisheriger nativer Versuch.')
            os.environ[SH] = '80'
            add(render(refined, source))
            say('Sch, Probe eins.')
            os.environ[SH] = '65'
            add(render(refined, source))
            say('Sch, Probe zwei.')
            os.environ[SH] = '50'
            add(render(refined, source))

        say('Jetzt das Au, in drei Wörtern. Die ursprüngliche Aussprache, dann der bisherige gerundete Versuch, und zwei neue Abstufungen.')
        os.environ[SH] = '80'
        for index, word in enumerate(('auca', 'pauto', 'cudiau'), 1):
            say('Wort ' + str(index) + '. Ursprünglich.')
            os.environ[AU] = '0'
            before = render(refined, word)
            assert before == render(castilian, word)
            add(before)
            say('Bisheriger gerundeter Versuch.')
            os.environ[AU] = '100'
            add(render(refined, word))
            say('Au, Probe eins.')
            os.environ[AU] = '40'
            add(render(refined, word))
            say('Au, Probe zwei.')
            os.environ[AU] = '65'
            add(render(refined, word))
    finally:
        for engine in engines:
            engine.close()
        for key, value in old_env.items():
            if value is None:
                os.environ.pop(key, None)
            else:
                os.environ[key] = value

    if sys.byteorder != 'little':
        samples.byteswap()
    with wave.open(str(args.wav), 'wb') as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(16000)
        wav.writeframes(samples.tobytes())
    print(f'Created {args.wav}: {len(samples) / 16000:.1f} seconds')


if __name__ == '__main__':
    main()
