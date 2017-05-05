#pragma once
#include <IvVector3.h>
#include <IvVector4.h>

#include <limits>

#undef max

namespace Cali
{
	namespace World
	{
		static const float c_camera_far = std::numeric_limits<float>::max() / 100000000000.f;
		static const float c_earth_radius = 6360000.0f;
		static const IvVector3 c_earth_center = { 0.0, -c_earth_radius, 0.0 };

		static float c_horizon_distance = 100000000000000.f;

		static const IvVector3 c_sky_box_size = 
		{
			c_horizon_distance,
			c_horizon_distance,
			c_horizon_distance
		};

		static const IvVector4 sun_zenith_color = { 1.f, 1.f, 1.f, 1.f };
		static const IvVector4 sun_sunset_color = { 252.f / 255.f, 108.f / 255.f, 45.f / 255.f, 1.f };
	}
}