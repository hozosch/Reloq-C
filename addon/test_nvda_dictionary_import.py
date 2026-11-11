"""Check NVDA dictionary migration without changing any source files."""
import re
import sys
import tempfile
import types
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))


class Entry:
    def __init__(self, pattern, replacement, comment, caseSensitive=True, type=0):
        self.pattern, self.replacement, self.comment = pattern, replacement, comment
        self.type = type
        expression = pattern if type == 1 else r'\b' + re.escape(pattern) + r'\b' if type == 2 else re.escape(pattern)
        self.compiled = re.compile(expression, 0 if caseSensitive else re.IGNORECASE)

    def sub(self, text):
        return self.compiled.sub(self.replacement, text)


class SpeechDict(list):
    def load(self, path):
        self.fileName = path
        self.clear()
        for line in Path(path).read_text(encoding='utf-8-sig').splitlines():
            if not line.strip() or line.startswith('#'): continue
            pattern, replacement, case, kind = line.split('\t')
            self.append(Entry(pattern.replace(r'\#', '#'), replacement.replace(r'\#', '#'), '', bool(int(case)), int(kind)))

    def sub(self, text):
        for entry in self: text = entry.sub(text)
        return text


module = types.ModuleType('speechDictHandler')
module.SpeechDict, module.SpeechDictEntry = SpeechDict, Entry
sys.modules['speechDictHandler'] = module
from _reloq_nvda_dicts import ImportedDictionaries, dictionary_format, migrate_nvda_sources
from _reloq_dictionary_manager import DictionarySource


def main():
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        source = root / 'old-driver-German.dic'
        data = '# comment\nNVDA\tEn We De A\t0\t2\nfoo([0-9]+)\tbar\\1\t1\t1\n'.encode('utf-8-sig')
        source.write_bytes(data)
        manager = ImportedDictionaries(root / 'profile')
        copy, count, created = manager.import_file(source, 0x40000, 'all')
        assert created and count == 2 and copy.read_bytes() == data and source.read_bytes() == data
        assert manager.process('nvda foo42', 0x40000, '1') == 'En We De A bar42'
        assert manager.matches('nvda', 0x40000, '8')
        assert manager.process('nvda foo42', 0x10000, '1') == 'nvda foo42'
        assert dictionary_format(data) == 'nvda'
        assert dictionary_format(b'NVDA\tnormal reading\n') == 'eci'
        native = root / 'native.dic';native.write_bytes(b'anderes\tWort\n')
        accidental = DictionarySource(str(source), 0x40000, 0)
        correct = DictionarySource(str(native), 0x40000, 0)
        kept, migrated = migrate_nvda_sources((correct, accidental), manager)
        assert kept == (correct,) and migrated == (accidental,)
        assert manager.process('nvda foo42', 0x40000, '1') == 'En We De A bar42'
        assert source.read_bytes() == data and native.read_bytes() == b'anderes\tWort\n'
        assert len(manager.records) == 1, 'an already imported file must not be duplicated'
        explicit_native = DictionarySource(str(source), 0x40000, 0, format='eci')
        assert migrate_nvda_sources((explicit_native,), manager) == ((explicit_native,), ())
        malformed = b'word\treplacement\t0\t99\n'
        try: dictionary_format(malformed)
        except ValueError: pass
        else: raise AssertionError('invalid NVDA settings must not be interpreted as native replacement text')
        try: dictionary_format(data + b'native\tvalue\n')
        except ValueError: pass
        else: raise AssertionError('mixed NVDA and ECI files must not be silently misinterpreted')
        class FailedMigration:
            def __init__(self): self.created=[];self.removed=[]
            def import_file(self, path, language, voice):
                if self.created: raise OSError('simulated failure')
                self.created.append(Path(path));return self.created[-1], 1, True
            def remove(self, path): self.removed.append(path)
        rollback = FailedMigration()
        try: migrate_nvda_sources((accidental, accidental), rollback)
        except OSError: pass
        else: raise AssertionError('migration failure must be reported')
        assert rollback.removed == rollback.created and source.read_bytes() == data
        again, _, created = manager.import_file(source, 0x40000, 'all')
        assert again == copy and not created and len(manager.records) == 1
        other, _, created = manager.import_file(source, 0x10000, '2')
        assert created and manager.process('nvda', 0x10000, '1') == 'nvda'
        assert manager.process('nvda', 0x10000, '2') == 'En We De A'
        assert ImportedDictionaries(root / 'profile').process('nvda', 0x10000, '2') == 'En We De A'
        bad = root / 'bad.dic'
        bad.write_text('oops\tbad\t0\t99\n', encoding='utf-8')
        try: manager.import_file(bad, 0x40000, 'all')
        except ValueError: pass
        else: raise AssertionError('invalid NVDA entry must not be imported')
        assert len(manager.records) == 2 and source.read_bytes() == data
        copy.write_text('', encoding='utf-8')
        manager.reload()
        assert len(manager.records) == 2, 'an intentionally emptied dictionary remains editable'
        manager.remove(other)
        assert not other.exists() and source.read_bytes() == data
        empty = root / 'empty.dic'
        empty.write_text('# an empty speech dictionary\n', encoding='utf-8-sig')
        empty_copy, count, created = manager.import_file(empty, 0x40000, 'all')
        assert created and count == 0 and empty_copy.read_bytes() == empty.read_bytes()
        assert any(record[0] == empty_copy for record in manager.records)
        manager.remove(empty_copy)
    print('NVDA dictionary import: scope, preservation, regex, duplicate, validation and removal passed')


if __name__ == '__main__': main()
