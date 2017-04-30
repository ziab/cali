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

namespace Cali
{
	class Icosahedron : public Physical
	{
		Model<kTNPFormat, IvTNPVertex> m_model;

	private:
		void create_icosahedron();
	public:

		Icosahedron() 
		{ 
			Physical::set_position({ 0.0f, 100.0f, 50.0f });
			create_icosahedron();
		};
		~Icosahedron() {};

		void render(IvRenderer& renderer, IvShaderProgram* shader) const;
	};
}