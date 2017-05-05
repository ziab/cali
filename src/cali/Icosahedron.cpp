#include "Icosahedron.h"

namespace Cali
{
	struct Triangle
	{
		uint32_t vertex[3];
	};

	using TriangleList = std::vector<Triangle>;
	using VertexList = std::vector<IvVector3>;

	namespace icosahedron
	{
		const float X = .525731112119133606f;
		const float Z = .850650808352039932f;
		const float N = 0.f;

		static const VertexList vertices =
		{
			{ -X,N,Z },{ X,N,Z },{ -X,N,-Z },{ X,N,-Z },
			{ N,Z,X },{ N,Z,-X },{ N,-Z,X },{ N,-Z,-X },
			{ Z,X,N },{ -Z,X, N },{ Z,-X,N },{ -Z,-X, N }
		};

		static const TriangleList triangles =
		{
			{ 0,4,1 },{ 0,9,4 },{ 9,5,4 },{ 4,5,8 },{ 4,8,1 },
			{ 8,10,1 },{ 8,3,10 },{ 5,3,8 },{ 5,2,3 },{ 2,7,3 },
			{ 7,10,3 },{ 7,6,10 },{ 7,11,6 },{ 11,0,6 },{ 0,1,6 },
			{ 6,1,10 },{ 9,0,11 },{ 9,11,2 },{ 9,2,5 },{ 7,2,11 }
		};
	}

	void create_icosahedron_model(VertexList& veticies, TriangleList& triangles, size_t subdivisions)
	{

	}

	void Icosahedron::create_icosahedron()
	{
		const size_t indices_total = icosahedron::triangles.size() * 3;
		const size_t vertex_total = icosahedron::vertices.size();

		m_model.allocate(indices_total, vertex_total);
		auto& indices = m_model.load_indicies();
		auto& vertices = m_model.load_vertices();

		for (size_t i = 0; i < vertex_total; ++i)
		{
			auto& curr = vertices[i];
			curr.position = icosahedron::vertices[i];
			curr.normal = { 0.0f, 1.0f, 0.0f };
			curr.texturecoord = { 0.0f, 0.0f };
		}

		auto* icosahedron_indices = reinterpret_cast<const uint32_t*>(icosahedron::triangles.data());

		for (size_t i = 0; i < indices_total; ++i)
		{
			auto& curr = indices[i];
			curr = icosahedron_indices[i];
		}
	}

	void Icosahedron::render(IvRenderer & renderer, IvShaderProgram * shader) const
	{
		set_transformation_matrix(renderer);
		m_model.render(renderer, shader);
	}
}
