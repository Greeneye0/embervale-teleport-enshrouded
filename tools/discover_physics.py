"""Discover candidate physics bindings; does not install or activate hooks."""
import capstone, json, pefile, re, struct
from pathlib import Path

root = Path(__file__).resolve().parent.parent
paths = [Path(r'C:\temp\enshrouded-re\bin\enshrouded.exe'),
         Path(r'C:\Program Files (x86)\Steam\steamapps\common\Enshrouded\enshrouded.exe'),
         Path(r'C:\Program Files (x86)\Steam\steamapps\common\EnshroudedServer\enshrouded_server.exe')]
executables = [pefile.PE(str(p), fast_load=True) for p in paths]
images = [executable.get_memory_mapped_image() for executable in executables]
def function_spans(index, start):
    image = images[index]
    directory = executables[index].OPTIONAL_HEADER.DATA_DIRECTORY[3]
    def owner(begin, unwind, depth=0):
        assert depth < 16
        if image[unwind] & 0x20: # UNW_FLAG_CHAININFO: following region belongs to this function
            chain = unwind + ((4 + 2*image[unwind+2] + 3) & ~3)
            parent, _, parent_unwind = struct.unpack_from('<III', image, chain)
            return owner(parent, parent_unwind, depth+1)
        return begin
    return [(begin, end) for begin, end, unwind in struct.iter_unpack('<III', image[directory.VirtualAddress:directory.VirtualAddress+directory.Size])
            if start <= begin < start+0x2000 and owner(begin, unwind) == start]
dis = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
dis.detail = True
native = json.loads((root/'tools/native-bindings.json').read_text())
result = {}
for name, old in [('fall_damage', 0x274070),
                  ('glider', 0x2d6390),
                  ('gravity', 0x29e810), ('player_motion', 0x4362f0), ('fall_detection', 0x273b00)]:
    data = images[0][old:old+96]
    mask = [False]*len(data)
    end = 0
    for ins in dis.disasm(data, old):
        if ins.address-old >= 64:
            break
        if ins.group(capstone.CS_GRP_JUMP) or ins.mnemonic == 'call':
            for n in range(ins.imm_offset, ins.imm_offset+ins.imm_size):
                mask[ins.address-old+n] = True
        if any(op.type == capstone.x86.X86_OP_MEM and op.mem.base == capstone.x86.X86_REG_RIP for op in ins.operands):
            for n in range(ins.disp_offset, ins.disp_offset+ins.disp_size):
                mask[ins.address-old+n] = True
        end = ins.address-old+ins.size
    pattern = ' '.join('??' if mask[n] else f'{data[n]:02X}' for n in range(end))
    regex = b''.join(b'.' if mask[n] else re.escape(bytes([data[n]])) for n in range(end))
    hits = [[m.start() for m in re.finditer(regex, image, re.S)] for image in images]
    assert all(len(h) == 1 for h in hits), (name, hits)
    calls = []
    for index, image in enumerate(images):
        start = hits[index][0]
        spans = function_spans(index, start)
        assert spans, (name, index, 'function unwind boundary unavailable')
        offsets = [ins.address-start for begin, end in spans for ins in dis.disasm(image[begin:end], begin)
                   if ins.mnemonic == 'call' and ins.operands[0].type == capstone.x86.X86_OP_IMM
                   and ins.operands[0].imm in native['next']['hits'][index]]
        assert len(offsets) == 2, (name, index, offsets)
        calls.append(offsets)
    result[name] = {'pattern': pattern, 'hits': hits, 'iteratorCalls': calls}
    print(name, 'RVA:', [hex(h[0]) for h in hits], 'iterator offsets:', [[hex(v) for v in c] for c in calls])
(root/'tools/physics-bindings.json').write_text(json.dumps(result, indent=2))
(root/'src/physics_signatures.h').write_text('#pragma once\nnamespace teleport::native {\n' +
    ''.join(f'inline constexpr char {name}Pattern[]="{binding["pattern"]}";\n' for name, binding in result.items()) + '}\n')
print('Candidate bindings only; player-scoped protection still requires integration and gameplay validation.')

