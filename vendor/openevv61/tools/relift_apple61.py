"""Refresh every rule from the supplied Apple modules, keeping native C output.

This is an offline development tool. Module binaries are external inputs;
normal builds use the resulting C files and require neither modules nor Python.
Run tools/rebuild_native.py after refreshing the readable rules.
"""
from pathlib import Path
import argparse, hashlib, json, os, re, subprocess, sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from evv import ROOT, sibling

R = Path(ROOT)

def refresh(path, tag):
    module = sibling('module/lift61')
    lift = sibling('rules/lift64')
    # The supplied .up sources are sufficient to reconstruct call signatures.
    env = dict(os.environ, EVV_NOTATION_LANG=tag, EVV_LANG_DIR='lang/'+tag,
               EVV_FAST_C='1')
    subprocess.run([sys.executable, 'tools/rules/consts.py', tag],
                   cwd=R, check=True, stdout=subprocess.DEVNULL)
    subprocess.run([sys.executable, 'tools/rules/notation.py', 'build'],
                   cwd=R, env=env, check=True, stdout=subprocess.DEVNULL)
    lift.ARITY = lift.our_arities(tag)
    image = module.Module(str(path))
    machine = lift.Module(str(path))
    variables = module.varmap(image, tag)
    directory = R/'lang'/tag/'rules'
    files = {p: p.read_text() for p in directory.glob('*.up')}
    definitions = {}
    pattern = re.compile(r'^rule (\S+) takes \d+\n.*?^end$', re.M|re.S)
    for p, source in files.items():
        for match in pattern.finditer(source):
            assert match[1] not in definitions, match[1]
            definitions[match[1]] = p, match[0]
    constants = {}
    def gmap(offset, width):
        if offset not in variables:
            raise lift.Hole('unmapped Apple global %x' % offset)
        return str(variables[offset][0])
    def smap(address, name, data):
        if name == 'null_str':
            # Several translation units have their own empty string. The
            # next data symbol can leave a different amount of alignment
            # padding after it; only its terminating zero is a string.
            assert data and data[0] == 0, (name, address)
            data = b'\0'
        key = ('eti_null' if name == 'null_str' else
               'eti_fence_'+name[6:] if name and name.startswith('fence:') else
               'eti_s_%x' % address)
        assert constants.get(key, data) == data, key
        constants[key] = data
        return key
    report = []
    names = machine.rules()
    assert set(names) == set(definitions), (tag, set(names)^set(definitions))
    for ordinal, name in enumerate(names):
        rule = lift.Rule(machine, name)
        source = rule.lift(gmap, smap)
        p, previous = definitions[name]
        if source is None:
            # These two startup adapters were independently mapped in 0.2.102.
            # Never turn a failed translation into an executable placeholder.
            allowed = {'init_platform', 'set_phrase_defaults'}
            if tag == 'dede61': allowed.add('abbreviation')
            assert name in allowed, (name, rule.holes)
            status = 'retained-audited-native-adapter'
        else:
            status = 'translated'
            if source.strip() != previous.strip():
                assert files[p].count(previous) == 1
                files[p] = files[p].replace(previous, source.strip())
        report.append(dict(rule=name, status=status, source=p.name,
                           changed=bool(source and source.strip()!=previous.strip()),
                           diagnostic=rule.holes))
        if ordinal % 100 == 0:
            print(tag, ordinal, len(names), flush=True)
    for p, source in files.items():
        assert 'call unsupported61' not in source, p
        p.write_text(source)
    p = directory/'constants'
    old = p.read_text()
    held = {w[1]: bytes.fromhex(' '.join(w[2:]))
            for line in old.splitlines() if (w:=line.split()) and w[0]=='bytes'}
    for key, data in sorted(constants.items()):
        if key in held:
            assert held[key] == data, key
        else:
            old += 'bytes '+key+' '+data.hex(' ')+'\n'
    p.write_text(old)
    private_fences=[]
    probe=lift.Rule(machine,names[0])
    for address, name in machine.private.items():
        if name != 'fence': continue
        count=probe.private_fence_count(address)
        if probe.arity_of_private(address)==0:
            assert count is not None
            data=probe.private_fence_data(address,count)
            private_fences.append(dict(address=hex(address),count=count,characters=data[:-1].hex()))
    return dict(module_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                rules=report, specialised_fences=private_fences)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--english',type=Path)
    parser.add_argument('--german',type=Path)
    parser.add_argument('--module',action='append',default=[],metavar='TAG=PATH',
                        help='refresh an additional registered language')
    parser.add_argument('--report',type=Path,required=True)
    args=parser.parse_args()
    modules=[(tag,path)for tag,path in
             [('enus61',args.english),('dede61',args.german)]if path]
    for item in args.module:
        tag,sep,path=item.partition('=')
        if not sep or not re.fullmatch('[a-z][a-z0-9]*',tag) or not path:
            parser.error('--module requires TAG=PATH')
        modules.append((tag,Path(path)))
    if not modules:parser.error('at least one module is required')
    if len({tag for tag,_ in modules})!=len(modules):parser.error('duplicate language tag')
    report={tag:refresh(path,tag)for tag,path in modules}
    args.report.write_text(json.dumps(report,indent=2)+'\n')
    for tag, row in report.items():
        print(tag,len(row['rules']),'rules,',sum(x['changed']for x in row['rules']),
              'refreshed,',sum(x['count']>0 for x in row['specialised_fences']),
              'constant nonempty fences',flush=True)

if __name__=='__main__': main()
