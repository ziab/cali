#include "bruneton_common.fx"

Texture2D transmittance_texture;
SamplerState transmittance_textureSampler;
Texture3D scattering_texture;
SamplerState scattering_textureSampler;
Texture3D single_mie_scattering_texture;
SamplerState single_mie_scattering_textureSampler;
Texture2D irradiance_texture;
SamplerState irradiance_textureSampler;

RadianceSpectrum GetSkyRadiance(
    Position camera, 
    Direction view_ray, 
    Length shadow_length,
    Direction sun_direction, 
    out DimensionlessSpectrum transmittance) 
{
    return GetSkyRadiance(
        ATMOSPHERE, 
        transmittance_texture, 
        transmittance_textureSampler,
        scattering_texture, 
        scattering_textureSampler, 
        single_mie_scattering_texture, 
        single_mie_scattering_textureSampler,
        camera, 
        view_ray, 
        shadow_length, 
        sun_direction, 
        transmittance);
}

RadianceSpectrum GetSkyRadianceToPoint(
    Position camera, Position _point, Length shadow_length,
    Direction sun_direction, out DimensionlessSpectrum transmittance) 
{
    return GetSkyRadianceToPoint(
        ATMOSPHERE, 
        transmittance_texture, 
        transmittance_textureSampler,
        scattering_texture, 
        scattering_textureSampler, 
        single_mie_scattering_texture, 
        single_mie_scattering_textureSampler,
        camera, 
        _point, 
        shadow_length, 
        sun_direction, 
        transmittance);
}

IrradianceSpectrum GetSunAndSkyIrradiance(
    Position p, Direction normal, Direction sun_direction,
    out IrradianceSpectrum sky_irradiance) 
{
    return GetSunAndSkyIrradiance(ATMOSPHERE, transmittance_texture, transmittance_textureSampler,
        irradiance_texture, irradiance_textureSampler, p, normal, sun_direction, sky_irradiance);
}
Luminance3 GetSkyLuminance(
    Position camera, Direction view_ray, Length shadow_length,
    Direction sun_direction, out DimensionlessSpectrum transmittance) 
{
    return GetSkyRadiance(camera, view_ray, shadow_length, sun_direction,
        transmittance) * SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
}
Luminance3 GetSkyLuminanceToPoint(
    Position camera, Position _point, Length shadow_length,
    Direction sun_direction, out DimensionlessSpectrum transmittance) 
{
    return GetSkyRadianceToPoint(camera, _point, shadow_length, sun_direction, transmittance) * SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
}

Illuminance3 GetSunAndSkyIlluminance(
    Position p, Direction normal, Direction sun_direction,
    out IrradianceSpectrum sky_irradiance) 
{
    IrradianceSpectrum sun_irradiance = GetSunAndSkyIrradiance(p, normal, sun_direction, sky_irradiance);
    sky_irradiance *= SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
    return sun_irradiance * SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
}