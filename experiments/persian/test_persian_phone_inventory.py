"""Exercise the structured Persian phones and their existing sound bridge."""
from __future__ import annotations
import argparse
import ctypes as C
import os
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]

class Phone(C.Structure):
    _fields_ = [('id', C.c_uint32), ('flags', C.c_uint32),
                ('syllable', C.c_uint32), ('stress', C.c_uint32)]

class Info(C.Structure):
    _fields_ = [('id', C.c_uint32), ('category', C.c_uint32),
                ('name', C.c_char_p), ('ipa', C.c_char_p)]


def install(lib):
    lib.eciPersianPhoneInventory.argtypes = [C.POINTER(C.c_size_t)]
    lib.eciPersianPhoneInventory.restype = C.POINTER(Info)
    lib.eciPersianPhonesFromLegacy.argtypes = [C.c_char_p,C.c_size_t,C.POINTER(Phone),C.c_size_t,
                                              C.POINTER(C.c_size_t),C.POINTER(C.c_size_t)]
    lib.eciPersianPhonesToLegacy.argtypes = [C.POINTER(Phone),C.c_size_t,C.c_void_p,C.c_size_t,
                                            C.POINTER(C.c_size_t),C.POINTER(C.c_size_t)]
    lib.eciPersianWordPhonesUtf8.argtypes = [C.c_char_p,C.c_size_t,C.c_int,C.POINTER(Phone),C.c_size_t,
                                             C.POINTER(C.c_size_t),C.POINTER(C.c_size_t)]
    for name in ('eciPersianPhonesFromLegacy','eciPersianPhonesToLegacy','eciPersianWordPhonesUtf8'):
        getattr(lib,name).restype=C.c_int


def decode(lib, tag):
    text=tag.encode('ascii'); n=C.c_size_t(); err=C.c_size_t()
    status=lib.eciPersianPhonesFromLegacy(text,len(text),None,0,C.byref(n),C.byref(err))
    assert status==0,(tag,status,err.value)
    phones=(Phone*n.value)()
    status=lib.eciPersianPhonesFromLegacy(text,len(text),phones,len(phones),C.byref(n),C.byref(err))
    assert status==0,(tag,status,err.value)
    return phones


def encode(lib, phones):
    n=C.c_size_t(); err=C.c_size_t()
    status=lib.eciPersianPhonesToLegacy(phones,len(phones),None,0,C.byref(n),C.byref(err))
    assert status==0,(status,err.value)
    buf=C.create_string_buffer(n.value+1)
    status=lib.eciPersianPhonesToLegacy(phones,len(phones),buf,len(buf),C.byref(n),C.byref(err))
    assert status==0,(status,err.value)
    return buf.value.decode('ascii')


def word(lib, text, pinglish):
    raw=text.encode('utf8'); n=C.c_size_t(); err=C.c_size_t()
    status=lib.eciPersianWordPhonesUtf8(raw,len(raw),pinglish,None,0,C.byref(n),C.byref(err))
    assert status==0,(text,status,err.value)
    phones=(Phone*n.value)()
    assert lib.eciPersianWordPhonesUtf8(raw,len(raw),pinglish,phones,len(phones),C.byref(n),C.byref(err))==0
    return phones


def values(phones):
    return [(p.id,p.flags,p.syllable,p.stress) for p in phones]


