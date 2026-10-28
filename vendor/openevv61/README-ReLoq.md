# Private ReLoq 6.1 rule provider

This directory is a C source dependency, pinned to OpenEVV commit
`d2bc0c68a7769967bb3dfdc86701f69bc53767b4`, with US-English and German 6.1
rules transcribed from Apple's modules. See `LICENSE`, `NOTICE`, and the
release's `reference-apple/behavior-0.2.102/provenance102.json`.

The default build compiles the supplied C functions. It does not invoke
Python, an external language module, a proprietary runtime, or a bytecode
interpreter. `EVV_NO_BYTECODE` is mandatory in `Makefile.native`. The
`delta_rules_<tag>.c` files contain native function metadata and constants;
the rule-code, immediate and jump-map arrays have been removed. The small
`delta_rules_shim_<tag>.c` files are C entry adapters to native functions.

Build from the parent ReLoq project, or run `make -f Makefile.native` here.
The parent ECI makefile builds the Windows archive with the matching MinGW
compiler and includes it in the single ECI DLL. No additional redistributable
threading DLL is required.

`rules/*.up` holds readable imperative rule sources. The C port is partly
generated; it is not a claim that every function was handwritten. The lifted
control flow is executed as compiled C, including C branches, loops and native
runtime calls. The development tools retain an offline bytecode comparison
backend for checking translation. It is excluded from the release build.
`python3 tools/rebuild_native.py` regenerates C through that offline
intermediate and removes its arrays again. Ordinary compilation uses only C.

ReLoq changes include separate scan-field operations, preservation of semantic
machine arguments, mapped 64-bit cell-value aliases, a recovered German
abbreviation argument, and complete phrase-default branches. Version 0.2.104
also fixes private nonempty fence data, saved semantic register aliases, and
runtime call signatures when legacy .dr inventories are absent. The offline
`tools/relift_apple61.py` refreshes every rule from the supplied original Apple
modules and records provenance. All 1,021 English and 840 German rule entries
have native implementations, including `noun_cmpnd2` and `no_one_cases`.
Audited platform/default adapters remain explicit exceptions to fresh lifting.
There are no unsupported61 calls in either language's rule sources.

The private provider exports one-millisecond control frames and source-index
boundaries. It bypasses the provider's own PCM generation; ReLoq's existing
numeric Klatt renderer produces the audio. Index-manager IDs are resolved to
their source payloads before capture. Host pitch events use ReLoq's continuous
physical pitch law without introducing native mid-phrase command pauses.

See the source and build files for details; generated functions are kept
separate from the public ReLoq ECI ABI.

0.2.103 adds optional sparse interval delivery. The provider retains the same
single-precision interpolation endpoints; ReLoq realizes the integer controls
while streaming PCM instead of waiting for a whole dense matrix. Native source
index clocks are rebased across text stretches. German now uses a complete
sentence-context phoneme pass; the German host still owns its acoustic
composer. The release verifier checks both dense and sparse paths against the
saved original observations.

0.2.104 feeds full English text to the native frontend. Source marks use the
consumed-character callbacks and emitted control clock without fragmenting text.
Setup commands are excluded from source-position accounting. One owner-scoped
native worker is reused for each ECI instance; no engine is shared between public
handles. After successful plain-text requests the provider fully resets and
prepares that worker with the current voice, annotation and phoneme mode while
the host already has its controls/audio. The next matching request can use that
fresh prepared state. Tags or changed settings use the full reset path before
input. Controller and sent-setting snapshots are restored together, and array
interpolation cursors/clocks are cleared per request. The release test counts
native handles as well as comparing results, so accidental recreation cannot
masquerade as reuse. Setup-character counts are excluded from source indices. Rule C and the
shared runtime use `-O2`; `-Os` was slower in the comparison and is not the
release default. Floating-point contraction remains disabled. Superseded ReLoq
English model tables are excluded by the parent ECI
makefile, not silently retained as an alternate acoustic path.

Release 0.2.149 gives Greek its own 18.0 language identity and replaces character names inside the shared native character rule. Chinese locales share 597 call-graph-closed identical C bodies. The C romanizers retain whole Latin-word properties and the original Pinyin classifier. Supplied ViaVoice desktop Japanese word data replaces the former word banks and normal index; the complete desktop algorithm is not yet ported. See the release working document and behavior-0.2.149 checks for remaining limits.
