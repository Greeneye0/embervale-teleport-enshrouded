import pefile,capstone,bisect
p=pefile.PE('C:/temp/enshrouded-re/bin/enshrouded.exe');im=p.get_memory_mapped_image();cs=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_64);cs.detail=True;cs.skipdata=True
for x in cs.disasm(im[0x410000:0x450000],0x140410000):
 if x.id and any(o.type==capstone.x86.X86_OP_MEM and o.mem.disp in (0x38,0x3c,0x40) and o.mem.base not in (capstone.x86.X86_REG_RSP,capstone.x86.X86_REG_RBP) for o in x.operands) and (x.mnemonic in ('mulss','addss','subss','movss','comiss','ucomiss') or 'byte ptr' in x.op_str):print(hex(x.address),x.mnemonic,x.op_str)

