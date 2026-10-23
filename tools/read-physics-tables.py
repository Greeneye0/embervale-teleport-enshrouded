import pefile,struct
p=pefile.PE('C:/temp/enshrouded-re/bin/enshrouded.exe');im=p.get_memory_mapped_image();base=0x140000000
for start in (0x1ddea80,0x1ddeC60,0x1ddee50,0x1dcc510):
 print('\nTABLE',hex(base+start))
 for a in range(start,start+0x80,8):
  v=struct.unpack_from('<Q',im,a)[0];s=''
  if base<v<base+len(im):
   b=im[v-base:v-base+64].split(b'\0')[0]
   if b and all(32<=c<127 for c in b):s=b.decode()
  print(hex(base+a),hex(v),s)
