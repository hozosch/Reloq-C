"""Express the original pitch window directly, without expanded temporaries."""
import re
_START=r'(?P<indent>[ ]+)r1 = \(\(int32_t\)STATE_D\(int16_t, c14, (?P<inflection>28|36)\)\);'
_END=r'STATE\(int32_t, f42\) = \(\(int32_t\)[^\n]+\);'
_PATTERN=re.compile(_START+r'(?P<body>.*?)'+_END,re.S)
def apply(text):
    if not re.search(r'\breloq_\w+_set_f0_wind_and_base\(',text):return text,0
    def compact(m):
        b=m[0]
        assert b.count('STATE(int32_t, f43) =')==1
        assert b.count('STATE(int32_t, f42) =')==1
        base_offset=36 if m['inflection']=='28'else 28
        assert f'STATE_D(int32_t, c14, {base_offset})'in b
        assert '1000000'in b and '-90000'in b and '100000'in b
        assert not re.search(r'\br0\s*=|reloq61_typed_|\bCALL\(',b)
        p=m['indent']
        return ('{\n'+p+'    /* Original range = inflection * baseline / 100;\n'
            +p+'     * keep only the final fixed-point targets in int32. */\n'
            +p+f'    const int32_t pitch_base = STATE_D(int32_t, c14, {base_offset});\n'
            +p+f'    const int32_t pitch_range = delta_muldiv(STATE_D(int16_t, c14, {m["inflection"]}), pitch_base, 100, 1);\n'
            +p+'    STATE(int32_t, f43) = pitch_range;\n'
            +p+'    STATE(int32_t, f42) = pitch_base + delta_muldiv(pitch_range, -9, 10, 1);\n'
            +p+'}')
    return _PATTERN.subn(compact,text)
def convert_bodies(bodies):
    return [(name,apply(body)[0])for name,body in bodies]
