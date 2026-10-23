import pefile,capstone
p=pefile.PE('C:/temp/enshrouded-re/bin/enshrouded.exe');cs=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_64);cs.detail=True;cs.skipdata=True
for sec in p.sections:
 if sec.Name.startswith(b'.text'):
  for x in cs.disasm(sec.get_data(),p.OPTIONAL_HEADER.ImageBase+sec.VirtualAddress):
   if x.id and any(o.type==capstone.x86.X86_OP_MEM and o.mem.disp in (0x270,0x26c,0x274) for o in x.operands): print(hex(x.address),x.mnemonic,x.op_str)
