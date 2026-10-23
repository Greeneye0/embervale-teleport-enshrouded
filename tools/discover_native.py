import pefile,capstone,re,json
from pathlib import Path
paths=[r'C:\temp\enshrouded-re\bin\enshrouded.exe',r'C:\Program Files (x86)\Steam\steamapps\common\Enshrouded\enshrouded.exe',r'C:\Program Files (x86)\Steam\steamapps\common\EnshroudedServer\enshrouded_server.exe']
imgs=[pefile.PE(p,fast_load=True).get_memory_mapped_image() for p in paths]
c=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_64);c.detail=True
funcs={'player':0x2d77f0,'apply':0x27b340,'next':0x9b5a50,'entity':0x9b2e40,'ray':0xa01470,'dirty':0x9c5df0,'time':0x9b8030,'start':0x215e40}
res={}
for name,a in funcs.items():
 data=imgs[0][a:a+80]; mask=[False]*len(data)
 for i in c.disasm(data,a):
  if i.address-a>=64:break
  if i.mnemonic in ('call','jmp') or i.group(capstone.CS_GRP_JUMP):
   for j in range(i.imm_offset,i.imm_offset+i.imm_size):mask[i.address-a+j]=True
  if any(o.type==capstone.x86.X86_OP_MEM and o.mem.base==capstone.x86.X86_REG_RIP for o in i.operands):
   for j in range(i.disp_offset,i.disp_offset+i.disp_size):mask[i.address-a+j]=True
  n=i.address-a+i.size
 pat=' '.join('??' if mask[k] else f'{data[k]:02X}' for k in range(n))
 regex=b''.join(b'.' if mask[k] else re.escape(bytes([data[k]])) for k in range(n))
 hits=[[m.start() for m in re.finditer(regex,d,re.S)] for d in imgs]
 print(name,[[hex(x) for x in h] for h in hits])
 res[name]={'pattern':pat,'hits':hits}
Path('MapTeleport/tools/native-bindings.json').write_text(json.dumps(res,indent=2))
for name in ('player','apply'):
 for k in (1,2):
  hs=res[name]['hits'][k]
  if len(hs)!=1:continue
  a=hs[0]; lim=0x3800 if name=='player' else 0x800
  calls=[i.address-a for i in c.disasm(imgs[k][a:a+lim],a) if i.mnemonic=='call' and i.operands[0].type==capstone.x86.X86_OP_IMM and i.operands[0].imm in res['next']['hits'][k]]
  print(name,k,'calls',[hex(x) for x in calls])
