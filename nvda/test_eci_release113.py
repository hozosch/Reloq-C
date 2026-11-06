"""Regression tests for long speech, native URL handling, and Greek English."""
import argparse
from pathlib import Path
from _reloq_eci_direct import DirectEciEngine, EciIndex, SAMPLE_RATE, PROCESS_TAGS
from test_eci_german_clock_native import phonemes

# NVDA 2026.3 symbol processing of the report in issue 19, with slash
# preservation set to never, at the none and some symbol levels.
ISSUE19_NONE = 'https:  www change org p ai-data-centre-72mw-at-thames-valley-park-reading-stop-construction signed-petition-home activityId 35563811 activityType starter cs tk A2O3GeBaf9fNSvi39WoAAHicyyvNyQEABF8BvDNmYzM3YTI2YWNhYWI5OTE1ZTI3NjQyMzMyMzZkZDljYjVkZWJkOWY1M2U0NThmZDYzOTU1M2Q4ZWIzMTdhMzk 3D utm campaign 591080b3c0de404881f759784d1d4b39 utm content sunrise v1 2 3 utm medium email utm source petition update utm term cs'
ISSUE19_SOME = 'https: slash  slash www dot change dot org slash p slash ai-data-centre-72mw-at-thames-valley-park-reading-stop-construction slash signed-petition-home activityId equals 35563811 and activityType equals starter and cs tk equals A2O3GeBaf9fNSvi39WoAAHicyyvNyQEABF8BvDNmYzM3YTI2YWNhYWI5OTE1ZTI3NjQyMzMyMzZkZDljYjVkZWJkOWY1M2U0NThmZDYzOTU1M2Q4ZWIzMTdhMzk percent 3D and utm campaign equals 591080b3c0de404881f759784d1d4b39 and utm content equals sunrise v1 2 3 and utm medium equals email and utm source equals petition update and utm term equals cs'
ISSUE19_URL = 'https://www.change.org/p/ai-data-centre-72mw-at-thames-valley-park-reading-stop-construction/signed-petition-home?activityId=35563811&activityType=starter&cs_tk=A2O3GeBaf9fNSvi39WoAAHicyyvNyQEABF8BvDNmYzM3YTI2YWNhYWI5OTE1ZTI3NjQyMzMyMzZkZDljYjVkZWJkOWY1M2U0NThmZDYzOTU1M2Q4ZWIzMTdhMzk%3D&utm_campaign=591080b3c0de404881f759784d1d4b39&utm_content=sunrise_v1_2_3&utm_medium=email&utm_source=petition_update&utm_term=cs'

# Each of these language modules implements the native HTTP URL rule.
URL_LANGUAGES = (0x10000, 0x10001, 0x20000, 0x20001, 0x30000,
                 0x30001, 0x40000, 0x50000, 0x60000, 0x60001,
                 0x70000, 0x90000, 0xa0000)

def capture(engine, parts):
    audio = bytearray()
    marks = []
    assert engine.speak([*parts, EciIndex(19)], audio.extend,
                        lambda value: marks.append((value, len(audio))))
    assert audio and marks == [(19, len(audio))], marks
    return bytes(audio)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    for language in (0x10000, 0x10001):
        engine = DirectEciEngine(args.library.resolve(), dialect=language,
                                 encoding='utf8', native_voice=True)
        try:
            engine.set_parameter(SAMPLE_RATE, 1)
            for text in (ISSUE19_NONE, ISSUE19_SOME, ISSUE19_URL):
                capture(engine, [text])
                capture(engine, ['Test after the long address.'])
            # Fragmentation by the caller must not introduce a new pause.
            text = ISSUE19_NONE
            assert capture(engine, [text]) == capture(engine, [text[:180], text[180:]])
        finally:
            engine.close()
    for language in URL_LANGUAGES:
        engine = DirectEciEngine(args.library.resolve(), dialect=language,
                                 encoding='utf8', native_voice=True)
        # The previous build did not correctly initialize the Taiwan English
        # phone table after mainland Chinese; do not reproduce that fault in
        # this optional baseline comparison.
        old = (DirectEciEngine(args.baseline.resolve(), dialect=language,
                               encoding='utf8', native_voice=True)
               if args.baseline and language != 0x60001 else None)
        try:
            engine.set_parameter(PROCESS_TAGS, 1)
            if old:
                old.set_parameter(PROCESS_TAGS, 1)
            for host in ('www.example.org/test', 'example.net/path-42'):
                http = phonemes(engine, 'http://' + host)
                https = phonemes(engine, 'https://' + host)
                assert '...' in http and '...' in https, (hex(language), http, https)
                assert http.split('...', 1)[1] == https.split('...', 1)[1], (hex(language), http, https)
                if old:
                    assert http == phonemes(old, 'http://' + host), (hex(language), host)
                    assert phonemes(engine, 'httpx://' + host) == phonemes(old, 'httpx://' + host)
        finally:
            engine.close()
            if old: old.close()
    greek = DirectEciEngine(args.library.resolve(), dialect=0x120000,
                            encoding='utf8', native_voice=True)
    try:
        greek.set_parameter(PROCESS_TAGS, 1)
        expected = {'go':'[.1gow]', 'Google':'[.1gu.0gel]', 'again':'[.0a.1gen]',
                    'together':'[.0to.1ge.0der]', 'message':'[.1me.0sidz]',
                    'language':'[.1lan.0gwidz]', 'comfortable':'[.1kamf.0ter.0bal]',
                    'wanted':'[.1wan.0tid]',
                    'audacity':'[.0o.1da.0si.0ti]',
                    'digital':'[.1did.0zi.0tal]',
                    'identity':'[.0ay.1den.0ti.0ti]',
                    'editor':'[.1e.0di.0ter]',
                    'Nevada':'[.0ne.1ba.0da]',
                    'cat':'[.1kat]', 'date':'[.1deyt]'}
        for word, expected_phones in expected.items():
            result = phonemes(greek, word)
            assert expected_phones in result, (word, result)
            capture(greek, [word])
    finally:
        greek.close()
    print('1.1.3: issue 19, native HTTP/HTTPS, Greek English and recovery passed')

if __name__ == '__main__':
    main()
