# Embervale Teleport — design notes (written during 0.2.0-test.1)

Test candidate for Greeneye Loader API 2. The prank, protection, recovery and deluxe option are integrated. Automated checks pass; **offline, self-hosted and dedicated gameplay validation remains pending**. Producing this build has not replaced the installed 0.1.1 prototype.

## Use

On the world map, press Shift and hold LMB for one second. The cursor ring fills; releasing early cancels. Moving the cursor retains the original target. Escape, right-click, closing the map and losing focus also cancel. A completed hold closes the map so the native animation and notices are visible.

The host resolves terrain height and uses the native travel action and Teleport component, instead of the old admin-position command. Actual animation, terrain ownership, foot placement and streaming still require gameplay checks.

## First-use prank

With Prank.NoFun=False, the first use by a Steam account on that host installation chooses one of the three approved scripts: scenic upgrade, metres versus kilometres, or the missing-ground delivery mix-up.

The host resolves ground, atomically journals the pending trip, enables fall-damage suppression, blocks glider eligibility and halves downward velocity. Native protection hooks must observe the player before travel starts. Remote clients also enable predicted-player protection and acknowledge readiness before the host countdown.

A native notice counts down 3, 2, 1. Native travel places the player four metres below the primary scene reset box's ceiling, above the target. The opening joke lasts six seconds, the problem message one second, and the correction 3.5 seconds. Native travel returns them to the terrain surface.

Arrival is confirmed from the actual native position. Protection remains for a two-second landing grace period. Remote clients must also observe ground arrival and acknowledge restoration before completion. Pending fall-damage values are cleared during protection, preventing a deferred hit afterward. Removing scoped protection lets native glider and gravity calculations resume normally.

The host's map-teleport-history.txt records pending/completed use per account. Completion is persisted only after arrival and restoration. Different characters on one account share that record. Separate server installations have separate records; independent server instances must not share this writable mod directory.

## Recovery

Pending journal entries resume as protected ground corrections on reconnect or process restart. Recovery resolves the terrain height again in the loaded world and does not repeat the aerial prank. Protection starts while recovery terrain is loading. After a world switch, the saved X/Z is resolved against the new world's terrain.

Failed native hops retain protection and retry ground correction. Departed player/world bindings are retired without recording completion. A protected status timeout does not release the client in midair. Replies must match account and nonce; client-authored host packets and delayed responses for completed trips are ignored.

Hot reload waits for travel and protection to finish. Upgrading from 0.1.1 requires a process restart because the candidate adds hooks. Dedicated companions should load before the world to capture reset bounds; unknown bounds reject high arrivals.

## Configuration

Edit the host's greeneye/map-teleport/map-teleport.ini and restart that process:

```ini
[Teleport]
DeluxeTeleports=false

[Prank]
NoFun=False
```

DeluxeTeleports=true makes regular trips arrive high with normal glider, fall speed and fall damage. It has no joke or automatic correction. Enabled first-use pranks take precedence. Disabling new pranks does not abandon pending recovery.

Install the same candidate on client and dedicated host. Offline and self-hosted sessions use the client installation's host logic.

## Checks and limits

Run build.bat and tools/tests.bat. Tests cover normal/deluxe paths, all scripts, protection readiness, position-confirmed arrival, server/client acknowledgments, prediction restoration, failed hops, disconnects, journal restart, first-use persistence, corrupt history and delayed packets. Physics simulations cover player/world isolation, half-speed integration, pending-damage clearing and restoration interlocks. Native signatures and redirected iterator calls match both installed executables.

These checks do not prove native component layouts, animation, UI appearance, reset-boundary enforcement, death/respawn behavior or real multiplayer prediction. Follow TESTING.md before treating this as a release.

Supported binaries: client timestamp 6ad77f0c, image size 02a1e000; dedicated timestamp 6ad77f14, image size 01dd1000. Other builds are rejected.

Set `[Messages] ShowBanners=False` in the client INI to hide all mod message banners (including prank dialogue, numeric countdown and error notices). The map cursor ring and all travel/recovery behavior remain active. The default is `True`; changes apply on restart or an idle hot reload. To disable the prank itself, use `[Prank] NoFun=True`.
