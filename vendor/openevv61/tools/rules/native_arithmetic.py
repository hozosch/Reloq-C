"""Retire unused flag arithmetic at the remaining C caller boundary.

This offline pass reasons about C branches and individual live flag fields.
It emits ordinary unsigned C arithmetic and switches, never runtime opcodes.
Unrecognized control flow or live arithmetic flags keep their existing path.
"""
import re
from collections import deque

FLAGS=frozenset(('zf','sf','cf','of'))
READ={'e':{'zf'},'ne':{'zf'},'s':{'sf'},'ns':{'sf'},
      'a':{'cf','zf'},'ae':{'cf'},'b':{'cf'},'be':{'cf','zf'},
      'g':{'zf','sf','of'},'ge':{'sf','of'},'l':{'sf','of'},'le':{'zf','sf','of'}}
PARTIAL={'incl':{'zf','sf','of'},'incw':{'zf','sf','of'},
         'decl':{'zf','sf','of'},'decw':{'zf','sf','of'},
         'shll':{'zf','sf'},'shlw':{'zf','sf'},'sarl':{'zf','sf'},'sarw':{'zf','sf'},
         'imull':set(),'imulw':set()}
FUNCTION=re.compile(r'(static int32_t evv_\w+\(void \*state, const int32_t \*args, int nargs\)\n\{\n)(.*?)(\n\})',re.S)
ASSIGN=re.compile(r'^(\s*)(.*?) = \(ALU\((\w+), (.*)\)\);$')
PAIR=re.compile(r'^\s*if \(IF\((e|ne|s|ns)\)\) goto (L\d+);$')
LABEL=re.compile(r'^\s*(L\d+):;\s*$')
BRANCH=re.compile(r'^\s*if \(.*\) goto (L\d+);\s*$')
JUMP=re.compile(r'^\s*goto (L\d+);\s*$')
CMP=re.compile(r'^(\s*)CMP\((test[blw]|cmp[blw]), (.*)\);$')
FLAG_BRANCH=re.compile(r'^\s*if \(IF\((\w+)\)\) goto (L\d+);$')

def operands(text):
    depth=0
    for i,c in enumerate(text):
        if c in '([':depth+=1
        elif c in ')]':depth-=1
        elif c==','and depth==0:return text[:i].strip(),text[i+1:].strip()
    raise ValueError('missing operand separator')

def expression(kind,a,b):
    """The existing masked result, with wraparound expressed in unsigned C."""
    width=16 if kind.endswith('w')else 32
    ua=f'(uint32_t)({a})';ub=f'(uint32_t)({b})'
    op=kind[:-1]
    if op in ('add','sub','and','or','imul'):
        value=f'{ub} '+{'add':'+','sub':'-','and':'&','or':'|','imul':'*'}[op]+f' {ua}'
    elif op in ('inc','dec'):value=f'{ub} '+('+'if op=='inc'else'-')+' 1u'
    elif op=='neg':value=f'0u - {ub}'
    elif op=='shl':value=f'{ub} << ({ua} & 31u)'
    elif op=='sar':
        # GCC and MinGW use arithmetic signed right shift, as does the
        # existing implementation. Preserve its explicit input narrowing.
        value=f'(uint32_t)((int{width}_t)({b}) >> ({ua} & 31u))'
    else:return None
    return '(int32_t)'+(''if width==32 else'(uint16_t)')+f'({value})'

def comparison(kind,a,b,condition):
    width={'b':8,'w':16,'l':32}[kind[-1]]
    signed=f'int{width}_t';unsigned=f'uint{width}_t'
    if kind.startswith('test'):
        result=f'({signed})((uint32_t)({a}) & (uint32_t)({b}))'
        op={'e':'==','ne':'!=','s':'<','ns':'>=','g':'>','ge':'>=','l':'<','le':'<=','a':'!=','be':'=='}
        if condition in op:return f'{result} {op[condition]} 0'
        if condition in ('ae','b'):return '1'if condition=='ae'else'0'
        return None
    if condition in ('s','ns'):
        result=f'({signed})((uint32_t)({b}) - (uint32_t)({a}))'
        return f'{result} '+('<'if condition=='s'else'>=')+' 0'
    op={'e':'==','ne':'!=','g':'>','ge':'>=','l':'<','le':'<=','a':'>','ae':'>=','b':'<','be':'<='}
    if condition not in op:return None
    cast=unsigned if condition in ('a','ae','b','be','e','ne')else signed
    return f'({cast})({b}) {op[condition]} ({cast})({a})'

def graph(lines):
    labels={m[1]:i for i,s in enumerate(lines)if(m:=LABEL.match(s))}
    edges=[]
    for i,line in enumerate(lines):
        t=line.strip();following=[i+1]if i+1<len(lines)else[]
        if t.startswith(('switch (','case ','default:','while (','do {','} else','else ')):return None
        if t.startswith('if (')and not BRANCH.match(line)and t!='if (frame == 0)':return None
        if t.startswith('for (')and t!='for (i = 0; i < nargs && i < '+str(_parameters(lines))+'; i++)':return None
        if m:=BRANCH.match(line):edges.append(following+[labels[m[1]]])
        elif m:=JUMP.match(line):edges.append([labels[m[1]]])
        elif t.startswith(('RETURN(','LEAVE(','return ')):edges.append([])
        else:edges.append(following)
    return edges

def _parameters(lines):
    for s in lines:
        m=re.fullmatch(r'\s*for \(i = 0; i < nargs && i < (\d+); i\+\+\)',s)
        if m:return int(m[1])
    return -1

