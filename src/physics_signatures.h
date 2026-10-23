#pragma once
namespace teleport::native {
inline constexpr char fall_damagePattern[]="40 55 41 57 48 8D AC 24 08 FF FF FF 48 81 EC F8 01 00 00 41 B8 C8 00 00 00 48 8D 54 24 60 4C 8B F9 E8 ?? ?? ?? ?? 41 B8 C8 00 00 00 48 8D 54 24 60 49 8B CF E8 ?? ?? ?? ?? 84 C0 0F 84 ?? ?? ?? ??";
inline constexpr char gliderPattern[]="40 53 48 81 EC E0 00 00 00 41 B8 80 00 00 00 48 8D 54 24 40 48 8B D9 E8 ?? ?? ?? ?? 41 B8 80 00 00 00 48 8D 54 24 40 48 8B CB E8 ?? ?? ?? ?? 84 C0 0F 84 ?? ?? ?? ?? 66 0F 1F 84 00 00 00 00 00";
inline constexpr char gravityPattern[]="48 89 7C 24 20 55 48 8D 6C 24 A9 48 81 EC F0 00 00 00 41 B8 58 00 00 00 48 8D 55 A7 48 8B F9 E8 ?? ?? ?? ?? 41 B8 58 00 00 00 48 8D 55 A7 48 8B CF E8 ?? ?? ?? ?? 84 C0 0F 84 ?? ?? ?? ?? 0F 29 BC 24 D0 00 00 00";
inline constexpr char player_motionPattern[]="40 55 56 48 8D AC 24 88 FD FF FF 48 81 EC 78 03 00 00 48 8D 95 98 02 00 00 48 8B F1 E8 ?? ?? ?? ?? 48 83 38 00 0F 84 ?? ?? ?? ?? 41 B8 F8 01 00 00 48 8D 54 24 60 48 8B CE E8 ?? ?? ?? ?? 41 B8 F8 01 00 00";
inline constexpr char fall_detectionPattern[]="40 55 41 56 48 8D AC 24 58 FF FF FF 48 81 EC A8 01 00 00 41 B8 70 00 00 00 48 8D 54 24 30 4C 8B F1 E8 ?? ?? ?? ?? 41 B8 70 00 00 00 48 8D 54 24 30 49 8B CE E8 ?? ?? ?? ?? 84 C0 0F 84 ?? ?? ?? ??";
}
