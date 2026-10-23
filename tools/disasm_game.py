import sys, pefile, capstone
p=pefile.PE(r'C:\Program Files (x86)\Steam\steamapps\common\Enshrouded\enshrouded.exe',fast_load=True)
b=p.OPTIONAL_HEADER.ImageBase
d=p.get_memory_mapped_image()
c=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_64)
if len(sys.argv)>1:
 a=int(sys.argv[1],16); n=int(sys.argv[2],16)
 for i in c.disasm(d[a-b:a-b+n],a): print(f'{i.address:x}: {i.mnemonic:9} {i.op_str}')
else:
 pat=bytes.fromhex('48 8D AC 24 A0 F8 FF FF 48 81 EC 60 08 00 00 48 8B 05')
 a=d.find(pat)-16
 print(hex(b+a))
 for i in c.disasm(d[a:a+0x1700],b+a): print(f'{i.address:x}: {i.mnemonic:9} {i.op_str}')
