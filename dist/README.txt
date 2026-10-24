EMBERVALE TELEPORT - MAP BASED TELEPORT  -  Enshrouded mod, version 1.1.2
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

Single player and hosting your own game: that's all. Your game is the host.


DEDICATED SERVER
----------------
The mod has to be on the server AND on the game of every player who wants to teleport.
1. Stop the server.
2. Open the server folder, the one with enshrouded_server.exe. In Steam: Library > Tools >
   Enshrouded Dedicated Server > right-click > Manage > Browse local files. With SteamCMD
   it's the folder you installed to; with a rented server, use the host's file manager/FTP.
3. Copy everything from this zip into that folder, the same way as for the game:
       dbgcore.dll                              next to enshrouded_server.exe
       greeneye\map-teleport\...
4. Start the server.
5. Check it loaded: greeneye\loader.log in the server folder should contain
   "Embervale Teleport 1.1.2: server companion loaded".
Players: each player installs the mod on their own game as above. Players without the mod
can still join and play; they just can't teleport, and see nothing from the mod.
Settings: the server's own map-teleport.ini decides how teleports behave for everyone on
that server; each player's own ini only decides whether their notices are shown.
Rented servers: the host must let you add files next to enshrouded_server.exe. Tested on
a Windows server; not tested on Linux/Wine hosting.

Install it by hand: Vortex does not know the Greeneye Loader yet.


USE
---
- Shift + hold left mouse on the world map for one second. A ring around the cursor fills
  up while you hold.
- Let go early, press Esc or right-click to cancel. Moving the mouse while holding keeps
  the spot you first clicked.
- If the land at the destination isn't loaded yet, the mod waits for it, so you never
  land inside a hill or in the air.
- Short notices ("Teleporting in 3...") tell you what is happening, in the game's language:
  English, French, Italian, German, Spanish, Japanese, Korean, Polish,
  Portuguese (Brazil), Russian, Simplified and Traditional Chinese, Turkish, Ukrainian
  and Thai. Change the game's language and the notices follow.


SETTINGS
--------
greeneye\map-teleport\map-teleport.ini. Every setting is explained in the file itself.
Restart the game (or the server) after changing it. For example:
    [Teleport] DeluxeTeleports = false   true: arrive high above the spot and glide down
    [Messages] ShowBanners = True        False: hide the mod's notices
    [Messages] Language = auto           or en, de, fr, ja, zh-hans...: the notices' language


UNINSTALL
---------
Delete greeneye\map-teleport. If no other mod is left in the greeneye folder, delete
dbgcore.dll and the greeneye folder too.


GOOD TO KNOW
------------
- Single player, hosting your own game, and dedicated servers (install it on the server
  too). Players without the mod can play on the same server and notice nothing.
- On a server without the mod, map teleport is simply unavailable: Shift + hold tells you
  so, and nothing shows up in chat.
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
1.1.2
- Runs on other builds of the game too (after a game patch, or copies whose exe
  differs from Steam's) when it recognises the game code; before, it switched
  itself off on any build but the current Steam one.

1.1.1
- Fixed: after leaving a server or world and joining another, map teleport could
  wrongly say the new server does not run Embervale Teleport.

1.1.0
- Notices in all 15 languages Enshrouded supports, following the game's language.
- [Messages] Language in the ini picks one language instead.

1.0.0
- First release.
