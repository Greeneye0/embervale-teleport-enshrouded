#pragma once
namespace teleport::native {
inline constexpr char queueTeleportPattern[]="48 89 74 24 10 57 48 83 EC 20 44 8B 52 10 48 8B FA 41 8B C2 83 E0 0F 74 ?? 44 2B D0 41 83 C2 10 43 8D 34 0A 8B C6 48 3B 42 08 76 ?? 33 C0 48 8B 74 24 38 48 83 C4 20 5F C3 41 8B D2 48 89 5C 24 30";
inline constexpr char notifyTeleportPattern[]="40 53 48 83 EC 30 8B C2 41 B9 10 00 00 00 41 8B D8 89 44 24 20 45 0F B6 C1 48 8D 51 10 E8 ?? ?? ?? ?? 48 85 C0 75 ?? 48 83 C4 30 5B C3 89 58 0C B0 01 48 83 C4 30 5B C3 CC CC CC CC CC CC CC CC";
}
