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
_NUCLEUS_SPEED = '''    r1 = (delta_muldiv((int32_t)AT(int32_t, -12), (int32_t)STATE(int32_t, f4), 10000, 1));
    AT(int32_t, -12) = (r1);
    r1 = (delta_muldiv((int32_t)AT(int32_t, -12), 10000, 1000000, 1));'''
_NUCLEUS_SPEED_WIDE = '''    r1 = delta_scale_percent((int32_t)AT(int32_t, -12), (int32_t)STATE(int32_t, f4));'''
_SPEED_ANNOTATION = re.compile(
    r'    r1 = \(delta_muldiv\((?P<a>\(int32_t\)AT\(int32_t, -?\d+\)), '
    r'(?P<b>\(int32_t\)AT\(int32_t, -?\d+\)), 10000, 1\)\);\n'
    r'    (?P<dst>AT\(int32_t, -?\d+\)) = \(r1\);\n'
    r'    r1 = \(delta_muldiv\(\(int32_t\)(?P=dst), 10000, 1000000, 1\)\);'
)

def wide_speed_annotation(body):
    def convert(m):
        if m['a'] != '(int32_t)' + m['dst']:
            return m[0]
        return '    r1 = delta_scale_percent(' + m['a'] + ', ' + m['b'] + ');'
    return _SPEED_ANNOTATION.sub(convert,body)

def convert_bodies(bodies, language=None):
    converted=[]
    for name,body in bodies:
        body=apply(body)[0]
        # The nucleus duration uses the same percentage as insert_dur. At
        # low VWPM its temporary can overflow despite a valid final value.
        if name.endswith('distribute_nucdur'):
            body=body.replace(_NUCLEUS_SPEED,_NUCLEUS_SPEED_WIDE)
        elif name.endswith('apply_speed_anno'):
            body=wide_speed_annotation(body)
        if language in ('zhcn61', 'jajp61', 'kokr61'):
            if name.endswith('distribute_nucdur'):
                body=body.replace('(int32_t)STATE(int32_t, f4));',
                    'reloq61_cjk_duration_factor(state, (int32_t)STATE(int32_t, f4)));')
            elif name.endswith('apply_speed_anno'):
                body=re.sub(r'(r1 = delta_scale_percent\([^\n]*, )'
                            r'(\(int32_t\)AT\(int32_t, -?\d+\))(\);)',
                            r'\1reloq61_cjk_duration_factor(state, \2)\3', body)
            elif name.endswith('wpm_anno'):
                body=body.replace('STATE(int32_t, f21) = ((int32_t)AT(int32_t, -20));',
                    'STATE(int32_t, f21) = reloq61_cjk_wpm_factor(state, '
                    '(int32_t)AT(int32_t, -20), (int32_t)STATE(int32_t, l1));')
        converted.append((name,body))
    return converted
