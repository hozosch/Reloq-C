"""Fuse exact fixed-point ratios before a range-losing 32-bit temporary.

The original evaluates both operations in floating point. Keep their full
numerator in the existing int64 C ratio primitive instead. The intermediate
assignment is private and unused: the next operation overwrites it. A
separate denominator must not alias that destination. No branch, rounding
or side effect occurs between the matched operations.
"""
import re
from math import gcd
_PAIR=re.compile(
 r'    r1 = \(delta_muldiv\((?P<x>[^\n]+), 10000, 1, 0\)\);\n'
 r'    (?P<dest>[^\n=]+) = \(r1\);\n'
 r'    r1 = \(delta_muldiv\((?P=x), 10000, (?P<den>[^\n]+), 1\)\);\n'
 r'    (?P=dest) = \(r1\);')
def safe_denominator(den,dest):
    if re.fullmatch(r'-?\d+',den):return True
    if re.fullmatch(r'\(int32_t\)reloq_local_\d+\.i32',den):return den.removeprefix('(int32_t)')!=dest.strip()
    a=re.fullmatch(r'\(int32_t\)AT\(int32_t, (-?\d+)\)',den)
    b=re.fullmatch(r'AT\(int32_t, (-?\d+)\)',dest.strip())
    return bool(a and b and abs(int(a[1])-int(b[1]))>=4)
def apply(text):
    count=0
    def fuse(m):
        nonlocal count
        if not safe_denominator(m['den'],m['dest']):return m[0]
        den=m['den'];num=100000000
        if re.fullmatch(r'-?\d+',den):
            d=int(den);g=gcd(num,abs(d));num//=g;den=str(d//g)
        count+=1
        return ('    /* One native ratio retains the full intermediate range. */\n'
            '    r1 = (delta_muldiv('+m['x']+', '+str(num)+', '+den+', 1));\n'
            '    '+m['dest']+' = (r1);')
    return _PAIR.sub(fuse,text),count
def convert_bodies(bodies):
    return [(name,apply(body)[0])for name,body in bodies]
