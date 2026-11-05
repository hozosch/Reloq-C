"""Mixed Mandarin/English, long NVDA example, native controls and indices."""
from pathlib import Path
import re
from _reloq_eci_direct import (DirectEciEngine,EciIndex,NATIVE_INFLECTION,
    NATIVE_SPEED,GENDER,SAMPLE_RATE,PAUSE_MODE,PROCESS_TAGS,EciLanguage)
from _reloq_languages import LanguageEngine,CHINESE_CHINA,US_ENGLISH
from test_eci_german_clock_native import phonemes,render

ROOT=Path(__file__).resolve().parents[1]


def chinese_accents_from_phonemes(phones,limit=None):
    groups=re.findall(r'\[\+([^]]+)\]',phones)
    if limit is not None:groups=groups[:limit]
    result=[tuple(map(int,pair)) for group in groups for pair in re.findall(r'\.([0-4])([0-4])',group)]
    assert result,(phones,groups)
    return result


def chinese_syllable_accents(engine,text):
    """Capture Mandarin lexical tones and stress, excluding English banks."""
    return chinese_accents_from_phonemes(phonemes(engine,text))


def main():
    text=(ROOT/'tests/fixtures/chinese_nvda_about.txt').read_text(encoding='utf8')
    # Produced by the NVDA 2026.3 Chinese/English symbol dictionaries at
    # level NONE, then passed through NVDA's normal whitespace processing.
    symbol_none=(ROOT/'tests/fixtures/chinese_nvda_about_symbols_0.txt').read_text(encoding='utf8').strip()
    # NVDA 2026.3 at Chinese symbol level SOME supplies these spoken symbol
    # names to the synthesizer between the English words and numbers.
    symbol_text=(ROOT/'tests/fixtures/chinese_nvda_about_symbols.txt').read_text(encoding='utf8')
    assert 'https: 斜杠  斜杠 www 点 nvaccess 点 org' in symbol_text
    assert 'https:  www nvaccess org Copyright (C) 2006 2026' in symbol_none
    assert 'Version: 2026.3 (2026.3.0.58174)' in text
    checks=0
    for dialect in (0x60000,0x60001):
        engine=DirectEciEngine(ROOT/'eci/libreloq_eci.so',dialect=dialect,encoding='utf8',native_voice=True)
        try:
            engine.set_parameter(SAMPLE_RATE,1)
            # NVDA 2026.3 symbol level NONE removes URL slashes, dots and
            # the year dash. A long English bank followed by a second bank
            # used to invalidate the native word/phrase cursors and leave
            # the following Mandarin syllables without their normal stress.
            engine.set_parameter(NATIVE_INFLECTION,47)
            for prefix,suffix in (
                ('https:  www nvaccess org ','Copyright (C) 2006 2026'),
                ('a a a a ','Copyright (C) 2006 2026'),
                ('a a a a ','Copyright (C) 你好'),
                ('alpha beta gamma delta ','Copyright (C) 42'),
                ('hello world this is a longer string of english text ','Copyright (C) 你好 2006 2026'),
            ):
                expected=chinese_syllable_accents(engine,suffix)
                actual=chinese_syllable_accents(engine,prefix+suffix)
                assert actual==expected,(hex(dialect),prefix,suffix,expected,actual)
                checks+=1
            # Also verify the exact complete About text, including the
            # preceding version and URL, not merely the reduced example.
            full_phones=phonemes(engine,symbol_none)
            copyright_at=full_phones.find('[.1ka.0pi.2rayt]')
            c_at=full_phones.find('[.1si]',copyright_at)
            assert copyright_at>=0 and c_at>copyright_at
            expected=chinese_syllable_accents(engine,'Copyright (C) 2006 2026')
            actual=chinese_accents_from_phonemes(full_phones[c_at+len('[.1si]'):],limit=6)
            assert actual==expected,(hex(dialect),'full NVDA About text',expected,actual)
            checks+=1
            for gender in (0,1):
                engine.set_parameter(GENDER,gender)
                for speed in (75,150):
                    engine.set_parameter(NATIVE_SPEED,speed)
                    for pause in (0,2):
                        engine.set_parameter(PAUSE_MODE,pause)
                        sounds=[]
                        for inflection in (0,30,47,100):
                            engine.set_parameter(NATIVE_INFLECTION,inflection)
                            result=render(engine,[text+' 你好世界。',EciIndex(9)])
                            assert result[0] and result[1][-1]==(9,len(result[0]))
                            sounds.append(result[0]);checks+=1
                        # The native Chinese words and numbers remain sensitive
                        # to the user's setting after many English fragments.
                        assert all(a!=b for a,b in zip(sounds,sounds[1:]))
                        engine.set_parameter(NATIVE_INFLECTION,30)
                        split=text.index('Copyright')
                        plain=render(engine,[text,EciIndex(9)])
                        chunked=render(engine,[text[:split],EciIndex(1),text[split:],EciIndex(9)])
                        assert plain[0]==chunked[0] and [i for i,_ in chunked[1]]==[1,9]
                        assert 0<=chunked[1][0][1]<=chunked[1][1][1]==len(chunked[0]);checks+=1
            # English output in the Mandarin voice remains neutral when
            # the user's Mandarin inflection setting changes.
            engine.set_parameter(PAUSE_MODE,0)
            english=[]
            for inflection in (0,47,100):
                engine.set_parameter(NATIVE_INFLECTION,inflection)
                english.append(render(engine,['Hello world.',EciIndex(9)])[0])
            assert english[0]==english[1]==english[2],(hex(dialect),'English bank inflection');checks+=1
            # Exercise the actual NVDA-processed mixed text, not just the raw
            # English source. Chinese symbol names follow English repeatedly.
            engine.set_parameter(NATIVE_SPEED,75)
            engine.set_parameter(GENDER,0)
            for inflection in (0,30,47,100):
                engine.set_parameter(NATIVE_INFLECTION,inflection)
                processed=render(engine,[symbol_text,EciIndex(9)])
                assert processed[0] and processed[1][-1]==(9,len(processed[0]))
                checks+=1
            split=symbol_text.index('Copyright')
            whole=render(engine,[symbol_text,EciIndex(9)])
            fragments=render(engine,[symbol_text[:split],EciIndex(1),symbol_text[split:],EciIndex(9)])
            assert whole[0]==fragments[0] and [i for i,_ in fragments[1]]==[1,9]
            checks+=1
            # With many successive switches, revisiting an emitted English
            # bank used to end the surrounding Chinese speech loop early.
            # NVDA's higher symbol levels supply precisely such a sequence.
            engine.set_parameter(NATIVE_SPEED,150)
            engine.set_parameter(NATIVE_INFLECTION,30)
            for level in (200,300):
                processed_text=(ROOT/f'tests/fixtures/chinese_nvda_about_symbols_{level}.txt').read_text(encoding='utf8')
                split=processed_text.index('NVDA is developed')
                prefix=render(engine,[processed_text[:split],EciIndex(9)])[0]
                complete=render(engine,[processed_text,EciIndex(9)])[0]
                extended=render(engine,[processed_text+' The big purple banana slowly dances along the beach.',EciIndex(9)])[0]
                # The final paragraphs and an additional sentence must both
                # contribute audible output; previously the lengths froze.
                assert len(complete)>len(prefix)+11025,(hex(dialect),level,'lost final paragraphs')
                assert len(extended)>len(complete)+11025,(hex(dialect),level,'lost appended sentence')
                checks+=2
            # A Chinese transliteration must not replace English-bank words;
            # the native dictionary used to do this only for lowercase forms.
            for word in ('clinton','microsoft','internet'):
                expected=phonemes(engine,word.capitalize())
                assert phonemes(engine,word)==expected,(hex(dialect),word)
                english_phones=''.join(re.findall(r'\[([^]]+)\]',expected))
                english_phones=re.sub(r'\.[0123]','',english_phones)
                for example in (word+' is here','你好 '+word+' 世界'):
                    actual=''.join(re.findall(r'\[([^]]+)\]',phonemes(engine,example)))
                    actual=re.sub(r'\.[0123]','',actual)
                    assert english_phones in actual,(hex(dialect),example,actual)
                    checks+=1
                checks+=1
            # Measure the real audio clock on both sides of the language
            # switch. The English-to-Chinese release used to leave these
            # examples 18 ms and 12 ms longer, respectively. Retain the
            # English final consonant, ordinary punctuation, and manual p50.
            for key,value in ((SAMPLE_RATE,3),(NATIVE_SPEED,75),(NATIVE_INFLECTION,0),
                              (GENDER,0),(PAUSE_MODE,0),(PROCESS_TAGS,1)):
                engine.set_parameter(key,value)
            def milliseconds(s):
                return len(render(engine,[s,EciIndex(9)])[0])/(16000*2)*1000
            hello=milliseconds('你好 Hello 世界')
            world=milliseconds('你好 world 世界')
            assert 999<=hello<1010,(hex(dialect),'hello return',hello)
            assert 973<=world<982,(hex(dialect),'world return',world)
            assert 775<=milliseconds('你好 Hello')<=785,(hex(dialect),'Chinese to English')
            assert milliseconds('你好 Hello，世界')>hello+100,(hex(dialect),'comma pause')
            assert milliseconds('你好 Hello `p50 世界')>hello+20,(hex(dialect),'explicit pause')
            long_english='The quick brown fox jumps over the lazy dog '*8
            assert 14645<=milliseconds(long_english)<=14655,(hex(dialect),'English bank split')
            checks+=6
        finally:engine.close()
    # The NVDA language router must restore the selected Chinese inflection
    # when an explicit language command returns from another language.
    routed=LanguageEngine(ROOT/'eci/libreloq_eci.so',language=CHINESE_CHINA.id,voice='4')
    direct=DirectEciEngine(ROOT/'eci/libreloq_eci.so',dialect=CHINESE_CHINA.id,encoding='utf8',native_voice=True)
    try:
        for key,value in (*CHINESE_CHINA.voice('4').parameters,(NATIVE_SPEED,75),
                          (SAMPLE_RATE,1),(PAUSE_MODE,0),(PROCESS_TAGS,1),(NATIVE_INFLECTION,47)):
            routed.set_parameter(key,value);direct.set_parameter(key,value)
        copyright_text=symbol_text[symbol_text.index('Copyright'):]
        assert render(routed,[EciLanguage(US_ENGLISH.id,'4'),'Hello',
            EciLanguage(CHINESE_CHINA.id,'4'),copyright_text,EciIndex(9)])[0]
        assert render(routed,[copyright_text,EciIndex(9)])==render(direct,[copyright_text,EciIndex(9)])
        checks+=1
    finally:
        routed.close();direct.close()
    print(f'Chinese mixed text: {checks} long-text, inflection, gender, speed, pause and index checks passed')


if __name__=='__main__':main()
