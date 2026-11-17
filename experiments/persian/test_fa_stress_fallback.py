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
    reference = PersianAnalysis((SpeechPart('fa', '`[.0nA.0mi.1rA.0vAm]'),), None, True)
    with DirectFirstPersian(args.library, None, None, None) as frontend:
        assert frontend.analyze_direct('نمی‌روم') is None
        parts = ('کتاب ', EciIndex(7), 'نمی‌روم سلام.')
        with patch.object(frontend, 'analyze', return_value=reference) as fallback:
            try:
                project(parts, frontend, allow_fallback=True)
                raise AssertionError('complete reference fallback discarded native word stress')
            except UnsupportedPronunciation:
                pass
            result = project_resilient(parts, frontend)
        assert result.tagged == '`[.0ke.1tab] `[.1nA.0mi.0rA.0vAm] `[.0sA.1lam].', result.tagged
        assert result.needs_native_validation
        assert [part.value for part in result.parts if isinstance(part, EciIndex)] == [7]
        fallback.assert_called_once_with('نمی‌روم')
        # One unrelated fallback word must not lose a two-word verbal prefix
        # or the verified split form of علامت‌دار.
        with patch.object(frontend, 'analyze', return_value=reference):
            prefix = project_resilient(('می ', EciIndex(8), 'روم پدیده‌ای'), frontend)
            compound = project_resilient(('علامت ', EciIndex(9), 'دار پدیده‌ای'), frontend)
        assert prefix.tagged.startswith('`[.1mi] `[.0rA.0vAm] '), prefix.tagged
        assert compound.tagged.startswith('`[.0A.0la.0mAt] `[.1dar] '), compound.tagged
        assert [part.value for part in prefix.parts if isinstance(part, EciIndex)] == [8]
        assert [part.value for part in compound.parts if isinstance(part, EciIndex)] == [9]
        with patch.object(frontend, 'analyze', return_value=reference):
            object_marker = project_resilient(('پدیده‌ای ', EciIndex(10), 'را'), frontend)
            separated = project_resilient(('پدیده‌ای، ', EciIndex(11), 'را'), frontend)
            conjunction = project_resilient(('کتاب و ', EciIndex(12), 'پدیده‌ای'), frontend)
        assert object_marker.tagged.endswith(' `[.0ra]'), object_marker.tagged
        assert separated.tagged.endswith(', `[.1ra]'), separated.tagged
        assert '`[.0vA]' in conjunction.tagged, conjunction.tagged
        assert [part.value for part in object_marker.parts if isinstance(part, EciIndex)] == [10]
    print('Passed mixed native/reference Persian stress and index projection.')


if __name__ == '__main__':
    main()
