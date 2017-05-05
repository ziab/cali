#include "Icosahedron.h"

#include <map>
#include <array>

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

		using Index = UInt32;

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

		using Lookup = std::map<std::pair<Index, Index>, Index>;

		Index vertex_for_edge(Lookup& lookup,
			VertexList& vertices, Index first, Index second)
		{
			Lookup::key_type key(first, second);
			if (key.first > key.second)	std::swap(key.first, key.second);

			auto inserted = lookup.insert({ key, (Index)vertices.size() });
			if (inserted.second)
			{
				auto& edge0 = vertices[first];
				auto& edge1 = vertices[second];
				auto point = (edge0 + edge1);
				point.Normalize();
				vertices.push_back(point);
			}

			return inserted.first->second;
		}

		TriangleList subdivide(VertexList& vertices, TriangleList triangles)
		{
			Lookup lookup;
			TriangleList result;

			for (auto&& each : triangles)
			{
				std::array<Index, 3> mid;
				for (int edge = 0; edge<3; ++edge)
				{
					mid[edge] = vertex_for_edge(lookup, vertices,
						each.vertex[edge], each.vertex[(edge + 1) % 3]);
				}

				result.push_back({ each.vertex[0], mid[0], mid[2] });
				result.push_back({ each.vertex[1], mid[1], mid[0] });
				result.push_back({ each.vertex[2], mid[2], mid[1] });
				result.push_back({ mid[0], mid[1], mid[2] });
			}

			return result;
		}

		using IndexedMesh = std::pair<VertexList, TriangleList>;

		void make_icosphere(VertexList& vertices, TriangleList& triangles, size_t subdivisions)
		{
			vertices = icosahedron::vertices;
			triangles = icosahedron::triangles;

			for (size_t i = 0; i < subdivisions; ++i)
			{
				triangles = subdivide(vertices, triangles);
			}
		}
	}

	void Icosahedron::create_icosahedron()
	{
		VertexList ico_vertices; TriangleList ico_triangles;
		icosahedron::make_icosphere(ico_vertices, ico_triangles, 3);

		const size_t indices_total = ico_triangles.size() * 3;
		const size_t vertex_total = ico_vertices.size();

		m_model.allocate(indices_total, vertex_total);
		auto& indices = m_model.load_indicies();
		auto& vertices = m_model.load_vertices();

		for (size_t i = 0; i < vertex_total; ++i)
		{
			auto& curr = vertices[i];
			curr.position = ico_vertices[i];
			curr.normal = { 0.0f, 1.0f, 0.0f };
			curr.texturecoord = { 0.0f, 0.0f };
		}

		auto* icosahedron_indices = reinterpret_cast<const uint32_t*>(ico_triangles.data());

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
