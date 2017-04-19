///////////////////////////////////////////////
//          GLSL to HLSL definitions
///////////////////////////////////////////////

//#define IN in
//#define OUT out
//#define vec3 float3
//#define vec4 float4

///////////////////////////////////////////////

#define Length float
#define Wavelength float
#define Angle float
#define SolidAngle float
#define Power float
#define LuminousPower float

#define Number float
#define Area float
#define Volume float
#define NumberDensity float
#define Irradiance float
#define Radiance float
#define SpectralPower float
#define SpectralIrradiance float
#define SpectralRadiance float
#define SpectralRadianceDensity float
#define ScatteringCoefficient float
#define InverseSolidAngle float
#define LuminousIntensity float
#define Luminance float
#define Illuminance float

// Use combined rayleigh(rgb) and mia(a) texture
#define COMBINED_SCATTERING_TEXTURES

// A generic function from Wavelength to some other type.
#define AbstractSpectrum float3
// A function from Wavelength to Number.
#define DimensionlessSpectrum float3
// A function from Wavelength to SpectralPower.
#define PowerSpectrum float3
// A function from Wavelength to SpectralIrradiance.
#define IrradianceSpectrum float3
// A function from Wavelength to SpectralRadiance.
#define RadianceSpectrum float3
// A function from Wavelength to SpectralRadianceDensity.
#define RadianceDensitySpectrum float3
// A function from Wavelength to ScaterringCoefficient.
#define ScatteringSpectrum float3

// A position in 3D (3 length values).
#define Position float3
// A unit direction vector in 3D (3 unitless values).
#define Direction float3
// A vector of 3 luminance values.
#define Luminance3 float3
// A vector of 3 illuminance values.
#define Illuminance3 float3

#define TransmittanceTexture Texture2D
#define AbstractScatteringTexture Texture3D
#define ReducedScatteringTexture Texture3D
#define ScatteringTexture Texture3D
#define ScatteringDensityTexture Texture3D
#define IrradianceTexture Texture2D

struct AtmosphereParameters
{
     // The solar irradiance at the top of the atmosphere.
    IrradianceSpectrum solar_irradiance; // float3 // r = 98242.786221752205, g = 69954.398111511371, b = 66475.012354368271
    // The sun's angular radius.
    Angle sun_angular_radius; // float
    // The distance between the planet center and the bottom of the atmosphere.
    Length bottom_radius; // float
    // The distance between the planet center and the top of the atmosphere.
    Length top_radius; // float
    // The scale height of air molecules, meaning that their density is
    // proportional to exp(-h / rayleigh_scale_height), with h the altitude
    // (with the bottom of the atmosphere at altitude 0).
    Length rayleigh_scale_height; // float
    // The scattering coefficient of air molecules at the bottom of the
    // atmosphere, as a function of wavelength.
    ScatteringSpectrum rayleigh_scattering; // float3
    // The scale height of aerosols, meaning that their density is proportional
    // to exp(-h / mie_scale_height), with h the altitude.
    Length mie_scale_height; // float
    // The scattering coefficient of aerosols at the bottom of the atmosphere,
    // as a function of wavelength.
    ScatteringSpectrum mie_scattering; // float3
    // The extinction coefficient of aerosols at the bottom of the atmosphere,
    // as a function of wavelength.
    ScatteringSpectrum mie_extinction; // float3
    // The asymetry parameter for the Cornette-Shanks phase function for the
    // aerosols.
    Number mie_phase_function_g; // float
    // The average albedo of the ground.
    DimensionlessSpectrum ground_albedo; // float3
    // The cosine of the maximum Sun zenith angle for which atmospheric scattering
    // must be precomputed (for maximum precision, use the smallest Sun zenith
    // angle yielding negligible sky light radiance values. For instance, for the
    // Earth case, 102 degrees is a good choice - yielding mu_s_min = -0.2).
    Number mu_s_min; // float
};

