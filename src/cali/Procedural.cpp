#include "Procedural.h"

#include <IvRenderer.h>
#include <IvResourceManager.h>
#include <IvTexture.h>

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstring>

namespace cali
{
namespace proc
{

uint64_t hash_string(const std::string& s)
{
    uint64_t h = 14695981039346656037ULL;
    for (unsigned char c : s) { h ^= c; h *= 1099511628211ULL; }
    return splitmix64(h);
}
uint64_t hash_string(const char* s){ if(!s) return 0; return hash_string(std::string(s)); }
uint64_t splitmix64(uint64_t x){
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}
static inline uint64_t hash_coords(int x,int y,uint64_t seed){
    uint64_t h = seed;
    h ^= (uint64_t)(uint32_t)x * 0x9e3779b97f4a7c15ULL;
    h ^= (uint64_t)(uint32_t)y * 0xbf58476d1ce4e5b9ULL;
    h ^= ((uint64_t)(uint32_t)x << 32) | (uint32_t)y;
    return splitmix64(h);
}
static inline float hash_to_float(uint64_t h){ return (h >> 32) * (1.0f / 4294967295.0f); }
static inline float lerp_f(float a,float b,float t){ return a + t*(b-a); }
static inline float smootherstep(float t){ return t*t*t*(t*(t*6 -15)+10); }

static float value_noise(float x,float y,uint64_t seed,int periodX,int periodY){
    int xi=(int)floorf(x), yi=(int)floorf(y);
    float xf=x-(float)xi, yf=y-(float)yi;
    float u=smootherstep(xf), v=smootherstep(yf);
    auto wrap=[](int v,int p)->int{ if(p<=0) return v; int r=v%p; if(r<0) r+=p; return r; };
    int xi0=wrap(xi,periodX), yi0=wrap(yi,periodY);
    int xi1=wrap(xi+1,periodX), yi1=wrap(yi+1,periodY);
    float h00=hash_to_float(hash_coords(xi0,yi0,seed));
    float h10=hash_to_float(hash_coords(xi1,yi0,seed));
    float h01=hash_to_float(hash_coords(xi0,yi1,seed));
    float h11=hash_to_float(hash_coords(xi1,yi1,seed));
    float x1=lerp_f(h00,h10,u), x2=lerp_f(h01,h11,u);
    return lerp_f(x1,x2,v);
}
static float fbm_internal(float x,float y,uint64_t seed,int octaves,float persistence,float lacunarity,int periodX,int periodY){
    float total=0, amp=1, freq=1, maxAmp=0;
    for(int i=0;i<octaves;++i){
        int pX=periodX>0?(int)(periodX*freq):0; if(pX==0) pX=periodX;
        int pY=periodY>0?(int)(periodY*freq):0; if(pY==0) pY=periodY;
        float n=value_noise(x*freq,y*freq,seed+(uint64_t)i*0x9e3779b97f4a7c15ULL,pX,pY);
        total+=n*amp; maxAmp+=amp; amp*=persistence; freq*=lacunarity;
    }
    return total/maxAmp;
}
float sample_fbm(float x,float y,uint64_t seed,int periodX,int periodY){ return fbm_internal(x,y,seed,6,0.5f,2.0f,periodX,periodY); }

static float voronoi(float x,float y,float cellSize,uint64_t seed,int periodX,int periodY, float* outBorder=nullptr, float* outCellValue=nullptr){
    int cellsX = periodX>0 ? std::max(1, (int)(periodX / cellSize + 0.5f)) : 64;
    int cellsY = periodY>0 ? std::max(1, (int)(periodY / cellSize + 0.5f)) : 64;
    float fx = x / cellSize, fy = y / cellSize;
    int cxi = (int)floorf(fx), cyi = (int)floorf(fy);
    float fxf = fx - cxi, fyf = fy - cyi;
    float minDist=1e6, secondDist=1e6;
    int bestCx=0,bestCy=0;
    for(int dy=-1;dy<=1;++dy) for(int dx=-1;dx<=1;++dx){
        int ncx=cxi+dx, ncy=cyi+dy;
        int wcx = ((ncx % cellsX)+cellsX)%cellsX;
        int wcy = ((ncy % cellsY)+cellsY)%cellsY;
        uint64_t h = hash_coords(wcx,wcy,seed);
        float ox = hash_to_float(h);
        uint64_t h2 = hash_coords(wcx,wcy,seed ^ 0x9e3779b97f4a7c15ULL);
        float oy = hash_to_float(h2);
        float px = (float)dx + ox - fxf;
        float py = (float)dy + oy - fyf;
        float d = sqrtf(px*px + py*py);
        if(d < minDist){ secondDist=minDist; minDist=d; bestCx=wcx; bestCy=wcy; }
        else if(d < secondDist){ secondDist=d; }
    }
    if(outBorder) *outBorder = secondDist - minDist;
    if(outCellValue){
        uint64_t hc = hash_coords(bestCx,bestCy,seed ^ 0x6a09e667f3bcc908ULL);
        *outCellValue = hash_to_float(hc);
    }
    return std::clamp(minDist / 1.41421356f, 0.0f, 1.0f);
}

// 3D helpers for planet sphere (non-tiled, unique)
static inline uint64_t hash_coords3(int x,int y,int z,uint64_t seed){
    uint64_t h = seed;
    h ^= (uint64_t)(uint32_t)x * 0x9e3779b97f4a7c15ULL;
    h ^= (uint64_t)(uint32_t)y * 0xbf58476d1ce4e5b9ULL;
    h ^= (uint64_t)(uint32_t)z * 0x94d049bb133111ebULL;
    h ^= ((uint64_t)(uint32_t)x << 32) | (uint32_t)y;
    h ^= ((uint64_t)(uint32_t)z << 16);
    return splitmix64(h);
}
static float value_noise3(float x,float y,float z,uint64_t seed){
    int xi=(int)floorf(x), yi=(int)floorf(y), zi=(int)floorf(z);
    float xf=x-xi, yf=y-yi, zf=z-zi;
    float u=smootherstep(xf), v=smootherstep(yf), w=smootherstep(zf);
    float h000=hash_to_float(hash_coords3(xi, yi, zi, seed));
    float h100=hash_to_float(hash_coords3(xi+1, yi, zi, seed));
    float h010=hash_to_float(hash_coords3(xi, yi+1, zi, seed));
    float h110=hash_to_float(hash_coords3(xi+1, yi+1, zi, seed));
    float h001=hash_to_float(hash_coords3(xi, yi, zi+1, seed));
    float h101=hash_to_float(hash_coords3(xi+1, yi, zi+1, seed));
    float h011=hash_to_float(hash_coords3(xi, yi+1, zi+1, seed));
    float h111=hash_to_float(hash_coords3(xi+1, yi+1, zi+1, seed));
    float x00=lerp_f(h000,h100,u), x10=lerp_f(h010,h110,u), x01=lerp_f(h001,h101,u), x11=lerp_f(h011,h111,u);
    float y0=lerp_f(x00,x10,v), y1=lerp_f(x01,x11,v);
    return lerp_f(y0,y1,w);
}
static float fbm3_internal(float x,float y,float z,uint64_t seed,int octaves,float persistence,float lacunarity){
    float tot=0, amp=1, freq=1, maxA=0;
    for(int i=0;i<octaves;++i){
        float n=value_noise3(x*freq,y*freq,z*freq,seed+(uint64_t)i*0x9e3779b97f4a7c15ULL);
        tot+=n*amp; maxA+=amp; amp*=persistence; freq*=lacunarity;
    }
    return tot/maxA;
}

IvTexture* generate_heightmap_texture(uint64_t seed,int width,int height){
    auto& renderer=*IvRenderer::mRenderer;
    auto& resman=*renderer.GetResourceManager();
    std::vector<unsigned char> data((size_t)width*height*3);
    int periodX=width, periodY=height;
    uint64_t seedBase=seed;
    uint64_t seedDetail=splitmix64(seed+0x123456789ABCDEF0ULL);
    uint64_t seedVorL=splitmix64(seed+0xA5A5A5A5A5A5A5A5ULL);
    uint64_t seedVorS=splitmix64(seed+0x5A5A5A5A5A5A5A5AULL);
    const float cellLarge = (float)width / 7.0f;
    const float cellSmall = (float)width / 28.0f;
    for(int y=0;y<height;++y) for(int x=0;x<width;++x){
        // soft continents: low frequency, few octaves, low persistence
        float u_cont = (float)x / width * 3.5f;
        float v_cont = (float)y / height * 3.5f;
        float continent = fbm_internal(u_cont, v_cont, seedBase, 3, 0.42f, 2.0f, 4, 4);
        // large voronoi soft blend – subtle, not dominant
        float vorL = voronoi((float)x,(float)y,cellLarge,seedVorL,periodX,periodY);
        float vorCellVal;
        voronoi((float)x,(float)y,cellLarge,seedVorL,periodX,periodY,nullptr,&vorCellVal);
        float continentVor = 1.0f - vorL;
        continentVor = powf(continentVor, 2.2f); // very soft
        float base = lerp_f(continent, continentVor, 0.20f); // only 20% voronoi influence
        // remap to 0-1 with soft contrast
        base = std::clamp((base - 0.38f) / 0.50f, 0.0f, 1.0f);
        base = lerp_f(base, smootherstep(base), 0.4f); // soften

        // fine detail – very low amplitude for soft hills
        float u_det = (float)x / width * 22.0f;
        float v_det = (float)y / height * 22.0f;
        float detail = fbm_internal(u_det, v_det, seedDetail, 2, 0.40f, 2.2f, 22, 22);
        detail = (detail - 0.5f) * 0.08f; // tiny variation

        // mountain ridges – only where base is high (mountain mask)
        float mountainMask = smootherstep(std::clamp((base - 0.45f)/0.35f, 0.0f, 1.0f)); // 0 in lowlands, 1 in highlands
        mountainMask = powf(mountainMask, 0.9f);
        float mountVar = 0.55f + vorCellVal * 1.1f; // 0.55-1.65, high vs mid

        // small Voronoi ridges localized to mountains
        float vorS = voronoi((float)x,(float)y,cellSmall,seedVorS,periodX,periodY);
        float ridgeS = 1.0f - fabsf(vorS*2.0f - 1.0f);
        ridgeS = powf(std::max(0.0f, ridgeS), 2.2f) * 0.10f * mountainMask * mountVar;

        // large ridge at continent borders – subtle
        float vorBorder; voronoi((float)x,(float)y,cellLarge,seedVorL,periodX,periodY,&vorBorder,nullptr);
        float largeRidge = powf(std::max(0.0f, 1.0f - vorBorder*3.0f), 2.0f) * 0.06f * mountainMask * mountVar;

        float h = base + detail + ridgeS + largeRidge;
        h = std::clamp(h, 0.0f, 1.0f);
        // final soften
        h = lerp_f(h, smootherstep(h), 0.25f);

        const float sea = 0.50f; // more ocean (50%)
        float tex;
        if(h < sea){
            float t = h / sea;
            t = powf(t, 1.2f);
            tex = t * 0.0032f; // ocean 0..0.0032 -> height 0..8.5
        }else{
            float t = (h - sea) / (1.0f - sea);
            t = powf(t, 0.88f);
            // base land, 3x peaks with variability
            // high cells get up to 3x, mid cells ~1.5x
            float peakScale = 0.55f * (0.75f + 0.5f*vorCellVal); // 0.41-0.68
            tex = 0.0042f + t * peakScale * mountVar * 0.55f;
            // extra high peaks for very high t, variable
            if(t > 0.62f){
                float m = (t - 0.62f)/0.38f;
                tex += powf(m, 1.6f) * 0.28f * mountVar;
            }
            if(tex > 1.0f) tex = 1.0f;
        }
        tex = std::clamp(tex, 0.0f, 1.0f);
        // add tiny hash dither to avoid banding
        float dither = (hash_to_float(hash_coords(x,y,seed ^ 0x9E3779B97F4A7C15ULL)) - 0.5f) * (0.5f/255.0f);
        tex = std::clamp(tex + dither, 0.0f, 1.0f);
        uint8_t v = (uint8_t)std::clamp((int)roundf(tex*255.0f),0,255);
        size_t idx=((size_t)y*width+x)*3;
        data[idx+0]=v; data[idx+1]=v; data[idx+2]=v;
    }
    IvTexture* tex = resman.CreateTexture(kRGB24TexFmt,width,height,data.data(),kDefaultUsage);
    if(!tex) return nullptr;
    tex->SetAddressingU(kWrapTexAddr);
    tex->SetAddressingV(kWrapTexAddr);
    tex->SetMagFiltering(kBilerpTexMagFilter);
    tex->SetMinFiltering(kBilerpTexMinFilter);
    return tex;
}

IvTexture* generate_planet_heightmap(uint64_t seed,int width,int height){
    // 200MB budget: e.g. 8192x4096 RGB24 = 100MB, 8192x8192 = 200MB
    // Default 8192x4096 stays well within 200MB and is 2:1 equirectangular.
    auto& renderer=*IvRenderer::mRenderer;
    auto& resman=*renderer.GetResourceManager();
    std::vector<unsigned char> data((size_t)width*height*3);
    uint64_t seedBase=seed;
    uint64_t seedDetail=splitmix64(seed+0x123456789ABCDEF0ULL);
    uint64_t seedRidge=splitmix64(seed+0xA5A5A5A5A5A5A5A5ULL);
    // 3D large voronoi for per-region mountain variability (cell ~0.6 rad)
    // we use 2D voronoi in lat/lon space for cheap variability, but evaluate via 3D hash for stability
    const float PI=3.14159265359f;
    for(int y=0;y<height;++y){
        float v = (float)y / (height-1); // 0 south ..1 north
        float lat = v * PI - PI*0.5f;
        float cosLat = cosf(lat);
        float sinLat = sinf(lat);
        for(int x=0;x<width;++x){
            float u = (float)x / width; // 0..1
            float lon = u * 2.0f*PI - PI;
            float cosLon = cosf(lon), sinLon = sinf(lon);
            // Y-up sphere
            float px = cosLat * sinLon;
            float py = sinLat;
            float pz = cosLat * cosLon;
            // 3D fbm at sphere surface – continents (low freq)
            float continent = fbm3_internal(px*2.2f, py*2.2f, pz*2.2f, seedBase, 4, 0.45f, 2.0f);
            // 2D voronoi large for soft continent shape – evaluate in equirectangular with wrap in lon
            // we cheat: use 2D voronoi in (u*7, v*3.5) space
            float vorU = u * 7.0f, vorV = v * 3.5f;
            // wrap voronoi in U only (lon wraps), not V
            float vorL = voronoi(vorU* (width/7.0f), vorV* (height/3.5f), width/7.0f, seedBase, width, height/2);
            float vorCellVal;
            voronoi(vorU* (width/7.0f), vorV* (height/3.5f), width/7.0f, seedBase, width, height/2, nullptr, &vorCellVal);
            float continentVor = 1.0f - vorL;
            continentVor = powf(continentVor, 2.0f);
            float base = lerp_f(continent, continentVor, 0.22f);
            base = std::clamp((base - 0.36f)/0.52f, 0.0f, 1.0f);
            base = lerp_f(base, smootherstep(base), 0.3f);

            // small detail – high freq 3D
            float detail = fbm3_internal(px*14.0f, py*14.0f, pz*14.0f, seedDetail, 2, 0.40f, 2.2f);
            detail = (detail - 0.5f) * 0.09f;

            float mountainMask = smootherstep(std::clamp((base - 0.48f)/0.30f, 0.0f, 1.0f));
            float mountVar = 0.55f + vorCellVal * 1.1f; // 0.55-1.65

            float ridge = fbm3_internal(px*9.0f, py*9.0f, pz*9.0f, seedRidge, 2, 0.5f, 2.0f);
            ridge = 1.0f - fabsf(ridge*2.0f - 1.0f);
            ridge = powf(std::max(0.0f, ridge), 2.2f) * 0.11f * mountainMask * mountVar * 1.2f;

            float h = base + detail + ridge;
            h = std::clamp(h, 0.0f, 1.0f);
            h = lerp_f(h, smootherstep(h), 0.20f);

            const float sea = 0.50f;
            float texv;
            if(h < sea){
                float t = h / sea;
                t = powf(t, 1.15f);
                texv = t * 0.0034f;
            }else{
                float t = (h - sea)/(1.0f - sea);
                t = powf(t, 0.90f);
                float peakScale = 0.55f * (0.75f + 0.5f*vorCellVal);
                texv = 0.0042f + t * 0.48f * peakScale * (0.9f + 0.2f*mountVar);
                if(t > 0.62f){
                    float m = (t - 0.62f)/0.38f;
                    texv += powf(m, 1.5f) * 0.30f * mountVar;
                }
                if(texv > 1.0f) texv = 1.0f;
            }
            texv = std::clamp(texv, 0.0f, 1.0f);
            float dither = (hash_to_float(hash_coords(x,y,seed ^ 0x9E3779B97F4A7C15ULL)) - 0.5f) * (0.5f/255.0f);
            texv = std::clamp(texv + dither, 0.0f, 1.0f);
            uint8_t vv = (uint8_t)std::clamp((int)roundf(texv*255.0f),0,255);
            size_t idx=((size_t)y*width+x)*3;
            data[idx+0]=vv; data[idx+1]=vv; data[idx+2]=vv;
        }
    }
    IvTexture* tex2 = resman.CreateTexture(kRGB24TexFmt,width,height,data.data(),kDefaultUsage);
    if(!tex2) return nullptr;
    tex2->SetAddressingU(kWrapTexAddr); // lon wraps
    tex2->SetAddressingV(kClampTexAddr); // lat clamp at poles
    tex2->SetMagFiltering(kBilerpTexMagFilter);
    tex2->SetMinFiltering(kBilerpTexMinFilter);
    return tex2;
}
}
}