def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--library',required=True,type=Path); args=ap.parse_args()
    lib=(C.WinDLL if os.name=='nt' else C.CDLL)(str(args.library.resolve()));install(lib)
    for name in ('reloq_fa_phone_inventory','reloq_fa_phones_from_legacy','reloq_fa_phones_to_legacy'):
        try:
            getattr(lib,name)
        except AttributeError:
            pass
        else:
            raise AssertionError(f'internal symbol unexpectedly exported: {name}')
    n=C.c_size_t(); info=lib.eciPersianPhoneInventory(C.byref(n))
    assert n.value==29
    names={info[i].name.decode():info[i].id for i in range(n.value)}
    assert set(info[i].id for i in range(n.value))==set(range(1,30))
    assert sum(info[i].category==1 for i in range(n.value))==6
    assert sum(info[i].category==2 for i in range(n.value))==23
    assert len(names)==29
    assert {names[k] for k in ('front-a','back-a','uvular','glottal-stop','kh','h','ch','j','r','y')}=={1,2,13,14,21,22,23,24,28,29}

    # Preserve the listener-selected existing alphabet and final-H carrier.
    for tag in ('`[.0sA.1lam]', "`[.1'i:'.0r'a:'n]", '`[.1solhw]', '`[.0sA.1wAt]',
                '`[.1Ci]', '`[.1Ja]', '`[.1Ga]', '`[.1Ra]', '`[.1ja]', '`[.1sA.0ssA]'):
        assert encode(lib,decode(lib,tag))==tag
    assert [p.id for p in decode(lib,'`[.1solhw]')]==[names['s'],names['o'],names['l'],names['h']]
    assert names['glottal-stop'] in [p.id for p in decode(lib,'`[.0sA.1wAt]')]
    assert [p.id for p in decode(lib,'`[.1sA.0ssA]')].count(names['s'])==3

    for fa,latin in [('سلام','salaam'),('ایران','iraan'),('صلح','solh')]:
        a,b=word(lib,fa,0),word(lib,latin,1)
        assert values(a)==values(b),(fa,latin,values(a),values(b))
    for source,pinglish in [('سلام دنیا',0),('physics',1)]:
        raw=source.encode('utf8');n=C.c_size_t();err=C.c_size_t()
        sentinel=(Phone*1)(Phone(77,77,77,77))
        assert lib.eciPersianWordPhonesUtf8(raw,len(raw),pinglish,sentinel,1,C.byref(n),C.byref(err))==1
        assert sentinel[0].id==77 and n.value==0

    # Reject ambiguous final H and invalid syntax without touching output.
    n=C.c_size_t(); err=C.c_size_t(); sentinel=(Phone*5)(*[Phone(77,77,77,77) for _ in range(5)])
    for tag,expected in [('`[.1sAh]',1),('`[.1sAhw.0a]',1),('`[.3a]',-1),
                         ('`[.1aA]',-1),('`[.1a',-1),('`[.1xa]',1),('`[.1\'t:\']',-1)]:
        raw=tag.encode(); status=lib.eciPersianPhonesFromLegacy(raw,len(raw),sentinel,5,C.byref(n),C.byref(err))
        assert status==expected,(tag,status,err.value)
        assert all(tuple((p.id,p.flags,p.syllable,p.stress))==(77,77,77,77) for p in sentinel)
    src=b'`[.0sA.1lam]'
    assert lib.eciPersianPhonesFromLegacy(src,len(src),sentinel,1,C.byref(n),C.byref(err))==-2
    assert n.value==5 and sentinel[0].id==77
    arr=decode(lib,src.decode()); buf=C.create_string_buffer(b'unchanged',30)
    assert lib.eciPersianPhonesToLegacy(arr,len(arr),buf,2,C.byref(n),C.byref(err))==-2
    assert n.value==len(src) and buf.value==b'unchanged'
    cases=[(Phone(names['s'],1,0,1),Phone(names['front-a'],0,0,1)),
           (Phone(names['front-a'],0,1,1),),
           (Phone(names['h'],0,0,1),Phone(names['glottal-stop'],0,0,1),Phone(names['front-a'],0,0,1))]
    for items in cases:
        arr=(Phone*len(items))(*items); buf=C.create_string_buffer(b'unchanged',30)
        status=lib.eciPersianPhonesToLegacy(arr,len(arr),buf,len(buf),C.byref(n),C.byref(err))
        assert status in (-1,1) and buf.value==b'unchanged'

    # All distinct generated legacy tags must survive a lossless round trip.
    tags=set()
    for path in (ROOT/'experiments/persian').glob('direct_fa_*.inc'):
        tags.update(re.findall(r'`\[[^\]]+\]',path.read_text(encoding='utf8')))
    assert len(tags)>40000,len(tags)
    for tag in tags:
        assert encode(lib,decode(lib,tag))==tag,tag
    print(f'Passed 29 phone identities, Persian/Pinglish words, guarded errors, and {len(tags):,} distinct existing pronunciation tags.')

if __name__=='__main__':main()
