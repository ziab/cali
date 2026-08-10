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

// Voronoi – tileable, cellSize in texels, returns distance in [0,1] and border, and cell value
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

IvTexture* generate_heightmap_texture(uint64_t seed,int width,int height){
    auto& renderer=*IvRenderer::mRenderer;
    auto& resman=*renderer.GetResourceManager();
    std::vector<unsigned char> data((size_t)width*height*3);
    int periodX=width, periodY=height;
    uint64_t seedBase=seed;
    uint64_t seedDetail=splitmix64(seed+0x123456789ABCDEF0ULL);
    uint64_t seedVorL=splitmix64(seed+0xA5A5A5A5A5A5A5A5ULL);
    uint64_t seedVorS=splitmix64(seed+0x5A5A5A5A5A5A5A5AULL);
    uint64_t seedMountVar=splitmix64(seed+0xC0FFEE123456789ULL);
    const float cellLarge = (float)width / 7.0f;
    const float cellSmall = (float)width / 22.0f;
    for(int y=0;y<height;++y) for(int x=0;x<width;++x){
        // normalized domain with integer periods for perfect tiling
        // continent: 4 periods across texture
        float u_cont = (float)x / width * 4.0f;
        float v_cont = (float)y / height * 4.0f;
        float continent = fbm_internal(u_cont, v_cont, seedBase, 4, 0.45f, 2.0f, 4, 4);
        // voronoi large – soft cellular continents, tileable via cell count 7
        float vorL = voronoi((float)x,(float)y,cellLarge,seedVorL,periodX,periodY);
        float vorCellVal;
        voronoi((float)x,(float)y,cellLarge,seedVorL,periodX,periodY,nullptr,&vorCellVal);
        float continentVor = 1.0f - vorL;
        continentVor = powf(continentVor, 1.7f);
        float base = lerp_f(continent, continentVor, 0.38f);
        base = std::clamp((base - 0.33f) / 0.60f, 0.0f, 1.0f);
        base = powf(base, 0.88f);

        // detail: 18 periods, softer
        float u_det = (float)x / width * 18.0f;
        float v_det = (float)y / height * 18.0f;
        float detail = fbm_internal(u_det, v_det, seedDetail, 3, 0.40f, 2.2f, 18, 18);
        detail = (detail - 0.5f) * 0.14f; // very soft variation

        // small voronoi ridges
        float vorS = voronoi((float)x,(float)y,cellSmall,seedVorS,periodX,periodY);
        float ridge = 1.0f - fabsf(vorS*2.0f - 1.0f);
        ridge = powf(std::max(0.0f, ridge), 1.9f) * 0.12f;

        // per-cell mountain variability: large cells have random height factor 0.6..1.6
        float mountVar = 0.7f + vorCellVal * 0.9f; // 0.7-1.6, some high, some mid
        // increase ridge/mountain 3x overall, with variability
        ridge *= mountVar * 3.0f;
        // add extra peak for high cells
        float h = base + detail + ridge;
        h = std::clamp(h, 0.0f, 1.0f);
        h = smootherstep(h);

        const float sea = 0.44f;
        float tex;
        if(h < sea){
            float t = h / sea;
            t = powf(t, 1.15f);
            tex = t * 0.0038f; // ocean 0..0.0038 -> height 0..9.2 (water)
        }else{
            float t = (h - sea) / (1.0f - sea);
            t = powf(t, 0.92f);
            // land: 3x higher with per-cell variability (some high, some mid)
            tex = 0.0045f + t * 0.45f * (0.85f + 0.3f*vorCellVal);
            if(t > 0.60f){
                float m = (t - 0.60f)/0.40f;
                tex += m*m * 0.35f * mountVar;
            }
            if(tex > 1.0f) tex = 1.0f;
        }
        tex = std::clamp(tex, 0.0f, 1.0f);
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
}
}
