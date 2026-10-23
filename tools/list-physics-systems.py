import pefile,struct,re
p=pefile.PE('C:/temp/enshrouded-re/bin/enshrouded.exe');im=p.get_memory_mapped_image();base=0x140000000
for m in re.finditer(rb'[a-z][a-z_]{4,}\x00',im):
 s=m.group()[:-1]
 if not any(k in s for k in (b'fall',b'gravity',b'locomotion',b'physics',b'movement')):continue
 addr=struct.pack('<Q',base+m.start());start=0
 while True:
  ref=im.find(addr,start)
  if ref<0:break
  start=ref+1
  if ref+24>len(im):continue
  fun=struct.unpack_from('<Q',im,ref+16)[0]
  if base+0x1000<fun<base+0x1510000:print(s.decode(),hex(fun),hex(base+ref))
