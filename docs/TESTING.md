# Gameplay test plan — 0.2.0-test.1

This candidate passes automated simulations and signature checks. No live gameplay row below has passed yet. Existing installed files have not been changed by creating this package.

## Setup

1. Leave the current session normally before replacing loaded files. Coordinate a dedicated restart if others are connected.
2. Back up greeneye/map-teleport. Extract this package's greeneye directory alongside the executable, retaining Greeneye Loader and other mods.
3. Install the same candidate on the dedicated host. Restart both processes: upgrading from 0.1.1 adds hooks and cannot be hot-reloaded.
4. For the first native-path test, set Prank.NoFun=True and Teleport.DeluxeTeleports=false on the host. Start the process and load the intended world.

## Check each hosting mode

Run offline, self-hosted and dedicated sessions separately.

| Check | Expected |
|---|---|
| Early LMB release | Ring cancels, no movement |
| Cursor moves during hold | Original target retained |
| Normal travel | Map closes, native animation, terrain-top landing |
| Unloaded terrain | Waits for terrain, no guessed height |
| First-use prank | Protection before countdown; random script; high native hop; half-speed fall; ground correction |
| Glider during prank | Blocked until correction/restoration |
| Landing | No immediate or delayed fall damage; normal glider/gravity return |
| Second use / another character on same account | No repeated prank on that host |
| Another account | Independent first use; no changes to other players' physics |
| Regular deluxe | High arrival, usable glider, normal fall speed/damage, no automatic correction |
| Disconnect during descent | Reconnect performs protected ground correction, not another high prank |
| Host restart mid-trip | Pending ledger survives; recovery completes on reconnect |
| Death/respawn during trip | Verify recovery and no lingering restrictions or repeated high prank |

After normal travel passes, enable the prank and restart the host. After its completed first use, enable deluxe and restart to test regular high arrivals. Confirm the selected height stays below the game's actual out-of-bounds respawn before intentional gliding.

## Repeating first use

Offline and self-hosted play from one installation share the host ledger. To repeat first use, close that host with no active trip, preserve map-teleport-history.txt as a test backup, and start with a fresh ledger. Never remove a pending recovery entry during a live trip. Restore the intended ledger after the test process closes. Dedicated installations have their own ledger.

## Evidence and rollback

Record mode, candidate version, config, target, animation, landing, glider behavior and relevant greeneye/loader.log entries. Automated simulation is not a gameplay pass.

If normal travel, terrain or protection fails, stop progression to high-arrival tests and retain logs. Restore backed-up mod files with the process stopped. Preserve pending test history for recovery investigation: 0.1.1 does not implement this candidate's recovery.