static const Length m = 1.0;
static const Wavelength nm = 1.0;
static const Angle rad = 1.0;
static const SolidAngle sr = 1.0;
static const Power watt = 1.0;
static const LuminousPower lm = 1.0;
static const float PI = 3.14159265358979323846;
static const Length km = 1000.0 * m;
static const Area m2 = m * m;
static const Volume m3 = m * m * m;
static const Angle pi = PI * rad;
static const Angle deg = pi / 180.0;
static const Irradiance watt_per_square_meter = watt / m2;
static const Radiance watt_per_square_meter_per_sr = watt / (m2 * sr);
static const SpectralIrradiance watt_per_square_meter_per_nm = watt / (m2 * nm);
static const SpectralRadiance watt_per_square_meter_per_sr_per_nm = watt / (m2 * sr * nm);
static const SpectralRadianceDensity watt_per_cubic_meter_per_sr_per_nm = watt / (m3 * sr * nm);
static const LuminousIntensity cd = lm / sr;
static const LuminousIntensity kcd = 1000.0 * cd;
static const Luminance cd_per_square_meter = cd / m2;
static const Luminance kcd_per_square_meter = kcd / m2;

#include "BrunetonCommonDefs.h"

static const float2 IRRADIANCE_TEXTURE_SIZE =
    float2(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);

static const float3 SKY_SPECTRAL_RADIANCE_TO_LUMINANCE = float3(114974.916437, 71305.954816, 65310.548555);
static const float3 SUN_SPECTRAL_RADIANCE_TO_LUMINANCE = float3(98242.786222, 69954.398112, 66475.012354);

static const AtmosphereParameters ATMOSPHERE =
{
    float3(1.474000, 1.850400, 1.911980), // solar_irradiance
    0.004675, // sun_angular_radius
    6360.000000, // bottom_radius
    6420.000000, // top_radius
    8.000000, // rayleigh_scale_height
    float3(0.005802, 0.013558, 0.033100), // rayleigh_scattering
    1.200000, // mie_scale_height
    float3(0.003996, 0.003996, 0.003996), // mie_scattering
    float3(0.004440, 0.004440, 0.004440), // mie_extinction
    0.800000, // mie_phase_function_g
    float3(0.100000, 0.100000, 0.100000), // ground_albedo
    -0.207912 // mu_s_min
};

Number GetUnitRangeFromTextureCoord(Number u, int texture_size)
{
    return (u - 0.5 / Number(texture_size)) / (1.0 - 1.0 / Number(texture_size));
}

Number GetTextureCoordFromUnitRange(Number x, int texture_size)
{
    return 0.5 / Number(texture_size) + x * (1.0 - 1.0 / Number(texture_size));
}

Number ClampCosine(Number mu)
{
    return clamp(mu, Number(-1.0), Number(1.0));
}

Length ClampDistance(Length d)
{
    return max(d, 0.0 * m);
}

Length ClampRadius(in AtmosphereParameters atmosphere, Length r) 
{
    return clamp(r, atmosphere.bottom_radius, atmosphere.top_radius);
}

bool RayIntersectsGround(in AtmosphereParameters atmosphere,
    Length r, Number mu) 
{
    //assert(r >= atmosphere.bottom_radius);
    //assert(mu >= -1.0 && mu <= 1.0);
    return mu < 0.0 && r * r * (mu * mu - 1.0) +
        atmosphere.bottom_radius * atmosphere.bottom_radius >= 0.0 * m2;
}

Length SafeSqrt(Area a)
{
    return sqrt(max(a, 0.0 * m2));
}

float GlMod(float x, float y)
{
    return x - y * floor(x / y);
}

InverseSolidAngle RayleighPhaseFunction(Number nu)
{
  InverseSolidAngle k = 3.0 / (16.0 * PI * sr);
    return k * (1.0 + nu * nu);
}

