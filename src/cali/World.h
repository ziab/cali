#pragma once
#include <IvVector3.h>
#include <IvVector4.h>

#include <limits>
#include <string>

#undef max

namespace cali
{
	namespace world
	{
		static const float c_camera_far = std::numeric_limits<float>::max() / 100000000000.f;
		static const float c_earth_radius = 63600.0f;
		static const IvVector3 c_earth_center = { 0.0, -c_earth_radius, 0.0 };

		static float c_horizon_distance = 100000000000000.f;

		static const IvVector3 c_sky_box_size = 
		{
            c_earth_radius * 4,
            c_earth_radius * 4,
            c_earth_radius * 4
		};

        static const IvVector3 c_star_box_size =
        {
            c_camera_far,
            c_camera_far,
            c_camera_far
        };

        static const float c_star_distance = world::c_earth_radius * 100.f;
        static const float c_star_visible_size_min = 4000.f;
        static const float c_star_visible_size_max = 7000.f;


		static const IvVector4 sun_zenith_color = { 1.f, 1.f, 1.f, 1.f };
		static const IvVector4 sun_sunset_color = { 252.f / 255.f, 108.f / 255.f, 45.f / 255.f, 1.f };

		// Procedural planet seed: same hash => same terrain (stable generation)
		static const inline std::string c_planet_hash = "cali_planet_v1";
		static const int c_heightmap_size = 1024; // legacy tiling
		static const int c_planet_heightmap_width = 8192;  // 8192x4096 RGB24 = 100MB, within 200MB budget
		static const int c_planet_heightmap_height = 4096;
		static const int c_planet_heightmap_bytes = c_planet_heightmap_width * c_planet_heightmap_height * 3; // ~100MB
		static_assert(c_planet_heightmap_bytes <= 200 * 1024 * 1024, "heightmap exceeds 200MB budget");
	}
}
