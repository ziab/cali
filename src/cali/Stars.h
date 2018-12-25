#pragma once
#include "Renderable.h"
#include "Model.h"
#include "IvRenderTexture.h"

namespace cali
{
    class star : public physical
    {
    public:
        star(const IvVector3& position, float scale) 
        { 
            set_position(position); 
            set_scale(scale);
        }

        ~star() override {}
    };

    class stars : public physical
    {
        //model<kTNPFormat, IvTNPVertex> m_box;
        model<kTNPFormat, IvTNPVertex> m_quad;
        std::vector<star> m_stars;
        IvShaderProgram* m_shader{ nullptr };

        void generate_stars(size_t count);

    public:
        stars();
        ~stars() override;

        virtual void update(float dt);
        virtual void render(IvRenderer& renderer);
    };
}