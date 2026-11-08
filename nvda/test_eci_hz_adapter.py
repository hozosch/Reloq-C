"""Exact pitch and temporary capital marks through the real ECI binding."""
from pathlib import Path
from _reloq_eci_direct import (DirectEciEngine, EciPitchHz, EciText, PITCH_HZ,
    NATIVE_PITCH, NATIVE_INFLECTION, SAMPLE_RATE, PROCESS_TAGS)
from test_eci_pitch_hz import measure_pitch


def main():
    engine=DirectEciEngine(Path(__file__).resolve().parents[1]/'eci/libreloq_eci.so',
                           native_voice=True,encoding='utf8')
    def speak(parts):
        chunks=[]
        assert engine.speak(parts,chunks.append,lambda _:None)
        return b''.join(chunks)
    try:
        for key,value in ((SAMPLE_RATE,3),(NATIVE_INFLECTION,0),(PROCESS_TAGS,0),
                          (NATIVE_PITCH,65),(PITCH_HZ,150)):
            engine.set_parameter(key,value)
        normal=speak(['Hallo.'])
        assert abs(measure_pitch(normal,16000,150)-150)<1
        raised=speak([EciPitchHz(180),EciText('Hallo.',False),EciPitchHz(150)])
        assert abs(measure_pitch(raised,16000,180)-180)<1
        assert speak(['Hallo.'])==normal
        assert engine._dll.eciGetParam(engine._handle,8)==0
        # A temporary mark is also scoped on failed/cancelled utterances.
        for invalid in (-1,24001):
            try:engine.set_parameter(PITCH_HZ,invalid)
            except ValueError:pass
            else:raise AssertionError(invalid)
        assert speak(['Hallo.'])==normal
        engine.set_parameter(PITCH_HZ,0)
        engine.set_parameter(NATIVE_PITCH,65)
        assert speak(['Hallo.'])!=normal
    finally:
        engine.close()
    print('Hertz adapter: pitch, temporary marks, ECI units and VB restoration passed')


if __name__=='__main__':main()
