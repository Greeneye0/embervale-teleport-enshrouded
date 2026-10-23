import pefile,capstone,re,json
from pathlib import Path
paths=[r'C:\temp\enshrouded-re\bin\enshrouded.exe',r'C:\Program Files (x86)\Steam\steamapps\common\Enshrouded\enshrouded.exe',r'C:\Program Files (x86)\Steam\steamapps\common\EnshroudedServer\enshrouded_server.exe']
imgs=[pefile.PE(p,fast_load=True).get_memory_mapped_image() for p in paths]
c=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_64);c.detail=True
out=['#pragma once','namespace teleport::native {']
for name,a in [('queueTeleport',0x952400),('notifyTeleport',0x9543b0)]:
 data=imgs[0][a:a+80];mask=[False]*len(data);n=0
 for i in c.disasm(data,a):
  if i.address-a>=64:break
  if i.mnemonic in ('call','jmp') or i.group(capstone.CS_GRP_JUMP):
   for j in range(i.imm_offset,i.imm_offset+i.imm_size):mask[i.address-a+j]=True
  if any(o.type==capstone.x86.X86_OP_MEM and o.mem.base==capstone.x86.X86_REG_RIP for o in i.operands):
   for j in range(i.disp_offset,i.disp_offset+i.disp_size):mask[i.address-a+j]=True
  n=i.address-a+i.size
 pattern=' '.join('??' if mask[k] else f'{data[k]:02X}' for k in range(n))
 rx=b''.join(b'.' if mask[k] else re.escape(bytes([data[k]])) for k in range(n))
 hits=[[m.start() for m in re.finditer(rx,d,re.S)] for d in imgs];assert all(len(x)==1 for x in hits),hits
 print(name,[[hex(x) for x in h] for h in hits]);out.append(f'inline constexpr char {name}Pattern[]="{pattern}";')
out.append('}')
Path('MapTeleport/src/direct_signatures.h').write_text('\n'.join(out)+'\n')
