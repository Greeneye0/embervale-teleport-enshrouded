EMBERVALE TELEPORT - MAP BASED TELEPORT  -  Enshrouded mod, version 1.0.0
==========================================================================

Open the world map, hold Shift and hold the left mouse button on any spot for one second.
The map closes and you travel there with the game's own teleport animation, landing on
the ground at that spot.


INSTALL
-------
1. Close Enshrouded.
2. In Steam: right-click Enshrouded > Manage > Browse local files.
   This opens the game folder (the one with enshrouded.exe), usually
   C:\Program Files (x86)\Steam\steamapps\common\Enshrouded
3. Copy everything from this zip into that folder, keeping the folders:
       dbgcore.dll                              next to enshrouded.exe
       greeneye\map-teleport\map-teleport.dll
       greeneye\map-teleport\map-teleport.ini
       greeneye\map-teleport\mod.json
4. Start the game, open the map, hold Shift and hold the left mouse button on a spot.

Dedicated server: copy the same files next to enshrouded_server.exe in the server folder
and restart the server. Every player who wants to teleport needs the mod on their game.

Install it by hand: Vortex does not know the Greeneye Loader yet.


USE
---
- Shift + hold left mouse on the world map for one second. A ring around the cursor fills
  up while you hold.
- Let go early, press Esc or right-click to cancel. Moving the mouse while holding keeps
  the spot you first clicked.
- If the land at the destination isn't loaded yet, the mod waits for it, so you never
  land inside a hill or in the air.
- Short notices ("Teleporting in 3...") tell you what is happening.


SETTINGS
--------
greeneye\map-teleport\map-teleport.ini. Every setting is explained in the file itself.
Restart the game (or the server) after changing it. For example:
    [Teleport] DeluxeTeleports = false   true: arrive high above the spot and glide down
    [Messages] ShowBanners = True        False: hide the mod's notices


UNINSTALL
---------
Delete greeneye\map-teleport. If no other mod is left in the greeneye folder, delete
dbgcore.dll and the greeneye folder too.


GOOD TO KNOW
------------
- Single player, hosting your own game, and dedicated servers (install it on the server
  too).
- dbgcore.dll is the Greeneye Loader: it loads the mods in the greeneye folder (Find,
  Quick Deposit, Continue and others share it) and forwards everything else to Windows'
  own dbgcore.dll.
- Works next to other mods. Where another mod (for example Creative Mode) hooks the same
  game function, Embervale Teleport chains onto its hook instead of replacing it.
- Made for the current game version. After a game update the mod checks that it still
  recognises the game; if not, it switches itself off instead of risking a crash.
- greeneye\map-teleport\map-teleport-history.txt is the mod's own small record. Leave it.
- If something goes wrong, greeneye\loader.log says what the mod did (its lines start with
  "Map Teleport:"). Please post it with a bug report, from the server folder too if you
  play on a dedicated server.


CHANGES
-------
1.0.0
- First release.
