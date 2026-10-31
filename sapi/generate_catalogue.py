"""Generate SAPI token metadata from the existing shared voice catalogue."""
from pathlib import Path
import argparse
import json
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_languages import LANGUAGES, LANGUAGE_DISPLAY_ORDER
from _reloq_eci_direct import SAMPLE_RATES, GENDER, HEAD_SIZE, NATIVE_PITCH, NATIVE_INFLECTION, ROUGHNESS, BREATHINESS, NATIVE_VOLUME

LCIDS = {'de_DE': 0x407, 'en_US': 0x409, 'en_GB': 0x809, 'es_ES': 0xc0a,
         'es_MX': 0x80a, 'fr_FR': 0x40c, 'fr_CA': 0xc0c, 'it_IT': 0x410,
         'pt_BR': 0x416, 'fi_FI': 0x40b, 'zh_CN': 0x804, 'ja_JP': 0x411,
         'ko_KR': 0x412, 'el_GR': 0x408}


def generate():
    java = (ROOT / 'android/java/org/reloq/tts/preview/Catalog.java').read_text()
    samples = {int(i, 16): json.loads('"' + sample + '"') for i, sample in
               re.findall(r'new Language\((0x[0-9a-f]+),"[^"]+","[^"]+","([^"]+)"\)', java)}
    wide = lambda value: 'L' + json.dumps(value, ensure_ascii=False)
    lines = ['/* Generated from the shared NVDA catalogue; one SAPI voice per language. */',
             'static const struct sapi_language sapi_languages[] = {']
    for ident in LANGUAGE_DISPLAY_ORDER:
        language = LANGUAGES[ident]
        lines.append('    {0x%x,0x%x,%s,%s,%s},' % (ident, LCIDS[language.locale],
                     wide(language.name), wide(language.locale), wide(samples[ident])))
    profiles = LANGUAGES[0x40000].voices
    anatomy = []
    for profile in profiles:
        parameters = dict(profile.parameters)
        assert parameters[NATIVE_VOLUME] == 100
        anatomy.append('{' + ','.join(str(parameters[key]) for key in
                       (GENDER, HEAD_SIZE, NATIVE_PITCH, NATIVE_INFLECTION, ROUGHNESS, BREATHINESS)) + '}')
    lines += ['};', 'static const int sapi_rates[]={' + ','.join(str(SAMPLE_RATES[i]) for i in range(7)) + '};',
              'static const WCHAR *const sapi_profiles[]={' + ','.join(wide(p.name) for p in profiles) + '};',
              'static const int sapi_anatomy[8][6]={' + ','.join(anatomy) + '};']
    return '\n'.join(lines) + '\n'


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    target = Path(__file__).with_name('catalogue.inc')
    content = generate()
    if args.check:
        assert target.read_text() == content, 'SAPI catalogue is stale'
    else:
        target.write_text(content)
