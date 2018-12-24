#pragma once
#include "Renderable.h"
#include "Model.h"
#include "IvRenderTexture.h"

namespace cali
{
    class stars : public physical
    {
        model<kTNPFormat, IvTNPVertex> m_box;
        IvShaderProgram* m_shader{ nullptr };

    public:
        stars();
        ~stars();

        virtual void update(float dt);
        virtual void render(IvRenderer& renderer);
    };
}