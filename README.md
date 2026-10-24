# Embervale Teleport — map based teleport for Enshrouded

Open the world map, hold **Shift** and hold the **left mouse button** on a spot for one second. The map closes and you travel there with the game's own teleport animation, landing on the ground at that spot.

- Uses the game's native travel action, not a position hack.
- Lands on the terrain surface, and waits for unloaded terrain instead of guessing the height.
- A ring around the cursor fills while you hold. Let go early, press Esc or right-click to cancel.
- Single player, self-hosted and dedicated servers (the server needs the mod too).
- Notices in all 15 languages the game supports, following the game's language setting (English, French, Italian, German, Spanish, Japanese, Korean, Polish, Portuguese (Brazil), Russian, Simplified Chinese, Traditional Chinese, Turkish, Ukrainian and Thai). The texts are in `src/strings.h`.

Download and install instructions: see the Nexus Mods page. The player-facing readme is [dist/README.txt](dist/README.txt).

## Settings

`greeneye\map-teleport\map-teleport.ini`. Each setting is documented in the file. Restart the game or server after changing it.

## Building

Needs Visual Studio 2022 Build Tools (x64).

```
build.bat          -> bin\loader\map-teleport.dll, mod.json, map-teleport.ini
tools\tests.bat    -> simulations plus signature checks (needs the game and server installed, and Python with pefile)
```

The mod runs through the Greeneye Loader (API 2, `src/greeneye.h`). All hooks go through the loader, which chains onto other mods' hooks instead of overwriting them. Every detour holds a `greeneye::Lease`, so the mod can be hot reloaded while idle.

Every game function is found by byte signature (`host->find`); if any binding is missing the mod installs nothing. Builds other than the ones it was tested on (PE timestamp and image size) are logged as untested and run when the signatures match.

`docs/` has the design notes and the gameplay test plan written during development.

## License

MIT, see [LICENSE](LICENSE). Made with the help of AI (Claude by Anthropic).
