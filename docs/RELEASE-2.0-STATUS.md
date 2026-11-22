# Release 2.0 preparation

This is a development checklist, not a release announcement. Keep issues open
until 2.0 is actually published; recheck their scope against the released code.

The following open issues have fixes and focused regression tests in this
development branch and are candidates for review after publication:

- [#46](https://github.com/hozosch/Reloq-C/issues/46): attached uppercase words
  and numbers; `nvda/test_eci_upper_number.py` checks the normal word reading,
  character mode, source indexes and dictionaries in German and both English
  voices.
- [#54](https://github.com/hozosch/Reloq-C/issues/54): uppercase currency `B`;
  `nvda/test_eci_english_currency_case.py` compares audio and indexes with the
  lowercase form in US and UK English.
- [#56](https://github.com/hozosch/Reloq-C/issues/56): parameter changes before
  digits; `nvda/test_eci_inline_queue_digits.py` checks queued controls and
  indexes, and the Apple ECI smoke test also exercises the reported example.

The native release workflow runs those checks. Package names follow their
respective manifest versions, and the version preflight rejects mismatched
NVDA, Android and SAPI versions or an incorrect Android version code. The
current public package metadata still says 1.4.1; choose and synchronize the
2.0 version only when preparing a real candidate.

In the Persian NVDA development voice, grouped numbers could cause the first
following Persian word to be cut at an English/Persian switch. The planner
now discards trailing punctuation introduced only by the English phoneme display;
`test_mixed_number_word_boundaries.py` checks the reported heading and related
numbers, source indexes, voices and both streaming modes. The reported case
still needs listening confirmation from users. Issue #49 remains open for the
rest of Persian development and release validation.

Persian is not yet an independent registered voice or integrated on all
platforms. See [PERSIAN-NATIVE-STATUS.md](PERSIAN-NATIVE-STATUS.md) for the
unfinished native orchestration, streaming and sound verification. A release
candidate also needs its own notes, installable packages, upgrade tests and
listening feedback. The separate Apple app must be updated by its maintainer;
successful unsigned CI builds do not verify VoiceOver on a device.
