import pefile,struct,sys
p=pefile.PE(r'C:\Program Files (x86)\Steam\steamapps\common\Enshrouded\enshrouded.exe'); d=p.get_memory_mapped_image(); b=p.OPTIONAL_HEADER.ImageBase
t=int(sys.argv[1],16)-b
for s in p.sections:
 if not s.Characteristics&0x20000000: continue
 start=s.VirtualAddress; end=start+s.Misc_VirtualSize
 for a in range(start,end-4):
  if a+4+struct.unpack_from('<i',d,a)[0]==t: print(hex(b+a),d[a-4:a+4].hex(' '))
