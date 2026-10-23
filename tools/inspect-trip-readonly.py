import ctypes,json,hashlib,struct,re
from pathlib import Path
k=ctypes.WinDLL('kernel32',use_last_error=True);k.OpenProcess.restype=ctypes.c_void_p;k.ReadProcessMemory.argtypes=[ctypes.c_void_p,ctypes.c_void_p,ctypes.c_void_p,ctypes.c_size_t,ctypes.POINTER(ctypes.c_size_t)];k.CloseHandle.argtypes=[ctypes.c_void_p]
built=Path('MapTeleport/bin/loader/map-teleport.dll').read_bytes()
for m in json.loads(Path('MapTeleport/build/live-modules.json').read_text(encoding='utf-8-sig')):
 assert Path(m['path']).read_bytes()==built
 h=k.OpenProcess(0x410,False,m['pid']);assert h
 def read(off,n):
  b=ctypes.create_string_buffer(n);done=ctypes.c_size_t();assert k.ReadProcessMemory(h,m['address']+off,b,n,ctypes.byref(done));return b.raw
 print(m['name'],'ack',struct.unpack('<I',read(0x593e8,4))[0])
 print('watching',read(0x57008,56).hex())
 print('pending first',read(0x585b0,168).hex())
 print('remoteStatus',read(0x593b0,48).hex())
 if m['name']=='enshrouded_server':
  world=read(0x59480,32);print('hostWorld',world.hex());terrain=struct.unpack_from('<Q',world,8)[0];realm=world[24]
  def ptr(a,n):return read(a-m['address'],n)
  realms,count=struct.unpack('<QQ',ptr(terrain+0x1bf8,16));grid=struct.unpack('<Q',ptr(realms+realm*0x3870,8))[0]
  print('realm',realm,'count',count,'grid',hex(grid));print('dimensions',struct.unpack('<IIII',ptr(grid+0x10,16)),'origin',struct.unpack('<fff',ptr(grid+0x70,12)))
 k.CloseHandle(h)
