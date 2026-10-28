"""Keep the unscaled level/range product wide until percentage division."""
import re
_FUNCTION=re.compile(r'^int32_t reloq_\w+_insert_f0\([^\n]*\)\n\{.*?^\}',re.M|re.S)
_BLOCK=re.compile(
 r'    r1 = \(\(int32_t\)AT\(int16_t, (?P<level>-\d+)\)\);\n'
 r'(?P<body>.*?)'
 r'    r1 = \(delta_muldiv\(\(int32_t\)AT\(int32_t, -12\), 10000, \(int32_t\)AT\(int32_t, (?P<decay>-\d+)\), 1\)\);\n'
 r'    AT\(int32_t, -12\) = \(r1\);\n'
 r'    AT\(int32_t, -8\) = [^\n]+\n'
 r'    r1 = \(delta_muldiv\(\(int32_t\)AT\(int32_t, -8\), 10000, 1000000, 1\)\);\n'
 r'    AT\(int32_t, -8\) = \(r1\);',re.S)
def apply(source):
    count=0
    def function(match):
        nonlocal count
        text=match[0];start=text.find('STATE(int32_t, f37) =')
        if start<0:return text
        def block(m):
            nonlocal count
            b=m['body']
            assert b.count('delta_muldiv(')==2
            assert 'STATE(int32_t, f42)'in b and '10000, 1, 0'in b
            assert not re.search(r'\br0\s*=|reloq61_typed_|\bCALL\(',b)
            count+=1
            return ('    /* Level * decayed range / 100, with no narrow product. */\n'
                +'    AT(int32_t, -8) = delta_muldiv(AT(int16_t, '+m['level']+'), STATE(int32_t, f37), 100, 1);\n'
                +'    AT(int32_t, -12) = delta_muldiv(STATE(int32_t, f42), 10000, AT(int32_t, '+m['decay']+'), 1);')
        return text[:start]+_BLOCK.sub(block,text[start:])
    result=_FUNCTION.sub(function,source)
    return result,count
def convert_bodies(bodies):
    return [(name,apply(body)[0])for name,body in bodies]
