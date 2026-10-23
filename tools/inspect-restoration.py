import ctypes,json,re,struct
from pathlib import Path
k=ctypes.WinDLL('kernel32');k.OpenProcess.restype=ctypes.c_void_p;k.ReadProcessMemory.argtypes=[ctypes.c_void_p,ctypes.c_void_p,ctypes.c_void_p,ctypes.c_size_t,ctypes.c_void_p];k.CloseHandle.argtypes=[ctypes.c_void_p]
mp=Path('MapTeleport/build/map-teleport.map').read_text()
def sym(name):
 line=next(x for x in mp.splitlines() if '?'+name+'@?' in x and '0003:' in x)
 return int(re.search(r'\b(000000018[0-9a-f]+)\b',line)[1],16)-0x180000000
for m in json.loads(Path('MapTeleport/build/live-modules.json').read_text(encoding='utf-8-sig')):
 if m['name']!='enshrouded':continue
 if Path(m['path']).read_bytes()!=Path('MapTeleport/bin/loader/map-teleport.dll').read_bytes():continue
 h=k.OpenProcess(0x410,False,m['pid'])
 def rd(a,n):
  b=ctypes.create_string_buffer(n);assert k.ReadProcessMemory(h,a,b,n,None);return b.raw
 def val(name,n):return rd(m['address']+sym(name),n)
 client=struct.unpack('<Q',val('client',8))[0];world=struct.unpack('<Q',rd(client+0x47d4e0,8))[0]
 pos=struct.unpack('<qqq',rd(world+0x2c08,24));status=val('remoteStatus',48);ground=struct.unpack_from('<qqq',status,24)
 print('status',status.hex(),'position metres',[x/2**32 for x in pos],'ground metres',[x/2**32 for x in ground]);print('ack',val('predictionAck',4).hex(),'landingSince',val('predictedLandingSince',8).hex());k.CloseHandle(h)

