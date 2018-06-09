#pragma once
#include <vector>
#include <IvMatrix33.h>
#include <IvVector2.h>
#include <IvVector3.h>
#include <IvVector4.h>
#include <IvVertexShader.h>
#include <IvVertexBuffer.h>
#include <IvIndexBuffer.h>
#include <IvShaderProgram.h>
#include <IvRenderer.h>

#include "Model.h"

namespace cali
{
	class icosahedron : public physical
	{
		model<kTNPFormat, IvTNPVertex> m_model;

	private:
		void create_icosahedron();
	public:

		icosahedron() 
		{ 
			physical::set_position({ 0.0f, 100.0f, 50.0f });
			create_icosahedron();
		};
		~icosahedron() {};

		void render(IvRenderer& renderer, IvShaderProgram* shader) const;
	};
}