"""Check isolated eSpeak analysis, concurrent voices and cancellation."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import sys
import tempfile
import threading
import time

from espeak_ng_probe import EspeakPhonemeProbe
import espeak_worker
from espeak_worker import EspeakWorker
from text_frontend import UnsupportedPronunciation, translate_text


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--worker', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    cases = (('fa', 'سلام. او آمد!'), ('fa', 'ایران، نقطه و دوازده.'),
             ('fa', 'آیا NVDA خوب است؟'), ('fa', 'من ساعت دارم.'),
             ('fa', 'یک کتاب، دو مداد و یک دفتر.'),
             ('fa-latn', 'salam donya'), ('fa-latn', 'noqte'),
             ('en-us', 'NVDA is reading English.'), ('en-us', 'one, two, three?'))
    with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as reference:
        expected = {(voice, text, separator): reference.translate(text, voice=voice, separator=separator)
                    for voice, text in cases for separator in ('', '|')}
        translated = {}
        for voice, text in cases:
            if voice != 'fa':
                continue
            try:
                translated[text] = translate_text(reference, text)
            except UnsupportedPronunciation:
                pass
        corpus = []
        for line in Path(__file__).with_name('reference_espeak_1_51.jsonl').read_text(encoding='utf8').splitlines():
            row = json.loads(line)
            if row['voice'] not in ('fa', 'fa-latn'):
                continue
            try:
                value = translate_text(reference, row['text'], pinglish=row['voice'] == 'fa-latn')
            except UnsupportedPronunciation as error:
                value = ('unsupported', str(error))
            corpus.append((row, value))

    with EspeakWorker(args.worker, args.espeak_library, args.data_parent) as worker:
        for (voice, text, separator), result in expected.items():
            assert worker.translate(text, voice=voice, separator=separator) == result
        for voice, text in cases:
            normal = worker.events(text, voice=voice, normal_speed=True)
            assert worker.events(text, voice=voice) == normal, (voice, text)
            # Event acceleration must not change the following pronunciation.
            assert worker.translate(text, voice=voice, separator='|') == expected[voice, text, '|']
        for text, result in translated.items():
            assert translate_text(worker, text) == result
        for row, expected_mapping in corpus:
            try:
                value = translate_text(worker, row['text'], pinglish=row['voice'] == 'fa-latn')
            except UnsupportedPronunciation as error:
                value = ('unsupported', str(error))
            assert value == expected_mapping, row['text']
        # Concurrent calls alternate voices on the same serialized worker.
        def one_case(key):
            voice, text, separator = key
            return worker.translate(text, voice=voice, separator=separator)
        with ThreadPoolExecutor(max_workers=3) as pool:
            keys = tuple(expected) * 2
            assert list(pool.map(one_case, keys)) == [expected[key] for key in keys]
        # An idle stop must not destroy the initialized helper or force a
        # process/DLL startup on the next unrelated utterance.
        process = worker._process
        assert process is not None and process.poll() is None
        worker.cancel()
        assert worker.translate('سلام. او آمد!', separator='|') == expected['fa', 'سلام. او آمد!', '|']
        assert worker._process is process and process.poll() is None
    try:
        worker.translate('')
        raise AssertionError('closed worker accepted a request')
    except RuntimeError:
        pass

    # A nonresponding child must be interruptible while another thread is
    # blocked in read, on both Windows and POSIX.
    with tempfile.TemporaryDirectory(prefix='reloq-espeak-cancel-') as name:
        folder = Path(name)
        helper = folder / 'blocked.py'
        marker = folder / 'started'
        helper.write_text('from pathlib import Path\nimport sys,time\n'
                          '(Path(sys.argv[1])/"started").write_text("ready")\n'
                          'time.sleep(60)\n', encoding='utf8')
        blocked = EspeakWorker(Path(sys.executable), helper, folder)
        caught = []
        def pending():
            try:
                blocked.translate('سلام')
            except (RuntimeError, OSError) as error:
                caught.append(error)
        thread = threading.Thread(target=pending)
        thread.start()
        deadline = time.monotonic() + 5
        while not marker.exists() and time.monotonic() < deadline:
            time.sleep(.01)
        assert marker.exists(), 'blocked worker failed to start'
        blocked.cancel()
        thread.join(5)
        assert not thread.is_alive() and caught, 'cancellation failed to unblock translation'
        blocked.close()

        # A late error from the stopped request must not cancel a newer
        # request already queued for the same worker.
        marker.unlink()
        interrupted, release, queued = threading.Event(), threading.Event(), threading.Event()
        original_read = espeak_worker._read_exact
        def delayed_error(stream, count):
            try:
                return original_read(stream, count)
            except RuntimeError:
                interrupted.set()
                if not release.wait(5):
                    raise AssertionError('cancelled read did not resume')
                raise
        class ObservedLock:
            def __init__(self):
                self.lock = threading.Lock()
            def __enter__(self):
                if threading.current_thread().name == 'new-espeak-request':
                    queued.set()
                self.lock.acquire()
            def __exit__(self, *_args):
                self.lock.release()
        blocked = EspeakWorker(Path(sys.executable), helper, folder)
        blocked._calls = ObservedLock()
        caught, fresh, fresh_errors = [], [], []
        def new_request():
            try:
                fresh.append(blocked.translate('سلام. او آمد!', separator='|'))
            except Exception as error:
                fresh_errors.append(error)
        espeak_worker._read_exact = delayed_error
        thread = threading.Thread(target=pending)
        successor = threading.Thread(target=new_request, name='new-espeak-request')
        try:
            thread.start()
            deadline = time.monotonic() + 5
            while not marker.exists() and time.monotonic() < deadline:
                time.sleep(.01)
            assert marker.exists(), 'second blocked worker failed to start'
            blocked.cancel()
            assert interrupted.wait(5), 'cancelled request did not reach its error handler'
            blocked._paths = tuple(str(p.resolve()) for p in (args.worker, args.espeak_library, args.data_parent))
            successor.start()
            assert queued.wait(5), 'new request did not reach the call lock'
            release.set()
            thread.join(5)
            successor.join(5)
            assert not thread.is_alive() and not successor.is_alive()
            assert caught and not fresh_errors, (caught, fresh_errors)
            assert fresh == [expected['fa', 'سلام. او آمد!', '|']]
        finally:
            release.set()
            if thread.is_alive(): thread.join(5)
            if successor.is_alive(): successor.join(5)
            espeak_worker._read_exact = original_read
            blocked.close()
    unsupported = sum(isinstance(value, tuple) and len(value) == 2 and value[0] == 'unsupported'
                      for _, value in corpus)
    print(f'Passed {len(expected)} direct comparisons and {len(corpus)} corpus mappings '
          f'({unsupported} explicitly unsupported); concurrent voices and cancellation passed.')


if __name__ == '__main__':
    main()
