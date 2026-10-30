"""Test native Chinese identities and Greek English word input; no audio files."""
from pathlib import Path
import argparse
import ctypes
import json
import re
import subprocess
import tempfile
import sys
sys.path.insert(0,str(Path(__file__).resolve().parent))
from update_chinese_traditional import generate,DATA

ROOT=Path(__file__).resolve().parents[1]


def compile_rom(target,source):
    vendor=ROOT/"vendor/openevv61"
    subprocess.run(["cc","-O2","-std=gnu11","-shared","-fPIC",
        "-I"+str(vendor/"include"),"-I"+str(ROOT/"include"),
        "-I"+str(vendor/"src/port"),
        "-I"+str(vendor/"rom/greek"),
        "-I"+str(vendor/"src/eci/lang"),"-I"+str(vendor/"rom/cjk"),
        str(ROOT/"tests/native_text_input_host.c"),str(source),str(vendor/"rom/greek/reloq_greek.c"),
        str(vendor/"rom/cjk/reloq_pinyin.c"),str(vendor/"src/reloq61_latin.c"),
        "-o",str(target)],check=True)
    lib=ctypes.CDLL(str(target));lib.romanize.argtypes=(ctypes.c_char_p,ctypes.c_size_t,ctypes.c_size_t,ctypes.c_void_p,ctypes.c_size_t)
    lib.romanize.restype=ctypes.c_int
    def run(text,split=None):
        raw=text.encode();out=ctypes.create_string_buffer(32768)
        assert lib.romanize(raw,len(raw),len(raw) if split is None else split,out,len(out))>=0,text
        return out.value.decode("cp1252")
    lib.greek_english_words.argtypes=(ctypes.c_char_p,ctypes.c_void_p,ctypes.c_size_t)
    lib.greek_english_words.restype=ctypes.c_int
    def greek_words(text):
        output=ctypes.create_string_buffer(32768)
        assert lib.greek_english_words(text.encode(),output,len(output))>=0,text
        return output.value.split(b'\t')[:-1]
    return run,greek_words


def readings(text):return re.findall(r"[a-z]+[0-5]",re.sub(r"`g[^_\s]+_","",text))


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--baseline-source",type=Path)
    a=p.parse_args();checks=0
    with tempfile.TemporaryDirectory() as folder:
        current,greek_words=compile_rom(Path(folder)/"current.so",ROOT/"vendor/openevv61/rom/cjk/reloq_cjk_romanizer.c")
        old=compile_rom(Path(folder)/"old.so",a.baseline_source)[0] if a.baseline_source else None
        expected={"發":["fa1"],"髮":["fa4"],"隻":["zhi1"],"只":["zhi3"],
            "臟":["zang4"],"髒":["zang1"],"癥":["zheng1"],"症":["zheng4"],
            "頭髮":["tou2","fa0"],"理髮":["li3","fa4"],"發展":["fa1","zhan3"],
            "發現":["fa1","xian4"],"幹部":["gan4","bu4"],"乾坤":["qian2","kun1"]}
        for word,pronunciation in expected.items():
            result=current(word);assert readings(result)==pronunciation,(word,result,pronunciation)
            for cut in range(len(word.encode())+1):
                assert current(word,cut)==result,(word,cut);checks+=1
        for traditional,simplified in [("甲殼","甲壳"),("纖維","纤维"),("頭髮","头发"),
            ("發現","发现"),("幹部","干部")]:
            assert readings(current(traditional))==readings(current(simplified)),traditional;checks+=1
        table,_=generate(json.loads(DATA.read_text()))
        for cp,options in re.findall(r'\{(\d+),"([^"]+)"\}',table):
            result=readings(current(chr(int(cp))));assert len(result)==1 and result[0] in options.split(),(cp,result,options);checks+=1
        name=current("G");assert current("g")==name and current("ｇ")==name;checks+=2
        assert re.sub(r"`ts[012]","",current("`ts1 g")).strip()==name.strip();checks+=1
        if old:
            assert old("g")!=name and readings(old("髮"))!=["fa4"];checks+=2
            for word in ["5g","5 g","５ｇ","50g 12kg","Hello world.","ni3 hao3 ma0?", "503和4004", "中文abc123 test."]:
                assert current(word)==old(word),(word,current(word),old(word));checks+=1
        for word in ["Müller","ÄÖÜäöü","café","naïve","élève","façade","Straße","Œuvre"]:
            expected_word=word.encode('cp1252')
            assert greek_words(word)==[expected_word],word
            assert greek_words("Γεια "+word+", κόσμε.")==[expected_word],word;checks+=2
        assert greek_words("Müller café and naive's façade.")==[b'M\xfcller',b'caf\xe9',b'and',b"naive's",b'fa\xe7ade'];checks+=1
        print(f"Native text input: {checks} Chinese identity/context/letter/unit/fragment and Greek accent-word checks passed")


if __name__=="__main__":main()
