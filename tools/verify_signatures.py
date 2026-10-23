import pathlib,re,struct,json,pefile
root=pathlib.Path(__file__).resolve().parent.parent
paths=[r'C:\Program Files (x86)\Steam\steamapps\common\Enshrouded\enshrouded.exe',r'C:\Program Files (x86)\Steam\steamapps\common\EnshroudedServer\enshrouded_server.exe']
bindings=json.loads((root/'tools/native-bindings.json').read_text())
physics=json.loads((root/'tools/physics-bindings.json').read_text())
def find(data,pattern):
 rx=b''.join(b'.' if x=='??' else re.escape(bytes.fromhex(x)) for x in pattern.split())
 return [v.start() for v in re.finditer(rx,data,re.S)]
for index,path in enumerate(paths):
 p=pefile.PE(path,fast_load=True);data=p.get_memory_mapped_image();hits={}
 # The registry getter is distinct from the enclosing ECS-world getter.
 assert len(find(data,'48 8B 81 78 04 00 00 C3'))>=1,('registry terrain getter',path)
 for name in ('player','apply','next','entity','ray','dirty'):
  matches=find(data,bindings[name]['pattern']);assert len(matches)==1,(path,name,matches);hits[name]=matches[0]
 for name,offsets in [('player',(0x36,0x21f6)),('apply',(0x31,0x7be))]:
  for offset in offsets:
   site=hits[name]+offset
   assert data[site]==0xe8 and site+5+struct.unpack_from('<i',data,site+1)[0]==hits['next']
 for pattern in re.findall(r'Pattern\[\]="([^"]+)"',(root/'src/direct_signatures.h').read_text()):assert len(find(data,pattern))==1,('direct return',path)
 for name,binding in physics.items():
  matches=find(data,binding['pattern']);assert len(matches)==1,(path,name,matches)
  for offset in binding['iteratorCalls'][index+1]:
   site=matches[0]+offset
   assert data[site]==0xe8 and site+5+struct.unpack_from('<i',data,site+1)[0]==hits['next']
 source=(root/'src/travel.cpp').read_text()
 for pat in re.findall(r'host->find\("(48 8B C4[^"]+)"',source):assert len(find(data,pat))==1,('bounds',pat)
 chat=find(data,'40 55 56 41 54 41 55 41 57 48 83 EC 30 49 8B 41 08 33 ED');assert len(chat)==1
 receivers=re.findall(r'host->find\("(40 55 53[^"]+)"',source)
 assert sum(len(find(data,pat)) for pat in receivers)==1
 if index==0:
  source=(root/'src/main.cpp').read_text()
  for pat in re.findall(r'host->find\("([^"]+)"',source): assert len(find(data,pat))==1,pat
  popup=(root/'src/popup.cpp').read_text()
  for pat in re.findall(r'Bind\(host,\w+,"([^"]+)"',popup):assert len(find(data,pat))==1,('popup',pat)
  a=data.find(bytes.fromhex('48 8D AC 24 A0 F8 FF FF 48 81 EC 60 08 00 00 48 8B 05'))
  assert data[a-16:a]==bytes.fromhex('48 89 5C 24 08 55 56 57 41 54 41 55 41 56 41 57')
  assert a+0x16+struct.unpack_from('<i',data,a+0x12)[0]==0x281e498
 print('PASS:',pathlib.Path(path).name,'unique travel/physics hooks, native iterator call targets and chat bindings')
print('Static validation only: animation, realm ownership and multiplayer behavior require gameplay testing.')
