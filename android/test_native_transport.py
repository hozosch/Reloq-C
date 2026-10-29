"""Host JVM/JNI integration against the real core; compare streaming PCM hashes.

This does not emulate Android or replace a device test. It catches JNI/control,
UTF-8, backpressure, cancellation and engine-lifetime errors without audio files.
"""
from pathlib import Path
import argparse
import hashlib
import os
import subprocess
import sys
import tempfile

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/"nvda"))
from _reloq_eci_direct import DirectEciEngine
from _reloq_languages import LANGUAGES


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--java-home",type=Path,default=os.environ.get("JAVA_HOME"))
    p.add_argument("--library",type=Path,default=ROOT/"eci/libreloq_eci.so")
    a=p.parse_args()
    if not a.java_home:p.error("--java-home is required")
    java=a.java_home.resolve()
    with tempfile.TemporaryDirectory() as temporary:
        build=Path(temporary)
        sys.path.insert(0,str(ROOT/"addon"))
        from build_synth import merged_german_lexicon
        lexicon=build/"german.rlex";lexicon.write_bytes(merged_german_lexicon())
        from codegen import generate
        generate(build/"generated",lexicon.read_bytes())
        library=a.library.resolve()
        folders=[]
        for name,value in [('one',b'banana'),('two',b'orange')]:
            folder=build/name;folder.mkdir();folders.append(folder)
            (folder/"ENUmain.dic").write_bytes(b'cancellationtoken\t'+value+b'\n')
            (folder/"DEUmain.dic").write_bytes(b'TokenX\tbei\n')
        subprocess.run(["cc","-std=c11","-Wall","-Wextra","-Werror","-fPIC","-shared",
            "-I"+str(java/"include"),"-I"+str(java/"include/linux"),"-I"+str(ROOT/"include"),
            str(ROOT/"android/native/reloq_jni.c"),str(library),
            "-pthread","-Wl,-rpath,"+str(library.parent),"-o",str(build/"libreloq_android.so")],check=True)
        sources=[*sorted((ROOT/"android/java").rglob("PcmQueue.java")),
                 *sorted((ROOT/"android/java").rglob("NativeEngine.java")),
                 *sorted((ROOT/"android/java").rglob("Catalog.java")),
                 *sorted((ROOT/"android/java").rglob("VoiceSettings.java")),
                 *sorted((build/"generated").rglob("RateCurve.java")),ROOT/"android/test/NativeTransportTest.java"]
        subprocess.run([str(java/"bin/javac"),"-encoding","UTF-8","-d",str(build),*map(str,sources)],check=True)
        result=subprocess.run([str(java/"bin/java"),"-Djava.library.path="+str(build),"-cp",str(build),
            "org.reloq.tts.preview.NativeTransportTest",str(lexicon),*map(str,folders)],check=True,stdout=subprocess.PIPE,text=True,timeout=180)
        checks=0
        for row in result.stdout.splitlines():
            dialect,controls,length,expected,dictionaries,text=row.split("\t",5)
            language=LANGUAGES[int(dialect)]
            with_engine=DirectEciEngine(library,dialect=language.id,encoding="utf8",
                experimental_generic=True,lexicon_path=lexicon if language.lexicon else None)
            try:
                if dictionaries!="-":with_engine.set_dictionary_path(dictionaries)
                keys=(0x1010,0x100d,0x1008,0x1009,0x100e,0x100f,0x100a,0x1007,5,0x100c,0x100b)
                for parameter,value in zip(keys,map(int,controls.split(','))):
                    with_engine.set_parameter(parameter,value)
                digest=hashlib.sha256();size=[0]
                def take(data):digest.update(data);size[0]+=len(data)
                assert with_engine.speak([text],take,lambda i:None)
                assert (size[0],digest.hexdigest())==(int(length),expected),(language.name,controls,text)
            finally:with_engine.close()
            checks+=1
        print(f"JNI integration: {checks} native PCM hashes identical; cancellation and subsequent output passed; no audio files")


if __name__=="__main__":main()
