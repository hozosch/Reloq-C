"""Create a labelled, offline comparison of Persian and US-derived contours."""
import argparse
from pathlib import Path
import re
import sys
import wave

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import DirectEciEngine, EciIndex, PROCESS_TAGS, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture, FrameReplay
from continuous_driver import _PlanningVoice
from continuous_plan import CapturedRows, _dense, _fit_context_pitch, _last_audible, persian_proxy_for_english
from direct_fa_sequence import project
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE, US_ENGLISH
from text_frontend import SpeechPart

RATE = 16000
SAMPLES = (
    ('سلام دنیا.', 'Aussage: Salam Donya.'),
    ('سلام دنیا؟', 'Frage: Salam Donya.'),
    ('این یک آزمایش است.', 'Aussage: Das ist ein Test.'),
    ('این یک آزمایش است؟', 'Frage: Ist das ein Test?'),
    ('پنجره را باز کنید.', 'Bitte das Fenster öffnen.'),
    ('کتاب شما خوب است.', 'Ihr Buch ist gut.'),
)


def with_indexes(text):
    matches = list(re.finditer(r'`\[[^]]+\]', text))
    out, previous = [], 0
    for i, match in enumerate(matches):
        if i:
            out.extend((text[previous:match.start()], EciIndex(i)))
        else:
            out.append(text[:match.start()])
        out.append(match.group())
        previous = match.end()
    out.append(text[previous:])
    return tuple(out), matches


def render(voice, parts):
    pcm = bytearray()
    assert voice.speak(parts, pcm.extend, lambda _i: None) and pcm
    return bytes(pcm)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    library = args.library.resolve()
    owner = make_language_engine(library, library, None, args.espeak_library, args.data_parent)
    german = DirectEciEngine(library, dialect=0x40000, encoding='utf8', native_voice=True)
    owner.set_parameter(SAMPLE_RATE, 3)
    german.set_parameter(SAMPLE_RATE, 3)
    german.set_parameter(PROCESS_TAGS, 1)
    fa_key = owner._key(PERSIAN_PROBE.id, '1')
    en_key = owner._key(US_ENGLISH.id, '1')
    fa_engine = owner._activate(fa_key)
    en_engine = owner._activate(en_key)
    fa = _PlanningVoice(owner, fa_key, fa_engine)
    en = _PlanningVoice(owner, en_key, en_engine)
    en.raw.set_parameter(PROCESS_TAGS, 1)
    capture = FrameCapture(fa._dll)
    audio = bytearray()

    def append(pcm):
        if not pcm:
            raise RuntimeError('empty sample')
        audio.extend(pcm)
        audio.extend(bytes(RATE))

    def label(text):
        append(render(german, (text,)))

    try:
        label('Persisch. Vergleich der bisherigen Satzmelodie mit einem amerikanischen Melodieversuch. '
              'Die persischen Laute und ihre Dauer sind jeweils gleich.')
        for number, (text, description) in enumerate(SAMPLES, start=1):
            projected = project((text,), fa_engine._frontend, allow_fallback=True)
            if projected.needs_native_validation:
                raise RuntimeError('comparison requires a directly covered Persian phrase')
            fa_parts, tags = with_indexes(projected.tagged)
            proxy = persian_proxy_for_english(projected.tagged, en)
            en_parts, proxy_tags = with_indexes(proxy)
            if len(tags) != len(proxy_tags) or len(tags) < 2:
                raise RuntimeError('English and Persian word markers differ')
            fa_result = CapturedRows.from_capture(capture.capture(fa, fa_parts), native=capture.frame_ticks)
            en_result = CapturedRows.from_capture(capture.capture(en, en_parts), native=capture.frame_ticks)
            count = len(tags)
            fa_bounds = (0, *(fa_result.indexes[i] for i in range(1, count)), len(fa_result.pcm) // 2)
            en_bounds = (0, *(en_result.indexes[i] for i in range(1, count)))
            curve = _dense(en_result, 1, native=capture.dense_curve)
            pilot_bounds = (*[round(i * 1000 / RATE) for i in en_bounds],
                            min(_last_audible(en_result.pcm, RATE, native=capture.last_audible_sample),
                                len(curve) - 1))
            if any(a >= b for a, b in zip(fa_bounds, fa_bounds[1:])) or any(
                    a >= b for a, b in zip(pilot_bounds, pilot_bounds[1:])):
                raise RuntimeError('word pitch intervals are not strictly ordered')
            parts = tuple(SpeechPart('fa', match.group()) for match in tags)
            owner._activate(fa_key)
            unchanged = bytearray()
            scope = fa_engine._scope(1)
            try:
                assert FrameReplay(fa._dll).replay(fa_engine._voice, fa_result.frames, (),
                                                   unchanged.extend, lambda _i: None)
            finally:
                fa_engine._scope(scope)
            assert unchanged == fa_result.pcm, 'unmodified frame replay changed the Persian audio'
            _fit_context_pitch(fa_result, fa_bounds, curve, pilot_bounds, parts, 'fa', RATE,
                               native=capture.fit_pitch)
            owner._activate(fa_key)
            altered = bytearray()
            scope = fa_engine._scope(1)
            try:
                ok = FrameReplay(fa._dll).replay(fa_engine._voice, fa_result.frames, (),
                                                  altered.extend, lambda _i: None)
                assert ok and len(altered) == len(fa_result.pcm) and altered != fa_result.pcm
            finally:
                fa_engine._scope(scope)
            label(f'Beispiel {number}. {description} Bisherige Melodie.')
            append(fa_result.pcm)
            label('Amerikanischer Melodieversuch.')
            append(altered)
            print(number, text, 'words', count, 'original/variant bytes', len(fa_result.pcm), len(altered), flush=True)
        label('Ende des Vergleichs.')
    finally:
        owner.close()
        german.close()
    output = args.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(output), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(audio)
    print('Wrote', output, 'seconds', len(audio) / (RATE * 2), flush=True)


if __name__ == '__main__':
    main()
