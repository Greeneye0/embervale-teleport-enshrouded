# Reset-boundary investigation

All addresses below refer to the older Ghidra image unless explicitly labelled current. No game writes or controls were used for this investigation.

- `SceneResource` descriptor `141a6a120`, type hash low32 `2bc68ebe`, field array `141a59000`.
- Its `resetPlayersOutOfBounds` field is at `+88`; the editable `Scene` representation has it at `+98`.
- `e998d0(manager, resetConfig)` copies world minimum at config `+8` (three fixed32.32 int64s) and world maximum at `+20` into the first box of manager's vector at `+b0`, count `+b8`, stride `38`. Additional active areas follow. Their flag semantics and enforcement are not yet verified.
- Current client setter RVA `e9c490`, dedicated setter RVA `9510f0`; unique matched signature:

```
48 8B C4 48 89 58 18 48 89 68 20 57 48 81 EC A0 00 00 00 48 8D 99 B0 00 00 00 C6 40 E8 00 48 C7 43 08 00 00 00 00 48 8B FA 0F 10 4A 08 48 8B CB C7 40 EC 00 00 00 00 0F 10 42 20 F2 0F 10 52 18
```

- Client caller `874600` reads manager at client `+4671d0`; verified through the existing read-only bridge in the loaded world. First box was min `(0,0,0)`, max `(10240,2560,10240)` metres; vector had 15 boxes. This is a world data observation, not a gameplay validation of the respawn boundary.
- Server scene load `e3a350` calls setter at `e3a4ea` with `param_2[2f2]` (offset `1790`). Server update `e46070` calls setter at `e4650e` similarly. Do not assume the chat receiver object is this same server object without verifying it.
- Scene load current RVA `e3ce40`, dedicated `7a6590`; its first 64-byte signature matched both. Server update's first 64 bytes did not match dedicated; do not use its client offset there.
- `altitude.h` implements bounded reading and a candidate destination four metres below the primary box ceiling, requiring more than 12 metres of total room above ground. Connected to regular deluxe travel, but not yet proven against native boundary enforcement.

Supporting decompilations: `prank-query4` through `prank-query8`. `safe_player_position` (`2e5650`) maintains a safe position and is not the out-of-bounds reset routine. `e99f70` is a camera-transition routine and unrelated.

## Current integration status

0.2.0-test.1 is packaged as a controlled-gameplay test candidate. The native host journey, persistent per-account history, random scenarios, protection adapters, client readiness/restoration acknowledgments, automatic pending-trip recovery and regular deluxe mode are integrated. Protection clears pending fall damage and retains a two-second landing grace. Failed native hops retry ground correction. The client checks ground proximity before restoration and ignores late completed-trip packets.

The full automated suite passed, including native-iterator simulations for local/remote trips, restart, disconnect, failed hops and predicted-client restoration. Both executable signatures match. Native animation, UI appearance, actual boundary enforcement, death/respawn and real multiplayer behavior still require gameplay checks. README.md and TESTING.md describe the current candidate and test sequence.

Package: bin/MapTeleport-0.2.0-test.1.zip. Building and packaging did not replace the installed 0.1.1 mod or alter the running player. Upgrading adds hooks and requires process restart; use the normal native path before trying the aerial prank.

Quick Deposit's DEV bridge from earlier setup has not been changed in this continuation. Its release backup remains build/test-backup/quick-deposit.dll; restore through the official loader workflow after live testing. No gameplay teleport has been tested in any of the three modes.