# Native long-text response

The German generic frontend analyzes the complete request and builds its
acoustic target curves before delivering PCM. The native first-PCM delay
therefore grows with text length, even with no playback device or NVDA queue.
This is already observable in the official 1.0 host.

Initial warm probes at native speed 75 used repeated complete sentences:

| Language | Characters | First PCM, 11.025 kHz | First PCM, 48 kHz |
| --- | ---: | ---: | ---: |
| German | 74 | about 2 ms | about 2 ms |
| German | 9,472 | about 150 ms | about 147 ms |
| US English | 63 | about 16 ms | about 17 ms |
| US English | 8,064 | about 16 ms | about 16 ms |

Diagnostic checkpoints for one 9,472-character German request measured
98.9 ms in text analysis and another 40.4 ms preparing the acoustic output.
At 37,888 characters those stages took 400.3 and 172.4 ms. These are Linux
host observations from synthetic repeated text, not audible NVDA or device
latency measurements.

Cancellation is a separate problem: a stop during the native German
phoneme pass previously waited for that complete pass to return. Polling
only the phoneme output sink is too late. The German phoneme pass and the shared language renderer now scope their
owner stop predicates into native rule evaluation. The existing rule error checkpoints
sample it every 64 checks and use the existing error backtrack to leave the
active rule. The preceding predicate is restored after the provider returns.
An interrupted pass returns cancellation, without retrying word analysis.
The private phoneme reader also stops on a failed engine fetch and discards
its partial buffer. Repeating a permanently failed fetch previously spun
forever during a pre-output abort.

A native outer unwind restores its arena frame cursor as well as its
landing depth. Otherwise skipped C frame pops create new landing addresses
on each interruption. Native rule ownership also separates nested machines:
an English child request receives its own outer landing and restores its
parent landing afterward. This prevents a child abort from skipping the
provider cleanup that restores its parent's inline queue and worker state.

This does not make the initial analysis incremental. A faster initial
response for an uninterrupted long German request still requires earlier
delivery of complete, correctly analyzed speech stretches. Raw character or
word splitting would risk abbreviations, dictionary normalization, source
indices and sentence stress; those semantics need to survive that change.

`tools/measure_eci_startup.py` records first PCM, interruption return and the
next short utterance, optionally with a dictionary directory. Audio stays in
RAM. Each library runs in a separate process; cold initialization is warmed
before length comparisons. Timing values are observations, not test limits.
`nvda/test_eci_analysis_cancel.py` checks German/US English cancellation and
exact next-speech recovery after the loaded dictionary source is removed.
`tests/test_reloq_german_cancel.c` checks native cancellation before output,
sink cancellation, allocation-error status propagation and exact
post-cancellation recovery without relying on wall-clock timing.

Validated with the public 1.0 host as baseline:

| Native response after stopping a long request | 1.0, 11.025 kHz | Candidate, 11.025 kHz | 1.0, 48 kHz | Candidate, 48 kHz |
| --- | ---: | ---: | ---: | ---: |
| German, 37,888 characters | 362.9 ms | 0.264 ms | 397.1 ms | 0.216 ms |
| US English, 32,256 characters | 1,256.4 ms | 0.249 ms | 1,240.7 ms | 0.187 ms |

These are medians of five probes, with Stop requested after 25 ms. The next
short request's first PCM arrives about 15–16 ms after Stop. The normal
uninterrupted waveform and indices remain bit-exact in 196 language/rate/
model cases. Median full-synthesis CPU ratio is 1.0012 in alternating host
measurements: the gain is in discarding unwanted work, not faster completion
of an uninterrupted utterance.

Thirty-two German/US stop/recovery cases retain exact dictionary output
with the source file absent. Another 28 stop/recovery cases across all 14
NVDA languages retain their next utterance exactly. The slowest of those
observed returns is 64.3 ms for Chinese; these probes do not establish a
universal cancellation deadline. CJK tests pass 366 Unicode/fragment/index
checks and 36 additional exact accent-PCM comparisons against 1.0.

The retained glibc allocation count stabilizes by the twentieth stop and
is unchanged through the fortieth after frame cleanup. This is an allocator
observation with Python garbage collection, not RSS. The new helpers remain
private, and the 21-symbol ECI interface is unchanged. Full measurements are
in `docs/validation/long-text-latency.json`. No installable package or public
release is changed by this branch.