def liveness(lines,edges):
    writes=[];reads=[];predecessors=[[]for _ in lines]
    for i,line in enumerate(lines):
        use=set();define=set()
        for cond in re.findall(r'\bIF\((\w+)\)',line):use.update(READ.get(cond,FLAGS))
        if m:=re.search(r'\bALU\((\w+),',line):
            define.update(PARTIAL.get(m[1],FLAGS))
            if m[1]=='sbbl':use.add('cf')
        if 'CMP('in line or 'memset(&fl,'in line:define.update(FLAGS)
        if 'LANDING('in line or 'ENTER('in line or 'EVV_LAND_SAVE('in line:
            use.update(FLAGS);define.update(FLAGS)
        if re.search(r'\bfl\.',line):use.update(FLAGS)
        reads.append(use);writes.append(define)
        for j in edges[i]:predecessors[j].append(i)
    live=[set()for _ in lines];out=[set()for _ in lines]
    work=deque(range(len(lines)-1,-1,-1));queued=set(work)
    while work:
        i=work.popleft();queued.discard(i)
        after=set().union(*(live[j]for j in edges[i]))if edges[i]else set()
        before=reads[i]|(after-writes[i]);out[i]=after
        if before!=live[i]:
            live[i]=before
            for j in predecessors[i]:
                if j not in queued:queued.add(j);work.append(j)
    return out,writes

def body(text):
    lines=text.splitlines();edges=graph(lines)
    if edges is None:return text
    # A zero/sign condition after a full register assignment describes its
    # stored result exactly. Other flag consumers remain live in the graph.
    for i,line in enumerate(lines[:-1]):
        m=ASSIGN.match(line);p=PAIR.match(lines[i+1])
        if not m or not p or not re.fullmatch(r'r[0-7]',m[2]):continue
        if m[3][:-1]not in ('add','sub','and','or','inc','dec','neg','shl','sar'):continue
        if p[1]in ('s','ns'):value=f'(int{16 if m[3].endswith("w")else 32}_t){m[2]}'
        else:value=m[2]
        operator={'e':'==','ne':'!=','s':'<','ns':'>='}[p[1]]
        lines[i+1]=f'{m[1]}if ({value} {operator} 0) goto {p[2]};'
    for i,line in enumerate(lines[:-1]):
        m=CMP.match(line);p=FLAG_BRANCH.match(lines[i+1])
        if not m or not p:continue
        a,b=operands(m[3]);condition=comparison(m[2],a,b,p[1])
        if condition is not None:lines[i+1]=f'{m[1]}if ({condition}) goto {p[2]};'
    live,writes=liveness(lines,edges)
    for i,line in enumerate(lines):
        if CMP.match(line)and not live[i]&writes[i]:lines[i]='';continue
        m=ASSIGN.match(line)
        if not m or live[i]&writes[i]or m[3]=='sbbl':continue
        a,b=operands(m[4]);value=expression(m[3],a,b)
        if value is not None:lines[i]=f'{m[1]}{m[2]} = ({value});'
    # The frequent alternative-selection chain becomes a C switch. Preserve
    # the residual result on every exit, including default and unsigned wrap.
    out=[];i=0
    dec=re.compile(r'^    (r[0-7]) = \(\(int32_t\)\(\(uint32_t\)\(\1\) - 1u\)\);$')
    while i<len(lines):
        m=dec.match(lines[i]);j=i;arms=[]
        if m:
            while j+1<len(lines)and dec.match(lines[j])and dec.match(lines[j])[1]==m[1]:
                p=re.fullmatch(r'    if \('+m[1]+r' == 0\) goto (L\d+);',lines[j+1])
                if not p:break
                arms.append(p[1]);j+=2
        if len(arms)>=2:
            out.append(f'    switch ((uint32_t){m[1]}) {{')
            for k,label in enumerate(arms,1):out.append(f'    case {k}: {m[1]} = 0; goto {label};')
            out.append(f'    default: {m[1]} = (int32_t)((uint32_t){m[1]} - {len(arms)}u); break;')
            out.append('    }');i=j
        else:out.append(lines[i]);i+=1
    joined='\n'.join(out)
    # The save helper takes no flag argument. When all actual flag readers
    # have gone, its conservative barrier above can no longer keep an
    # otherwise unused flag initialization alive.
    if not re.search(r'\b(?:IF|LANDING|ENTER)\(|\bfl\.|ALU\(sbbl,',joined):
        def remaining(m):
            a,b=operands(m[4]);value=expression(m[3],a,b)
            return m[0]if value is None else f'{m[1]}{m[2]} = ({value});'
        joined='\n'.join(remaining(m)if(m:=ASSIGN.match(line))else line
                         for line in joined.splitlines()if not CMP.match(line))
    if not re.search(r'\b(?:ALU|CMP|IF|LANDING|ENTER)\(',joined):
        joined=joined.replace('    delta_flags fl;\n','').replace('    memset(&fl, 0, sizeof fl);\n','')
    return joined

def bind(text):
    text=FUNCTION.sub(lambda m:m[1]+body(m[2])+m[3],text)
    return re.sub(r'(?s)^/\* Generated by tools/rules/decompile\.py\. Do not edit\..*?\*/',
        '/* Generated by tools/rules/decompile.py. Do not edit.\n\n'
        '   Remaining language rules compiled as C. Arithmetic, comparisons and\n'
        '   alternative selection use ordinary C expressions and switches.\n'
        '   The transitional record, argument and rule-frame boundary remains\n'
        '   while complete linguistic blocks are moved to authored C modules.\n'
        '   Native entry metadata is collected from these translation units. */',text,count=1)
