#pragma once
#include <cstdint>
#include <string>

class IvTexture;

namespace cali
{
namespace proc
{
    uint64_t hash_string(const std::string& s);
    uint64_t hash_string(const char* s);
    uint64_t splitmix64(uint64_t x);

    // Generate a tileable heightmap texture. Seed determines terrain; same seed => same terrain.
    // Width/height should be power-of-two for best tiling (default 1024).
    IvTexture* generate_heightmap_texture(uint64_t seed, int width = 1024, int height = 1024);

    inline IvTexture* generate_heightmap_texture(const std::string& hash_str, int w = 1024, int h = 1024)
    {
        return generate_heightmap_texture(hash_string(hash_str), w, h);
    }

    // Exposed for testing: single sample in [0,1] using tileable FBM
    float sample_fbm(float x, float y, uint64_t seed, int periodX, int periodY);
}
}
