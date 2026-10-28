"""Regenerate native C from readable rules; discard offline IR arrays."""
from pathlib import Path
import os,re,subprocess,sys
R=Path(__file__).resolve().parents[1]
def main():
 for tag in ('enus61','dede61','ptbr','fifi'):
  subprocess.run([sys.executable,'tools/rules/consts.py',tag],cwd=R,check=True)
  env=dict(os.environ,EVV_NOTATION_LANG=tag,EVV_LANG_DIR='lang/'+tag,EVV_FAST_C='1')
  subprocess.run([sys.executable,'tools/rules/notation.py','build'],cwd=R,env=env,check=True)
  subprocess.run([sys.executable,'tools/rules/decompile.py','all'],cwd=R,env=env,check=True)
  p=R/'lang'/tag/('delta_rules_'+tag+'.c');s=p.read_text()
  s,n=re.subn(r'const (?:uint8_t|int32_t) '+tag+r'_delta_rule_(?:code|imm|map)\[\] = \{[^}]*\};\s*','',s)
  assert n==3,(tag,n)
  p.write_text(s.replace("The language's rules as bytecode, with the constants,",'Native C rule metadata, with the constants,'))
  (p.parent/('delta_rules_none_'+tag+'.c')).unlink(missing_ok=True)
 print('Native C regenerated; offline bytecode arrays removed.')
if __name__=='__main__':main()
