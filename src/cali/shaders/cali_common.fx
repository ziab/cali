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
