"""Project the shared Unicode Latin identities into the compact CJK table."""
from pathlib import Path
import argparse
import re

ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--check",action="store_true")
    a=p.parse_args()
    text=(ROOT/"src/reloq_character_data.inc").read_text()
    rows=text.split("latin_characters[] = {",1)[1].split("\n};",1)[0]
    bases={int(cp):int(base) for cp,base in re.findall(r"\{(\d+),(\d+),",rows)}
    # Native Latin letters without a canonical accent decomposition. Their
    # CP1252 identities are retained by the reader before this fallback.
    bases.update({0xc6:65,0xd0:68,0xd8:79,0xde:84,0xdf:115,0xe6:97,
                  0xf0:100,0xf8:111,0xfe:116,0x152:79,0x153:111,0x192:102})
    output="/* Unicode Latin base identities shared with the existing character table. */\n"
    output+="static const struct latin_base {uint32_t scalar; unsigned char base;} latin_bases[] = {\n"
    output+="".join(f"{{{cp},{base}}},\n" for cp,base in sorted(bases.items()))+"};\n"
    path=ROOT/"vendor/openevv61/src/reloq61_latin_data.inc"
    if a.check:
        assert path.read_text()==output,"CJK Latin projection needs regeneration"
        print(f"CJK Latin projection matches {len(bases)} shared Unicode identities")
    else:path.write_text(output)


if __name__=="__main__":main()
