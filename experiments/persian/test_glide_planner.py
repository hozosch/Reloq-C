"""Exercise the selected transition planner without NumPy or native audio."""
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_persian_glide import TRACT, apply_u_a_glides, plan_u_a_glides


def native_frames():
    frames = [[0.0] * 62 for _ in range(140)]
    for i, row in enumerate(frames):
        rise = min(10, max(0, i - 50))
        row[0] = 1.0
        row[1] = 1300.0 - min(200, max(0, i - 50) * 10)
        row[2] = 50.0
        row[9] = 300.0 + 30 * rise
        row[13] = 700.0 + 30 * rise
    return frames, list(range(len(frames)))


class GlidePlannerTest(unittest.TestCase):
    def test_selected_variant_changes_only_tract_and_local_pitch(self):
        original, ticks = native_frames()
        tags = ("`[.1'u:']", "`[.1'a:'.0mAd]")
        plans = plan_u_a_glides(original, ticks, tags, (' ',), {1: 50})
        self.assertEqual(len(plans), 1)
        plan = plans[0]
        self.assertEqual(plan.summary, (1, 25, 60, 50, 65))
        result = [row[:] for row in original]
        apply_u_a_glides(result, ticks, plans)
        self.assertNotEqual(result[40][9], original[40][9])
        self.assertNotEqual(result[55][1], original[55][1])
        self.assertEqual(result[plan.first], original[plan.first])
        for i, (before, after) in enumerate(zip(original, result)):
            for column, (old, new) in enumerate(zip(before, after)):
                if column in TRACT and plan.first <= i <= plan.end:
                    continue
                if column == 1 and plan.peak <= i <= plan.pitch_end:
                    continue
                self.assertEqual(old, new, (i, column))
        self.assertEqual(result[plan.end][9], original[plan.end][9])
        self.assertEqual(result[plan.peak][1], original[plan.peak][1])
        self.assertEqual(result[plan.pitch_end][1], original[plan.pitch_end][1])

    def test_ineligible_boundaries_and_glottalization_are_unchanged(self):
        original, ticks = native_frames()
        left, right = "`[.1'u:']", "`[.1'a:'.0mAd]"
        for tags, sep in (((left, right), ','), ((left, '`[.1iran]'), ' '),
                          ((left, '`[.1wa]'), ' ')):
            self.assertEqual(plan_u_a_glides(original, ticks, tags, (sep,), {1: 50}), ())
        original[50][2] = 0.0
        self.assertEqual(plan_u_a_glides(original, ticks, (left, right), (' ',), {1: 50}), ())

    def test_missing_boundary_is_not_guessed(self):
        original, ticks = native_frames()
        with self.assertRaisesRegex(RuntimeError, 'missing vowel word boundary'):
            plan_u_a_glides(original, ticks, ("`[.1'u:']", '`[.1a]'), (' ',), {})


if __name__ == '__main__':
    unittest.main()
