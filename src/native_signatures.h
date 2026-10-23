#pragma once
namespace teleport::native {
inline constexpr char playerPattern[] = "48 89 4C 24 08 55 41 56 48 8D AC 24 E8 F5 FF FF 48 81 EC 18 0B 00 00 41 B8 A8 05 00 00 48 8D 55 00 4C 8B F1 E8 ?? ?? ?? ?? 41 B8 A8 05 00 00 48 8D 55 00 49 8B CE E8 ?? ?? ?? ?? 84 C0 0F 84 ?? ?? ?? ??";
inline constexpr char applyPattern[] = "40 55 56 48 8D AC 24 A8 FD FF FF 48 81 EC 58 03 00 00 41 B8 B0 01 00 00 48 8D 55 90 48 8B F1 E8 ?? ?? ?? ?? 41 B8 B0 01 00 00 48 8D 55 90 48 8B CE E8 ?? ?? ?? ?? 84 C0 0F 84 ?? ?? ?? ?? 48 89 9C 24 70 03 00 00";
inline constexpr char nextPattern[] = "40 57 48 83 EC 20 4C 8B 01 48 8B FA 8B 41 08 4C 8B C9 49 3B 80 30 08 00 00 75 ?? 32 C0 48 83 C4 20 5F C3 41 0F B6 80 B0 04 00 00 49 8D 88 70 04 00 00 48 89 5C 24 38 45 33 DB 41 0F B7 98 BA 04 00 00";
inline constexpr char entityPattern[] = "8B 41 08 4C 8B C2 85 C0 75 ?? 89 02 48 8B C2 C3 8D 50 FF 48 8B 01 48 03 D2 48 8B 88 28 08 00 00 8B 04 D1 41 89 00 49 8B C0 C3 CC CC CC CC CC CC 8B 05 ?? ?? ?? ?? 48 85 C9 74 ?? 8B 41 10 C3 CC";
inline constexpr char rayPattern[] = "48 8B C4 48 89 50 10 55 53 57 41 54 48 8D A8 58 FF FF FF 48 81 EC 88 01 00 00 41 0F 28 08 48 8B F9 F2 0F 10 05 ?? ?? ?? ?? 49 8B D9 0F 29 70 A8 4D 8B E0 41 0F 28 31 0F 29 78 98 0F 28 FE 44 0F 29 40 88";
inline constexpr char dirtyPattern[] = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 83 EC 30 48 8B B1 90 02 00 00 49 8B D9 0F B7 6C 24 70 4D 8B F8 4C 8B D2 4C 8B F1 44 8B A6 DC 04 00 00 48 85 D2";
}