InverseSolidAngle MiePhaseFunction(Number g, Number nu)
{
  InverseSolidAngle k = 3.0 / (8.0 * PI * sr) * (1.0 - g * g) / (2.0 + g * g);
    return k * (1.0 + nu * nu) / pow(abs(1.0 + g * g - 2.0 * g * nu), 1.5);
}

Length DistanceToTopAtmosphereBoundary(in AtmosphereParameters atmosphere, Length r, Number mu)
{
    //assert(r <= atmosphere.top_radius);
    //assert(mu >= -1.0 && mu <= 1.0);
    Area discriminant = r * r * (mu * mu - 1.0) +
        atmosphere.top_radius * atmosphere.top_radius;
    return ClampDistance(-r * mu + SafeSqrt(discriminant));
}

float2 GetTransmittanceTextureUvFromRMu(in AtmosphereParameters atmosphere,
    Length r, Number mu) 
{
    //assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
    //assert(mu >= -1.0 && mu <= 1.0);
    Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius -
        atmosphere.bottom_radius * atmosphere.bottom_radius);
    Length rho =
        SafeSqrt(r * r - atmosphere.bottom_radius * atmosphere.bottom_radius);
    Length d = DistanceToTopAtmosphereBoundary(atmosphere, r, mu);
    Length d_min = atmosphere.top_radius - r;
    Length d_max = rho + H;
    Number x_mu = (d - d_min) / (d_max - d_min);
    Number x_r = rho / H;
    return float2(GetTextureCoordFromUnitRange(x_mu, TRANSMITTANCE_TEXTURE_WIDTH),
                GetTextureCoordFromUnitRange(x_r, TRANSMITTANCE_TEXTURE_HEIGHT));
}

DimensionlessSpectrum GetTransmittanceToTopAtmosphereBoundary(
    in AtmosphereParameters atmosphere,
    in TransmittanceTexture transmittance_texture,
    in SamplerState transmittance_sampler,
    Length r, Number mu) 
{
    //assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
    float2 uv = GetTransmittanceTextureUvFromRMu(atmosphere, r, mu);
    return DimensionlessSpectrum(transmittance_texture.Sample(transmittance_sampler, uv).rgb);
}

DimensionlessSpectrum GetTransmittance(
    in AtmosphereParameters atmosphere,
    in TransmittanceTexture transmittance_texture,
    in SamplerState transmittance_sampler,
    Length r, Number mu, Length d, bool ray_r_mu_intersects_ground) 
{
    //assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
    //assert(mu >= -1.0 && mu <= 1.0);
    //assert(d >= 0.0 * m);
    Length r_d = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
    Number mu_d = ClampCosine((r * mu + d) / r_d);
    if (ray_r_mu_intersects_ground) {
    return min(
        GetTransmittanceToTopAtmosphereBoundary(
            atmosphere, transmittance_texture, transmittance_sampler, r_d, -mu_d) /
        GetTransmittanceToTopAtmosphereBoundary(
            atmosphere, transmittance_texture, transmittance_sampler, r, -mu),
        DimensionlessSpectrum(1.0, 1.0, 1.0));
    } else {
    return min(
        GetTransmittanceToTopAtmosphereBoundary(
            atmosphere, transmittance_texture, transmittance_sampler, r, mu) /
        GetTransmittanceToTopAtmosphereBoundary(
            atmosphere, transmittance_texture, transmittance_sampler, r_d, mu_d),
        DimensionlessSpectrum(1.0, 1.0, 1.0));
    }
}

Length DistanceToBottomAtmosphereBoundary(in AtmosphereParameters atmosphere,
    Length r, Number mu) 
{
  //assert(r >= atmosphere.bottom_radius);
  //assert(mu >= -1.0 && mu <= 1.0);
    Area discriminant = r * r * (mu * mu - 1.0) +
        atmosphere.bottom_radius * atmosphere.bottom_radius;
    return ClampDistance(-r * mu - SafeSqrt(discriminant));
}

