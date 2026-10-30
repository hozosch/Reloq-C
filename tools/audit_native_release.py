"""Report native dependencies and ensure development rule tracing is absent.

This inspects local binaries; it is not an antivirus scan or a device verdict.
"""
from pathlib import Path
import argparse
import json
import re
import subprocess


def command(*args):
    return subprocess.run(list(map(str,args)),check=True,capture_output=True,text=True).stdout


def symbols(path):
    rows=[]
    for line in command("nm","-S","--size-sort","--defined-only",path).splitlines():
        match=re.fullmatch(r"[0-9a-fA-F]+\s+([0-9a-fA-F]+)\s+(\w)\s+(\S+)",line)
        if match:rows.append((match[3],match[2],int(match[1],16)))
    return rows


def inspect(path,platform,objdump=None,release=True,archive=None,nm=None):
    text_section=None
    if platform=="windows":
        deps=re.findall(r"DLL Name:\s*(\S+)",command(objdump,"-p",path))
        allowed={"kernel32.dll","msvcrt.dll","user32.dll"}
        section=re.search(r"^\s*\d+\s+\.text\s+([0-9a-fA-F]+)",command(objdump,"-h",path),re.M)
        if section:text_section=int(section[1],16)
    else:
        deps=re.findall(r"\(NEEDED\).*\[([^]]+)\]",command("readelf","-d",path))
        allowed=({"libm.so.6","libc.so.6","ld-linux-x86-64.so.2"}
                 if platform=="host" else {"libm.so","libdl.so","libc.so"})
    if not deps or set(d.lower() for d in deps)-allowed:
        raise ValueError(f"Unexpected {platform} dependencies: {deps}")
    rows=symbols(path)
    if platform!="windows" and not rows:raise ValueError("Symbol audit needs an unstripped ELF library")
    diagnostic={name:size for name,kind,size in rows if name in {
        "delta_rule_trace","delta_rule_here","delta_rule_report",
        "delta_rule_calls","delta_rule_steps"}}
    bytecode=[name for name,kind,size in rows if re.match(r"delta_rule_(code|imm|map)_",name)]
    symbol_source="unstripped library"
    if platform=="windows":
        # The distributable DLL is stripped. Check the actual input archive,
        # including GCC's LTO symbols, rather than treating no symbols as zero.
        if not archive or not nm:raise ValueError("Windows symbol audit needs its native archive and GCC nm")
        names={line.split()[-1] for line in command(nm,"-A",archive).splitlines() if line.split()}
        diagnostic={name:None for name in names if name in {
            "delta_rule_trace","delta_rule_here","delta_rule_report","delta_rule_calls","delta_rule_steps"}}
        bytecode=[name for name in names if re.match(r"delta_rule_(code|imm|map)_",name)]
        if b"DELTA_RULE_TRACE\0" in path.read_bytes():raise ValueError("Rule trace environment lookup remains in DLL")
        symbol_source="native input archive including GCC LTO symbols; DLL symbol table is stripped"
    if release and (diagnostic or bytecode):
        raise ValueError(f"Development rule state/data in release: {diagnostic}, {bytecode}")
    return dict(bytes=path.stat().st_size,dependencies=deps,rule_diagnostic_symbols=diagnostic,
        bytecode_arrays=bytecode,symbol_audit_source=symbol_source,text_section_bytes=text_section,
        defined_text_bytes=sum(size for name,kind,size in rows if kind in "tT") if rows else None,
        language_call_glue_text_bytes=sum(size for name,kind,size in rows if kind in "tT" and name.startswith("reloq_call_")) if rows else None)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ("host","windows","android"):parser.add_argument("--"+name,type=Path,required=True)
    parser.add_argument("--windows-objdump",type=Path,required=True)
    parser.add_argument("--windows-archive",type=Path,required=True)
    parser.add_argument("--windows-nm",type=Path,required=True)
    parser.add_argument("--baseline",type=Path)
    parser.add_argument("--output",type=Path,required=True)
    args=parser.parse_args()
    report={name:inspect(getattr(args,name),name,args.windows_objdump,
        archive=args.windows_archive,nm=args.windows_nm) for name in ("host","windows","android")}
    if args.baseline:report["baseline_host"]=inspect(args.baseline,"host",release=False)
    report["scope"]="Static native dependency/symbol inspection only; no malware classification or device performance claim."
    args.output.write_text(json.dumps(report,indent=2)+"\n")
    print(json.dumps(report,indent=2))


if __name__=="__main__":main()
