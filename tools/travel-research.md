# Native travel research — work in progress

The installed 0.1.1 build implements the hold countdown only. It still uses command 0x47. It does **not** implement the subsequently requested native animation, guaranteed terrain-top placement, or verified dedicated-server support.

Required final behavior:

- Ctrl+Shift+LMB starts a one-second cursor ring. LMB must remain held; releasing cancels.
- After confirmation, run the built-in teleport sequence, with movement timed by that sequence.
- Resolve the highest terrain surface at the selected X/Z and place the character standing on it. Do not reuse the player's current Y or assume Y=0 is the terrain surface.
- Support solo/offline, listen/self-hosted, and dedicated multiplayer. The user approved a server companion. Dedicated operation is not client-only.

Verified against installed client executable:

- UI dispatcher at 0x140852760; command 0x47 selects 0x140853073. The event carries a 24-byte position.
- At 0x1403ab77a the event's position is copied to input +0x238 and word +0x250 is set to 0x0101. This is the admin path, not evidence of a native animation.
- TeleportAction reflection descriptor at 0x141fbe340: size 0x0c, fields at 0x141fbe280: `target` at +0, `teleporterType` at +4, `versionData` at +8. This targets an entity rather than an arbitrary position.
- AdminTeleportAction reflection: size 0x28, transform +0, searchBestSpawnPosition +0x20, versionData +0x24. A generic spawn search is not verified to mean highest terrain surface.
- `actor_teleport` system registration at 0x141dc5d40 points to 0x1402707f0. Disassembly saved in actor-teleport.txt.
- `teleport_interaction` registration at 0x141e26320 points to 0x140347a20. It resolves target entities and starts an actor sequence via 0x140215fe0; destination state is set at +0xec. This is research evidence, not a callable mod API yet. Disassembly saved in teleport-interaction.txt.
- Local dedicated executable exists at C:/Program Files (x86)/Steam/steamapps/common/EnshroudedServer/enshrouded_server.exe. Its hooks and layouts have not been validated.

Do not dispatch another instant position request after an arbitrary animation delay: that would not establish native-sequence synchronization or safe multiplayer placement. Do not install guessed host offsets or silently label the current build as supporting these new requirements.


## October 22 continuation

`native-query6` traces the native action command, actor TeleportToTarget event, and Teleport component application. `native-query7`/`8` trace voxel ray completeness and terrain block loading. Ray result +0x12 is completeness, +0x13 is hit; +0x14 is not a completeness flag. `native-query11` traces path-grid terrain ownership. `native-bindings.json` records matching client and dedicated signatures. These are static findings, not live verification.

Source 0.2.0-preview is now compiled but not installed or packaged as a release. It requires verification of its self-target native travel action, terrain manager ownership, entity deferral and foot height. The README tracks these gaps explicitly.

The user additionally requested a first-use joke with temporary fall protection, disabled glider, half fall speed, native countdown/story windows, high-altitude arrival and subsequent ground correction. First use is once per player per server, persistent across restarts/character changes. This sequence is not implemented; its native gameplay bindings remain unverified.
