struct VS_UPWN
{
    float2 uv : TEXCOORD0;
    float4 screen_position : SV_POSITION;
    float3 world_position : TEXCOORD1;
    float3 normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 screen_position : SV_POSITION;
    float3 world_position : TEXCOORD0;
    float3 normal : NORMAL;
};

/*
struct VS_TERRAIN_OUTPUT
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : NORMAL;
    float alpha : BLENDWEIGHT0;
};
*/

cbuffer SunLight : register(b1)
{
    float3 world_origin;
    float3 world_up;
    float3 camera_position;
    float4 sky_color_zenith;
    float4 sky_color_horizon;
    float3 sun_position;
    float3 sun_intensity;
    float3 sun_attenuation;
    float4 sun_color;
}

float3 quad_lerp(
		in float3 a,
		in float3 b,
		in float3 c,
		in float3 d,
		in float2 uv)
{
	// Given a (u,v) coordinate that defines a 2D local position inside a planar quadrilateral, find the
	// absolute 3D (x,y,z) coordinate at that location.
	//
	//  0 <----u----> 1
	//  a ----------- b    0
	//  |             |   /|\
	//  |             |    |
	//  |             |    v
	//  |  *(u,v)     |    |
	//  |             |   \|/
	//  d------------ c    1
	//
	// a, b, c, and d are the vertices of the quadrilateral. They are assumed to exist in the
	// same plane in 3D space, but this function will allow for some non-planar error.
	//
	// Variables u and v are the two-dimensional local coordinates inside the quadrilateral.
	// To find a point that is inside the quadrilateral, both u and v must be between 0 and 1 inclusive.  
	// For example, if you send this function u=0, v=0, then it will return coordinate "a".  
	// Similarly, coordinate u=1, v=1 will return vector "c". Any values between 0 and 1
	// will return a coordinate that is bi-linearly interpolated between the four vertices.

    float3 abu = lerp(a, b, uv.x);
    float3 dcu = lerp(d, c, uv.x);
    return lerp(abu, dcu, uv.y);
}