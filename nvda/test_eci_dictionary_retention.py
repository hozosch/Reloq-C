"""Loaded dictionary data survives interrupted machines without reopening files.

Removing a test-owned source file after loading makes any accidental reload
observable in the synthesized pronunciation. PCM and indices stay in RAM.
"""
from pathlib import Path
import argparse
import hashlib
import tempfile
from _reloq_eci_direct import DirectEciEngine,EciIndex,NATIVE_SPEED,HEAD_SIZE

ROOT=Path(__file__).resolve().parents[1]
TEXT="This is cancellationtoken."


def capture(engine):
    digest=hashlib.sha256();size=[0];indices=[]
    def take(data):digest.update(data);size[0]+=len(data)
    assert engine.speak([TEXT,EciIndex(7)],take,lambda i:indices.append((i,size[0])))
    assert size[0] and indices==[(7,size[0])]
    return size[0],digest.hexdigest(),indices


def interrupt(engine,exception=False):
    class SinkError(RuntimeError):pass
    received=[]
    def take(data):
        if not received:
            received.append(len(data))
            if exception:raise SinkError("test output callback failure")
            engine.cancel()
    try:
        completed=engine.speak([TEXT+" This is a longer following sentence with more words."],take,lambda i:None)
    except SinkError:
        assert exception
    else:assert not completed and not exception
    assert received


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--library",type=Path,default=ROOT/"eci/libreloq_eci.so")
    a=p.parse_args();checks=0
    with tempfile.TemporaryDirectory() as temporary:
        first=Path(temporary)/"first";first.mkdir()
        second=Path(temporary)/"second";second.mkdir()
        source=first/"ENUmain.dic";source.write_bytes(b"cancellationtoken\tbanana\n")
        (second/"ENUmain.dic").write_bytes(b"cancellationtoken\torange\n")
        engine=DirectEciEngine(a.library,dialect=0x10000,encoding="utf8",experimental_generic=True)
        try:
            engine.set_parameter(NATIVE_SPEED,98);engine.set_dictionary_path(first)
            expected=capture(engine)
            source.rename(first/"not-a-dictionary.hidden")
            for _ in range(12):
                interrupt(engine);assert capture(engine)==expected,"Cancel discarded the loaded dictionary"
                checks+=1
            interrupt(engine,exception=True)
            assert capture(engine)==expected,"Callback failure discarded the loaded dictionary";checks+=1
            engine.set_parameter(HEAD_SIZE,22)
            changed=capture(engine);assert changed!=expected
            interrupt(engine);assert capture(engine)==changed;checks+=1
            engine.set_parameter(HEAD_SIZE,50)
            assert capture(engine)==expected;checks+=1
            try:engine.set_dictionary_path(first/"missing-directory")
            except RuntimeError:pass
            else:raise AssertionError("Invalid dictionary directory accepted")
            assert capture(engine)==expected;checks+=1
            engine.set_dictionary_path(second)
            updated=capture(engine);assert updated!=expected,"New entries were not applied"
            (second/"ENUmain.dic").unlink()
            interrupt(engine);assert capture(engine)==updated;checks+=1
            engine.set_dictionary_path(None)
            assert capture(engine)!=updated,"Disabling retained an active dictionary";checks+=1
        finally:engine.close()
    print(f"Dictionary retention: {checks} cancel/error/profile/update/disable checks passed with source files absent; no audio files")


if __name__=="__main__":main()
