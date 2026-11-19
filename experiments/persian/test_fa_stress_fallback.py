"""The reference fallback must not undo stress for neighboring native words."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex
from direct_fa_sequence import project, project_resilient
from hybrid_fa_frontend import DirectFirstPersian, PersianAnalysis
from text_frontend import SpeechPart, UnsupportedPronunciation


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    reference = PersianAnalysis((SpeechPart('fa', '`[.0nA.0mi.1pA.0zi.0rAd]'),), None, True)
    with DirectFirstPersian(args.library, None, None, None) as frontend:
        assert frontend.analyze_direct('نمی‌روم') is not None
        assert frontend.analyze_direct('نمی‌پذیرد') is None
        parts = ('کتاب ', EciIndex(7), 'نمی‌پذیرد سلام.')
        with patch.object(frontend, 'analyze', return_value=reference) as fallback:
            try:
                project(parts, frontend, allow_fallback=True)
                raise AssertionError('complete reference fallback discarded native word stress')
            except UnsupportedPronunciation:
                pass
            result = project_resilient(parts, frontend)
        assert result.tagged == '`[.0ke.1tab] `[.1nA.0mi.0pA.0zi.0rAd] `[.0sA.1lam].', result.tagged
        assert result.needs_native_validation
        assert [part.value for part in result.parts if isinstance(part, EciIndex)] == [7]
        fallback.assert_called_once_with('نمی‌پذیرد')
        # One unrelated fallback word must not lose a two-word verbal prefix
        # or the verified split form of علامت‌دار.
        with patch.object(frontend, 'analyze', return_value=reference):
            prefix = project_resilient(('می ', EciIndex(8), 'روم پدیده‌ای'), frontend)
            compound = project_resilient(('علامت ', EciIndex(9), 'دار پدیده‌ای'), frontend)
        assert prefix.tagged.startswith('`[.1mi] `00 `[.0rA.0vAm] '), prefix.tagged
        assert compound.tagged.startswith('`00 `[.0A.0la.0mAt] `[.1dar] '), compound.tagged
        assert [part.value for part in prefix.parts if isinstance(part, EciIndex)] == [8]
        assert [part.value for part in compound.parts if isinstance(part, EciIndex)] == [9]
        with patch.object(frontend, 'analyze', return_value=reference):
            object_marker = project_resilient(('پدیده‌ای ', EciIndex(10), 'را'), frontend)
            copula_after_reference = project_resilient(('پدیده‌ای ', EciIndex(17), 'است'), frontend)
            separated = project_resilient(('پدیده‌ای، ', EciIndex(11), 'را'), frontend)
            conjunction = project_resilient(('کتاب و ', EciIndex(12), 'پدیده‌ای'), frontend)
        assert object_marker.tagged.endswith(' `00 `[.0ra]'), object_marker.tagged
        assert copula_after_reference.tagged.endswith(' `00 `[.0Ast]'), copula_after_reference.tagged
        assert separated.tagged.endswith(', `[.1ra]'), separated.tagged
        assert '`[.0vA]' in conjunction.tagged, conjunction.tagged
        assert [part.value for part in object_marker.parts if isinstance(part, EciIndex)] == [10]
        # An uncovered Pinglish word must not let a whole-clause reference
        # replace the native weak copula, object marker or conjunction.
        original_pinglish = frontend.analyze_pinglish
        unknown = PersianAnalysis((SpeechPart('fa', '`[.1zon]'),), None, True)
        def pinglish_reference(text, *, positioned, allow_fallback):
            if text.startswith('zooon'):
                return unknown if text == 'zooon' and allow_fallback else None
            return original_pinglish(text, positioned=positioned, allow_fallback=allow_fallback)
        with patch.object(frontend, 'analyze_pinglish', side_effect=pinglish_reference):
            try:
                project(('zooon ', EciIndex(13), 'ast'), frontend, pinglish=True, allow_fallback=True)
                raise AssertionError('Pinglish whole-clause fallback discarded native stress')
            except UnsupportedPronunciation:
                pass
            copula = project_resilient(('zooon ', EciIndex(13), 'ast'), frontend, pinglish=True)
            object_marker = project_resilient(('zooon ', EciIndex(14), 'ra'), frontend, pinglish=True)
            separated = project_resilient(('zooon, ', EciIndex(15), 'ast'), frontend, pinglish=True)
            conjunction = project_resilient(('ketab va ', EciIndex(16), 'zooon'), frontend, pinglish=True)
        assert copula.tagged.endswith(' `00 `[.0Ast]'), copula.tagged
        assert object_marker.tagged.endswith(' `00 `[.0ra]'), object_marker.tagged
        assert separated.tagged.endswith(', `[.1Ast]'), separated.tagged
        assert '`[.0vA]' in conjunction.tagged, conjunction.tagged
        assert [part.value for part in copula.parts if isinstance(part, EciIndex)] == [13]
        # A transient reference error gets one whole-word retry, while an
        # interrupted job must not start that retry.
        with patch.object(frontend, 'analyze', side_effect=(RuntimeError('transient'), reference)) as fallback:
            recovered = project_resilient(('نمی‌پذیرد',), frontend)
        assert recovered.tagged == '`[.1nA.0mi.0pA.0zi.0rAd]' and fallback.call_count == 2
        cancelled = [False]
        def fail_and_cancel(_word):
            cancelled[0] = True
            raise RuntimeError('cancelled reference')
        with patch.object(frontend, 'analyze', side_effect=fail_and_cancel) as fallback:
            try:
                project_resilient(('نمی‌پذیرد',), frontend, cancelled=lambda: cancelled[0])
                raise AssertionError('cancelled reference was retried')
            except InterruptedError:
                pass
        assert fallback.call_count == 1
    print('Passed mixed native/reference Persian stress and index projection.')


if __name__ == '__main__':
    main()