Length DistanceToNearestAtmosphereBoundary(in AtmosphereParameters atmosphere,
    Length r, Number mu, bool ray_r_mu_intersects_ground) 
{
    if (ray_r_mu_intersects_ground) {
        return DistanceToBottomAtmosphereBoundary(atmosphere, r, mu);
    } else {
        return DistanceToTopAtmosphereBoundary(atmosphere, r, mu);
    }
}

void GetRMuMuSNuFromScatteringTextureUvwz(in AtmosphereParameters atmosphere,
    in float4 uvwz, out Length r, out Number mu, out Number mu_s,
    out Number nu, out bool ray_r_mu_intersects_ground)
{
    //assert(uvwz.x >= 0.0 && uvwz.x <= 1.0);
    //assert(uvwz.y >= 0.0 && uvwz.y <= 1.0);
    //assert(uvwz.z >= 0.0 && uvwz.z <= 1.0);
    //assert(uvwz.w >= 0.0 && uvwz.w <= 1.0);
    Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius -
        atmosphere.bottom_radius * atmosphere.bottom_radius);
    Length rho =
        H * GetUnitRangeFromTextureCoord(uvwz.w, SCATTERING_TEXTURE_R_SIZE);
    r = sqrt(rho * rho + atmosphere.bottom_radius * atmosphere.bottom_radius);
    if (uvwz.z < 0.5)
    {
        Length d_min = r - atmosphere.bottom_radius;
        Length d_max = rho;
        Length d = d_min + (d_max - d_min) * GetUnitRangeFromTextureCoord(
            1.0 - 2.0 * uvwz.z, SCATTERING_TEXTURE_MU_SIZE / 2);
        mu = d == 0.0 * m ? Number(-1.0) :
            ClampCosine(-(rho * rho + d * d) / (2.0 * r * d));
        ray_r_mu_intersects_ground = true;
    }
    else
    {
        Length d_min = atmosphere.top_radius - r;
        Length d_max = rho + H;
        Length d = d_min + (d_max - d_min) * GetUnitRangeFromTextureCoord(
            2.0 * uvwz.z - 1.0, SCATTERING_TEXTURE_MU_SIZE / 2);
        mu = d == 0.0 * m ? Number(1.0) :
            ClampCosine((H * H - rho * rho - d * d) / (2.0 * r * d));
        ray_r_mu_intersects_ground = false;
    }
    Number x_mu_s =
        GetUnitRangeFromTextureCoord(uvwz.y, SCATTERING_TEXTURE_MU_S_SIZE);
    Length d_min = atmosphere.top_radius - atmosphere.bottom_radius;
    Length d_max = H;
    Number A =
        -2.0 * atmosphere.mu_s_min * atmosphere.bottom_radius / (d_max - d_min);
    Number a = (A - x_mu_s * A) / (1.0 + x_mu_s * A);
    Length d = d_min + min(a, A) * (d_max - d_min);
    mu_s = d == 0.0 * m ? Number(1.0) :
        ClampCosine((H * H - d * d) / (2.0 * atmosphere.bottom_radius * d));
    nu = ClampCosine(uvwz.x * 2.0 - 1.0);
}

void GetRMuMuSNuFromScatteringTextureFragCoord(
    in AtmosphereParameters atmosphere, in float3 gl_frag_coord,
    out Length r, out Number mu, out Number mu_s, out Number nu,
    out bool ray_r_mu_intersects_ground)
{
    const float4 SCATTERING_TEXTURE_SIZE = float4(
        SCATTERING_TEXTURE_NU_SIZE - 1,
        SCATTERING_TEXTURE_MU_S_SIZE,
        SCATTERING_TEXTURE_MU_SIZE,
        SCATTERING_TEXTURE_R_SIZE);
    Number frag_coord_nu =
        floor(gl_frag_coord.x / Number(SCATTERING_TEXTURE_MU_S_SIZE));
    Number frag_coord_mu_s =
        GlMod(gl_frag_coord.x, Number(SCATTERING_TEXTURE_MU_S_SIZE));
    float4 uvwz =
        float4(frag_coord_nu, frag_coord_mu_s, gl_frag_coord.y, gl_frag_coord.z) /
            SCATTERING_TEXTURE_SIZE;
    GetRMuMuSNuFromScatteringTextureUvwz(
        atmosphere, uvwz, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
    nu = clamp(nu, mu * mu_s - sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)),
        mu * mu_s + sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)));
}

