"""Execute the compiled AAPCS64 save/jump pair; no Android or audio needed.

Requires pyelftools and Unicorn. Pass the actual NDK-built evv_land.o.
"""
import argparse
from pathlib import Path
from elftools.elf.elffile import ELFFile
from unicorn import Uc, UC_ARCH_ARM64, UC_MODE_ARM
from unicorn import arm64_const as regs


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("object", type=Path)
    args = parser.parse_args()
    with args.object.open("rb") as stream:
        elf = ELFFile(stream)
        assert elf["e_machine"] == "EM_AARCH64"
        section = elf.get_section_by_name(".text")
        code = section.data()
        symbols = {s.name: s["st_value"] for s in elf.get_section_by_name(".symtab").iter_symbols()}
    base, sentinel, buffer, stack = 0x100000, 0x180000, 0x200000, 0x300000
    for answer in (0, 7, 0xfffffffd):
        uc = Uc(UC_ARCH_ARM64, UC_MODE_ARM)
        uc.mem_map(base, 0x100000)
        uc.mem_write(base, code)
        uc.mem_map(buffer, 0x1000)
        uc.mem_map(stack, 0x2000)
        uc.mem_write(buffer, b"\xa5" * 256)
        saved = {getattr(regs, "UC_ARM64_REG_X"+str(n)): 0x12340000+n for n in range(18,30)}
        saved.update({regs.UC_ARM64_REG_X30: sentinel, regs.UC_ARM64_REG_SP: stack+0x1000,
                      regs.UC_ARM64_REG_FPCR: 1 << 22, regs.UC_ARM64_REG_FPSR: 0x08000015})
        saved.update({getattr(regs,"UC_ARM64_REG_D"+str(n)):0x3ff0000000000000+n for n in range(8,16)})
        for register,value in saved.items(): uc.reg_write(register,value)
        uc.reg_write(regs.UC_ARM64_REG_X0,buffer)
        uc.emu_start(base+symbols["evv_land_save"],sentinel,count=100)
        assert uc.reg_read(regs.UC_ARM64_REG_X0)==0
        assert bytes(uc.mem_read(buffer+192,64))==b"\xa5"*64, "save overflowed the 24-word buffer"
        for register in saved: uc.reg_write(register,0)
        uc.reg_write(regs.UC_ARM64_REG_SP,stack+0x1800)
        uc.reg_write(regs.UC_ARM64_REG_X0,buffer)
        uc.reg_write(regs.UC_ARM64_REG_X1,answer)
        uc.emu_start(base+symbols["evv_land_jump"],sentinel,count=100)
        for register,value in saved.items():
            assert uc.reg_read(register)==value,(register,hex(uc.reg_read(register)),hex(value))
        assert uc.reg_read(regs.UC_ARM64_REG_X0)==answer
        assert uc.reg_read(regs.UC_ARM64_REG_PC)==sentinel
    print("ARM64 landing: platform/GPR/SIMD registers, SP/LR, FP state and buffer bounds passed")


if __name__ == "__main__": main()
