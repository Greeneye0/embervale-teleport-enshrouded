"""Offline client 1.0.1.0 checks. Usage: python tools/verify_signatures.py path/to/enshrouded.exe"""
import argparse,json,os,pathlib,re,struct
root=pathlib.Path(__file__).resolve().parent.parent
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument("exe",nargs="?",default=os.environ.get("ENSHROUDED_EXE",r"C:\Program Files (x86)\Steam\steamapps\common\Enshrouded\enshrouded.exe"))
args=parser.parse_args();raw=pathlib.Path(args.exe).read_bytes()
pe=struct.unpack_from("<I",raw,60)[0];stamp=struct.unpack_from("<I",raw,pe+8)[0]
opt=pe+24;size=struct.unpack_from("<I",raw,opt+56)[0]
assert (stamp,size)==(0x6ae4601e,0x2a40000),"This check targets the 1.0.1.0 client; dedicated-server layouts are not verified."
count=struct.unpack_from("<H",raw,pe+6)[0];sections=opt+struct.unpack_from("<H",raw,pe+20)[0]
data=bytearray(size)
for i in range(count):
 at=sections+i*40;rva,n,offset=struct.unpack_from("<III",raw,at+12)
 data[rva:rva+n]=raw[offset:offset+n]
def find(pattern):
 rx=b"".join(b"." if "?" in x else re.escape(bytes.fromhex(x)) for x in pattern.split())
 return [m.start() for m in re.finditer(rx,data,re.S)]
def unique(pattern,name):
 hits=find(pattern);assert len(hits)==1,(name,hits);return hits[0]
def call(site,target):
 assert data[site]==0xe8 and site+5+struct.unpack_from("<i",data,site+1)[0]==target,hex(site)
bindings=json.loads((root/"tools/native-bindings.json").read_text());physics=json.loads((root/"tools/physics-bindings.json").read_text())
hits={name:unique(bindings[name]["pattern"],name) for name in ("player","apply","next","entity","ray","dirty")}
for name,offsets in [("player",(0x35,0x11c6)),("apply",(0x31,0x7be))]:
 for offset in offsets:call(hits[name]+offset,hits["next"])
for name,binding in physics.items():
 address=unique(binding["pattern"],name)
 for offset in binding["iteratorCalls"][1]:call(address+offset,hits["next"])
for pattern in re.findall(r'Pattern\[\]="([^"]+)"',(root/"src/direct_signatures.h").read_text()):unique(pattern,"direct return")
source=(root/"src/travel.cpp").read_text()
for pattern in re.findall(r'host->find\("(48 8B C4[^"]+)"',source):unique(pattern,"bounds")
receivers=re.findall(r'host->find\("(40 55 53[^"]+)"',source)
assert sum(len(find(p)) for p in receivers)==1
for pattern in re.findall(r'host->find\("([^"]+)"',(root/"src/main.cpp").read_text()):unique(pattern,"UI/chat")
for pattern in re.findall(r'Bind\(host,\w+,"([^"]+)"',(root/"src/popup.cpp").read_text()):unique(pattern,"popup")
# Verify the cast travel gate's call and the map hook's complete prologue.
travel=(root/"src/travel.cpp").read_text()
gate_pattern=re.search(r'auto g=host->find\("([^"]+)"',travel)
assert gate_pattern,"Travel gate signature missing"
call(hits["player"]+0x10b8,unique(gate_pattern[1],"travel gate"))
a=unique("48 8D AC 24 A0 F8 FF FF 48 81 EC 60 08 00 00 48 8B 05","map tail")
assert data[a-16:a]==bytes.fromhex("48 89 5C 24 08 55 56 57 41 54 41 55 41 56 41 57")
slot=a+0x16+struct.unpack_from("<i",data,a+0x12)[0];assert 0<slot<size
print("PASS: client 1.0.1.0 travel/physics/UI signatures, iterator targets and map prologue")
print("Static checks only; actor animation and multiplayer behavior require gameplay verification.")
