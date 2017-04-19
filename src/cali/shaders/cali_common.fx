struct VS_UPWND
{
    float2 uv : TEXCOORD0;
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD1;
    float3 normal : NORMAL;
    float4 depth : TEXTURE0;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : NORMAL;
    float4 depth : TEXTURE0;
};

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

float map_in_range(float value_to_map, float input_min, float input_max, float output_min, float output_max)
{
    // TODO : optimize!
    /*
        // more readable way:
        slope = (output_end - output_start) / (input_end - input_start)
        output = output_start + slope * (input - input_start)
    */
    return output_min + ((output_max - output_min) / (input_max - input_min)) * (value_to_map - input_min);
}