"""Compile a cautious native Persian word table from the eSpeak sources.

The eSpeak reference is needed only by this offline generator, never by the
resulting C lookup. Only exact complete-word candidates whose supported phone
tags agree in four reference contexts are admitted. This is a first lexical
component, not a replacement for the context rules in fa_rules.
"""
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import unicodedata

from espeak_ng_probe import EspeakPhonemeProbe
from text_frontend import UnsupportedPronunciation, native_tag_accepted, word_tag
from test_selected_probe import engine, set_confirmed_persian_probe

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'reference-espeak/persian'
CORPUS = Path(__file__).with_name('reference_espeak_1_51.jsonl')
DEFAULT_OUTPUT = Path(__file__).with_name('direct_fa_lexicon.inc')
DEFAULT_CONTEXT_OUTPUT = Path(__file__).with_name('direct_fa_context.inc')
EXPECTED_SOURCES = {
    'fa_list': '33b451d3bbe0ed6cc84eb03428277be35e90bfaaa511fbb6ce15d699232a5843',
    'fa_rules': '6a5d75f7bddeb9634a11eb6a5a5aa1ad2e8b170be32c6a322930bc5874138203',
}
EXPECTED_DICT = '5e9830a37191dc958767407a8e515dbde9901710033a012a7b2336136855e4ac'


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def lexical_fingerprint(rows) -> str:
    """Fingerprint both ordered keys and tags for generated dependent tables."""
    encoded = json.dumps(rows, ensure_ascii=False, separators=(',', ':')).encode('utf8')
    return hashlib.sha256(encoded).hexdigest()[:16]


def is_persian_word(text: str) -> bool:
    if not 1 <= len(text) <= 64:
        return False
    return all((0x600 <= ord(c) <= 0x6ff and unicodedata.category(c)[0] in 'LM')
               or c == '\u200c' for c in text)


def candidates(source: Path, corpus: Path) -> tuple[str, ...]:
    # The common conjunction و comes from fa_rules, not fa_list, and is not
    # guaranteed to occur in the small reference-sentence corpus.
    words = {'و'}
    for raw in source.read_text(encoding='utf8').splitlines():
        line = raw.split('//', 1)[0].strip()
        if not line:
            continue
        fields = line.split()
        if len(fields) >= 2 and is_persian_word(fields[0]):
            words.add(fields[0])
    # Include reference-sentence words absent from fa_list (for example
    # سلام), whose reference pronunciation comes from fa_rules instead.
    for line in corpus.read_text(encoding='utf8').splitlines():
        row = json.loads(line)
        if row['voice'] != 'fa':
            continue
        token = ''
        for char in row['text'] + ' ':
            if (0x600 <= ord(char) <= 0x6ff and unicodedata.category(char)[0] in 'LM') or char == '\u200c':
                token += char
            else:
                if is_persian_word(token):
                    words.add(token)
                token = ''
    return tuple(sorted(words, key=lambda s: s.encode('utf8')))


def reference_tags(api: EspeakPhonemeProbe, word: str) -> tuple[str, ...]:
    """Return four word-aligned reference contexts, rejecting unsupported phones."""
    def at(text: str, count: int, index: int) -> str:
        clauses = api.translate(text, separator='|')
        if len(clauses) != 1:
            raise UnsupportedPronunciation('clause alignment')
        words = clauses[0].phonemes.strip().split()
        if len(words) != count or '(en)' in words[index] or '(fa)' in words[index]:
            raise UnsupportedPronunciation('word alignment')
        return word_tag(words[index])
    return (at(word, 1, 0), at(word + ' آمد.', 2, 0),
            at('من ' + word + ' آمد.', 3, 1), at('من ' + word + '.', 2, 1))


