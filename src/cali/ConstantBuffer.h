#pragma once

#ifndef ALIGN16
#define ALIGN16 __declspec(align(16))
#endif

#include <IvVector3.h>
#include <IvVector4.h>

namespace Cali
{
	namespace ConstantBuffer
	{
		ALIGN16 struct GlobalState
		{
			ALIGN16 IvVector3 world_origin;
			ALIGN16 IvVector3 world_up;
			ALIGN16 IvVector3 camera_position;
			ALIGN16 IvVector4 sky_color_zenith;
			ALIGN16 IvVector4 sky_color_horizon;
			ALIGN16 IvVector3 sun_position;
			ALIGN16 IvVector3 sun_intensity;
			ALIGN16 IvVector3 sun_attenuation;
			ALIGN16 IvVector4 sun_color;
		};
	}
}