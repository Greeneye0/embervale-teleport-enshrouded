#pragma once
#include <cstdint>

namespace teleport {
// Development-only, read-only terrain sample. Registry is supplied by the
// caller's current world; samples never move an entity or write world voxels.
enum class TerrainProbeState : uint32_t { unavailable, loading, invalid, ready };
struct TerrainProbe {
    uint32_t size{sizeof(TerrainProbe)}, version{1};
    void* registry{};
    double x{}, z{}, startY{};
    TerrainProbeState state{TerrainProbeState::unavailable};
    uint32_t realm{};
    uint64_t sourceAgeMs{};
    double height{};
    char reason[96]{};
};
using TerrainProbeFn = int (*)(TerrainProbe*);
}