def checked_tag(api: EspeakPhonemeProbe, word: str) -> str:
    """Reject misalignment, unsupported sounds, and differing stress forms."""
    tags = reference_tags(api, word)
    # Comma-bearing words live in the separately generated pause table. The
    # ordinary lexicon and its dependent plural bitmap remain unchanged.
    if any(',' in tag for tag in tags):
        raise UnsupportedPronunciation('unsupported eSpeak phone short pause')
    if any(tag != tags[0] for tag in tags[1:]):
        raise UnsupportedPronunciation('context-dependent pronunciation or stress')
    return tags[0]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--probe', type=Path, required=True,
                        help='selected Finnish-based probe for checking native phoneme tags')
    parser.add_argument('--output', type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument('--context-output', type=Path, default=DEFAULT_CONTEXT_OUTPUT)
    args = parser.parse_args()
    sources = (SOURCE / 'fa_list', SOURCE / 'fa_rules')
    if any(digest(path) != EXPECTED_SOURCES[path.name] for path in sources):
        raise RuntimeError('the Persian rule sources differ from the pinned reference')
    if digest(args.data_parent / 'espeak-ng-data/fa_dict') != EXPECTED_DICT:
        raise RuntimeError('the eSpeak Persian dictionary differs from the pinned reference')
    words = candidates(sources[0], CORPUS)
    accepted = []
    contextual = []
    rejected = Counter()
    native = engine(args.probe, 0x90000)
    try:
        set_confirmed_persian_probe()
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            version = api.version
            if version != '1.53.0':
                raise RuntimeError('this generated table requires eSpeak NG 1.53.0')
            for i, word in enumerate(words, 1):
                try:
                    tag = checked_tag(api, word)
                    if not native_tag_accepted(tag, native._phonemes_for_probe(tag)):
                        raise UnsupportedPronunciation('native phoneme tag rejected or changed')
                    accepted.append((word, tag))
                except UnsupportedPronunciation as error:
                    category = str(error).split(':', 1)[0]
                    if category == 'context-dependent pronunciation or stress':
                        # These function words are stressed when phrase-final,
                        # and take the other reference form before a word.
                        # Do not generalize other context changes into a rule.
                        variants = reference_tags(api, word)
                        if (variants[0] == variants[3] and variants[1] == variants[2]
                                and re.sub(r'\.[012]', '.', variants[0]) ==
                                    re.sub(r'\.[012]', '.', variants[1])
                                and all(native_tag_accepted(t, native._phonemes_for_probe(t))
                                        for t in set(variants))):
                            contextual.append((word, variants[0], variants[1]))
                    if category.startswith('unsupported eSpeak phone'):
                        category = 'unsupported phone'
                    elif category.startswith('unexpected stress or length'):
                        category = 'unsupported stress or length'
                    rejected[category] += 1
                if i % 3000 == 0:
                    print(f'Checked {i}/{len(words)} candidate words', flush=True)
    finally:
        native.close()
    if len(accepted) < len(words) // 2:
        raise RuntimeError('unexpectedly low reference coverage; no table written')
    metadata = {
        'espeak_source_commit': '688cb60368d6e0998f3632af8d1c3c2ad4191fc2',
        'espeak_version': version,
        'fa_list_sha256': digest(sources[0]),
        'fa_rules_sha256': digest(sources[1]),
        'fa_dict_sha256': digest(args.data_parent / 'espeak-ng-data/fa_dict'),
        'native_probe_sha256': digest(args.probe),
        'corpus_sha256': digest(CORPUS),
        'candidate_words': len(words), 'admitted_words': len(accepted),
        'rejected': dict(sorted(rejected.items())),
    }
    lines = [
        '/* Generated by compile_direct_lexicon.py. Do not edit.\n',
        ' * Based on eSpeak NG Persian rules and dictionary by Shadyar Khodayari\n',
        ' * and Ehsan Esmaili, GPL-3.0-or-later; see reference-espeak/persian.\n',
        ' * This experimental complete-word table does not implement fa_rules.\n',
        ' * Reference metadata: ' + json.dumps(metadata, sort_keys=True, ensure_ascii=True) + '\n',
        ' */\n',
        '#define RELOQ_FA_LEXICON_FINGERPRINT 0x' + lexical_fingerprint(accepted) + 'ULL\n',
        'static const struct fa_direct_entry fa_direct_entries[] = {\n',
    ]
    for word, tag in accepted:
        lines.append('    {' + json.dumps(word, ensure_ascii=False) + ', '
                     + json.dumps(tag, ensure_ascii=True) + '},\n')
    lines.append('};\n')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    temporary = args.output.with_name(args.output.name + '.tmp')
    temporary.write_text(''.join(lines), encoding='utf8', newline='\n')
    temporary.replace(args.output)
    context_lines = [
        '/* Generated by compile_direct_lexicon.py. Do not edit.\n',
        ' * Derived from eSpeak NG Persian rules and dictionary, GPL-3.0-or-later;\n',
        ' * see reference-espeak/persian. Final and nonfinal tags only.\n',
        ' */\n',
        'static const struct fa_context_entry fa_context_entries[] = {\n',
    ]
    for word, final, nonfinal in contextual:
        context_lines.append('    {' + ', '.join(json.dumps(item, ensure_ascii=False)
                                                 for item in (word, final, nonfinal)) + '},\n')
    context_lines.append('};\n')
    args.context_output.parent.mkdir(parents=True, exist_ok=True)
    temporary = args.context_output.with_name(args.context_output.name + '.tmp')
    temporary.write_text(''.join(context_lines), encoding='utf8', newline='\n')
    temporary.replace(args.context_output)
    print(f'Context-sensitive words admitted: {len(contextual)}', flush=True)
    print(json.dumps(metadata, ensure_ascii=False, sort_keys=True), flush=True)


if __name__ == '__main__':
    main()
