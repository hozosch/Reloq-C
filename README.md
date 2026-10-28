# ReLoq C

Native C speech synthesis and an experimental NVDA add-on.

This repository is being initialized from the verified 0.2.186 source release. The active C implementation, data, NVDA driver and reproducible checks are the working baseline. Historical source archives remain preserved separately.

## Current work

The following user-reported problems remain open:

- American English can crash while pronunciation dictionaries are enabled.
- Greek letters are still not handled correctly in ordinary Finnish and Korean sentences in the user's NVDA setup.
- Main text replacements must enter ordinary text analysis and preserve case-sensitive matching.
- Changes in the intended dictionary folder do not reliably become active.

A passing isolated pronunciation test is not sufficient evidence that these runtime problems are fixed.

The existing German acoustic planner and documented pronunciation departures are retained. No speech-result cache, original runtime DLL/SYN, interpreter or bytecode belongs in the add-on. Verification audio stays in memory.

The source import is in progress. Build instructions and project files follow in the next commit.