float4 GetScatteringTextureUvwzFromRMuMuSNu(in AtmosphereParameters atmosphere,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground) 
{
    //assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
    //assert(mu >= -1.0 && mu <= 1.0);
    //assert(mu_s >= -1.0 && mu_s <= 1.0);
    //assert(nu >= -1.0 && nu <= 1.0);
    Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius -
        atmosphere.bottom_radius * atmosphere.bottom_radius);
    Length rho =
        SafeSqrt(r * r - atmosphere.bottom_radius * atmosphere.bottom_radius);
    Number u_r = GetTextureCoordFromUnitRange(rho / H, SCATTERING_TEXTURE_R_SIZE);
    Length r_mu = r * mu;
    Area discriminant =
        r_mu * r_mu - r * r + atmosphere.bottom_radius * atmosphere.bottom_radius;
    Number u_mu;
    if (ray_r_mu_intersects_ground) {
    Length d = -r_mu - SafeSqrt(discriminant);
    Length d_min = r - atmosphere.bottom_radius;
    Length d_max = rho;
    u_mu = 0.5 - 0.5 * GetTextureCoordFromUnitRange(d_max == d_min ? 0.0 :
        (d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2);
    } else {
    Length d = -r_mu + SafeSqrt(discriminant + H * H);
    Length d_min = atmosphere.top_radius - r;
    Length d_max = rho + H;
    u_mu = 0.5 + 0.5 * GetTextureCoordFromUnitRange(
        (d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2);
    }
    Length d = DistanceToTopAtmosphereBoundary(
        atmosphere, atmosphere.bottom_radius, mu_s);
    Length d_min = atmosphere.top_radius - atmosphere.bottom_radius;
    Length d_max = H;
    Number a = (d - d_min) / (d_max - d_min);
    Number A =
        -2.0 * atmosphere.mu_s_min * atmosphere.bottom_radius / (d_max - d_min);
    Number u_mu_s = GetTextureCoordFromUnitRange(
        max(1.0 - a / A, 0.0) / (1.0 + a), SCATTERING_TEXTURE_MU_S_SIZE);
    Number u_nu = (nu + 1.0) / 2.0;
    return float4(u_nu, u_mu_s, u_mu, u_r);
}

//TEMPLATE(AbstractSpectrum)
AbstractSpectrum GetScattering(
    in AtmosphereParameters atmosphere,
    in AbstractScatteringTexture scattering_texture,
    in SamplerState scattering_textureSampler,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground)
{
    float4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu(
      atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
    Number tex_coord_x = uvwz.x * Number(SCATTERING_TEXTURE_NU_SIZE - 1);
    Number tex_x = floor(tex_coord_x);
    Number lerp = tex_coord_x - tex_x;
    float3 uvw0 = float3((tex_x + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);
    float3 uvw1 = float3((tex_x + 1.0 + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);

    float3 scattering = float4(scattering_texture.Sample(scattering_textureSampler, uvw0) * (1.0 - lerp) +
      scattering_texture.Sample(scattering_textureSampler, uvw1) * lerp).rgb;

    return AbstractSpectrum(scattering);
}

RadianceSpectrum GetScattering(
    in AtmosphereParameters atmosphere,
    in ReducedScatteringTexture single_rayleigh_scattering_texture,
    in SamplerState single_rayleigh_scattering_textureSampler,
    in ReducedScatteringTexture single_mie_scattering_texture,
    in SamplerState single_mie_scattering_textureSampler,
    in ScatteringTexture multiple_scattering_texture,
    in SamplerState multiple_scattering_textureSampler,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground,
    int scattering_order) 
{
    if (scattering_order == 1) {
    IrradianceSpectrum rayleigh = GetScattering(
        atmosphere, single_rayleigh_scattering_texture, single_rayleigh_scattering_textureSampler, r, mu, mu_s, nu,
        ray_r_mu_intersects_ground);
    IrradianceSpectrum mie = GetScattering(
        atmosphere, single_mie_scattering_texture, single_mie_scattering_textureSampler, r, mu, mu_s, nu,
        ray_r_mu_intersects_ground);
    return rayleigh * RayleighPhaseFunction(nu) +
        mie * MiePhaseFunction(atmosphere.mie_phase_function_g, nu);
    } else {
    return GetScattering(
        atmosphere, multiple_scattering_texture, multiple_scattering_textureSampler, r, mu, mu_s, nu,
        ray_r_mu_intersects_ground);
    }
}

float2 GetIrradianceTextureUvFromRMuS(in AtmosphereParameters atmosphere,
    Length r, Number mu_s) 
{
    //assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
    //assert(mu_s >= -1.0 && mu_s <= 1.0);
    Number x_r = (r - atmosphere.bottom_radius) /
          (atmosphere.top_radius - atmosphere.bottom_radius);
    Number x_mu_s = mu_s * 0.5 + 0.5;
    return float2(GetTextureCoordFromUnitRange(x_mu_s, IRRADIANCE_TEXTURE_WIDTH),
                  GetTextureCoordFromUnitRange(x_r, IRRADIANCE_TEXTURE_HEIGHT));
}

IrradianceSpectrum GetIrradiance(
    in AtmosphereParameters atmosphere,
    in IrradianceTexture irradiance_texture,
    in SamplerState irradiance_textureSampler,
    Length r, Number mu_s) 
{
    float2 uv = GetIrradianceTextureUvFromRMuS(atmosphere, r, mu_s);
    return IrradianceSpectrum(irradiance_texture.Sample(irradiance_textureSampler, uv).rgb);
}

float3 GetExtrapolatedSingleMieScattering(
    in AtmosphereParameters atmosphere, in float4 scattering) 
{
    if (scattering.r == 0.0) 
    {
        return float3(0.0, 0.0, 0.0);
    }
    return scattering.rgb * scattering.a / scattering.r *
        (atmosphere.rayleigh_scattering.r / atmosphere.mie_scattering.r) *
        (atmosphere.mie_scattering / atmosphere.rayleigh_scattering);
}

IrradianceSpectrum GetCombinedScattering(
    in AtmosphereParameters atmosphere,
    in ReducedScatteringTexture scattering_texture,
    in SamplerState scattering_textureSampler,
    in ReducedScatteringTexture single_mie_scattering_texture,
    in SamplerState ssingle_mie_scattering_textureSampler,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground,
    out IrradianceSpectrum single_mie_scattering) 
{
    float4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu(
        atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);

    Number tex_coord_x = uvwz.x * Number(SCATTERING_TEXTURE_NU_SIZE - 1);
    Number tex_x = floor(tex_coord_x);
    Number lerp = tex_coord_x - tex_x;
    float3 uvw0 = float3((tex_x + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE), uvwz.z, uvwz.w);
    float3 uvw1 = float3((tex_x + 1.0 + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE), uvwz.z, uvwz.w);

#ifdef COMBINED_SCATTERING_TEXTURES
    float4 combined_scattering =
        scattering_texture.Sample(scattering_textureSampler, uvw0) * (1.0 - lerp) +
        scattering_texture.Sample(scattering_textureSampler, uvw1) * lerp;
    IrradianceSpectrum scattering = IrradianceSpectrum(combined_scattering.rgb);
    single_mie_scattering = GetExtrapolatedSingleMieScattering(atmosphere, combined_scattering);
#else
    IrradianceSpectrum scattering = IrradianceSpectrum(
        texture(scattering_texture, uvw0) * (1.0 - lerp) +
        texture(scattering_texture, uvw1) * lerp);
    single_mie_scattering = IrradianceSpectrum(
        texture(single_mie_scattering_texture, uvw0) * (1.0 - lerp) +
        texture(single_mie_scattering_texture, uvw1) * lerp);
#endif
    return scattering;
}

RadianceSpectrum GetSkyRadiance(
    in AtmosphereParameters atmosphere,
    in TransmittanceTexture transmittance_texture,
    in SamplerState transmittance_textureSampler,
    in ReducedScatteringTexture scattering_texture,
    in SamplerState scattering_textureSampler,
    in ReducedScatteringTexture single_mie_scattering_texture,
    in SamplerState single_mie_scattering_textureSampler,
    Position camera, in Direction view_ray, Length shadow_length,
    in Direction sun_direction, out DimensionlessSpectrum transmittance) 
{
    Length r = length(camera);
    Length rmu = dot(camera, view_ray);
    Length distance_to_top_atmosphere_boundary = -rmu -
        sqrt(rmu * rmu - r * r + atmosphere.top_radius * atmosphere.top_radius);
    if (distance_to_top_atmosphere_boundary > 0.0 * m) 
    {
        camera = camera + view_ray * distance_to_top_atmosphere_boundary;
        r = atmosphere.top_radius;
        rmu += distance_to_top_atmosphere_boundary;
    }
    if (r > atmosphere.top_radius) 
    {
        transmittance = DimensionlessSpectrum(1.0, 1.0, 1.0);
        float component = 0.0 * watt_per_square_meter_per_sr_per_nm;
        return RadianceSpectrum(component, component, component);
    }
    Number mu = rmu / r;
    Number mu_s = dot(camera, sun_direction) / r;
    Number nu = dot(view_ray, sun_direction);
    bool ray_r_mu_intersects_ground = RayIntersectsGround(atmosphere, r, mu);
    
    transmittance = ray_r_mu_intersects_ground ? DimensionlessSpectrum(0.0, 0.0, 0.0) :
        GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittance_texture, transmittance_textureSampler, r, mu);

    IrradianceSpectrum single_mie_scattering;
    IrradianceSpectrum scattering;
    if (shadow_length == 0.0 * m) 
    {
        scattering = GetCombinedScattering(
            atmosphere, 
            scattering_texture, 
            scattering_textureSampler, 
            single_mie_scattering_texture, 
            single_mie_scattering_textureSampler,
            r, mu, mu_s, nu, ray_r_mu_intersects_ground,
            single_mie_scattering);
    } 
    else 
    {
        Length d = shadow_length;
        Length r_p = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
        Number mu_p = (r * mu + d) / r_p;
        Number mu_s_p = (r * mu_s + d * nu) / r_p;
        
        scattering = GetCombinedScattering(
            atmosphere, scattering_texture, scattering_textureSampler, 
            single_mie_scattering_texture, single_mie_scattering_textureSampler,
            r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground,
            single_mie_scattering);
        
        DimensionlessSpectrum shadow_transmittance =
            GetTransmittance(atmosphere, transmittance_texture, transmittance_textureSampler,
                r, mu, shadow_length, ray_r_mu_intersects_ground);
        
        scattering = scattering * shadow_transmittance;
        single_mie_scattering = single_mie_scattering * shadow_transmittance;
    }

    return scattering * RayleighPhaseFunction(nu) + single_mie_scattering *
        MiePhaseFunction(atmosphere.mie_phase_function_g, nu);
}

RadianceSpectrum GetSkyRadianceToPoint(
    in  AtmosphereParameters atmosphere,
    in TransmittanceTexture transmittance_texture,
    in SamplerState transmittance_textureSampler,
    in ReducedScatteringTexture scattering_texture,
    in SamplerState scattering_textureSampler,
    in ReducedScatteringTexture single_mie_scattering_texture,
    in SamplerState single_mie_scattering_textureSampler,
    Position camera, in Position _point, Length shadow_length,
    in Direction sun_direction, out DimensionlessSpectrum transmittance) 
{
    Direction view_ray = normalize(_point - camera);
    Length r = length(camera);
    Length rmu = dot(camera, view_ray);
    Length distance_to_top_atmosphere_boundary = -rmu - sqrt(rmu * rmu - r * r + atmosphere.top_radius * atmosphere.top_radius);
    if (distance_to_top_atmosphere_boundary > 0.0 * m) 
    {
        camera = camera + view_ray *
        distance_to_top_atmosphere_boundary;
        r = atmosphere.top_radius;
        rmu += distance_to_top_atmosphere_boundary;
    }
    Number mu = rmu / r;
    Number mu_s = dot(camera, sun_direction) / r;
    Number nu = dot(view_ray, sun_direction);
    Length d = length(_point - camera);
    bool ray_r_mu_intersects_ground = RayIntersectsGround(atmosphere, r, mu);
    transmittance = GetTransmittance(atmosphere, transmittance_texture, transmittance_textureSampler,
        r, mu, d, ray_r_mu_intersects_ground);

    IrradianceSpectrum single_mie_scattering;
    IrradianceSpectrum scattering = GetCombinedScattering(
        atmosphere, scattering_texture, scattering_textureSampler, 
        single_mie_scattering_texture, single_mie_scattering_textureSampler,
        r, mu, mu_s, nu, ray_r_mu_intersects_ground, single_mie_scattering);

    d = max(d - shadow_length, 0.0 * m);
    Length r_p = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
    Number mu_p = (r * mu + d) / r_p;
    Number mu_s_p = (r * mu_s + d * nu) / r_p;
    IrradianceSpectrum single_mie_scattering_p;
    
    IrradianceSpectrum scattering_p = GetCombinedScattering(
        atmosphere, scattering_texture, scattering_textureSampler, 
        single_mie_scattering_texture, single_mie_scattering_textureSampler,
        r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground,
        single_mie_scattering_p);

    DimensionlessSpectrum shadow_transmittance = transmittance;
    if (shadow_length > 0.0 * m) 
    {
        shadow_transmittance = GetTransmittance(atmosphere, transmittance_texture, transmittance_textureSampler,
            r, mu, d, ray_r_mu_intersects_ground);
    }
    scattering = scattering - shadow_transmittance * scattering_p;
    single_mie_scattering = single_mie_scattering - shadow_transmittance * single_mie_scattering_p;

    #ifdef COMBINED_SCATTERING_TEXTURES
    single_mie_scattering = GetExtrapolatedSingleMieScattering(
        atmosphere, float4(scattering, single_mie_scattering.r));
    #endif
    single_mie_scattering = single_mie_scattering *
        smoothstep(Number(0.0), Number(0.01), mu_s);
    return scattering * RayleighPhaseFunction(nu) + single_mie_scattering *
        MiePhaseFunction(atmosphere.mie_phase_function_g, nu);
}

IrradianceSpectrum GetSunAndSkyIrradiance(
    in AtmosphereParameters atmosphere,
    in TransmittanceTexture transmittance_texture,
    in SamplerState transmittance_textureSampler,
    in IrradianceTexture irradiance_texture,
    in SamplerState irradiance_textureSampler,
    in Position _point, in Direction normal, in Direction sun_direction,
    out IrradianceSpectrum sky_irradiance) 
{
    Length r = length(_point);
    Number mu_s = dot(_point, sun_direction) / r;

    sky_irradiance = GetIrradiance(atmosphere, irradiance_texture, irradiance_textureSampler, r, mu_s) *
        (1.0 + dot(normal, _point) / r) * 0.5;
  
    return atmosphere.solar_irradiance *
      GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittance_texture, transmittance_textureSampler, r, mu_s) *
        smoothstep(-atmosphere.sun_angular_radius / rad, atmosphere.sun_angular_radius / rad, mu_s) *
        max(dot(normal, sun_direction), 0.0);
}