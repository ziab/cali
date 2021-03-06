#pragma once
#include <IvMatrix33.h>
#include <IvVector2.h>
#include <IvVector3.h>
#include <IvVector4.h>
#include <IvVertexShader.h>
#include <IvVertexBuffer.h>
#include <IvIndexBuffer.h>
#include <IvShaderProgram.h>
#include <IvRenderer.h>
#include <IvResourceManager.h>

#include <memory>
#include <vector>

#include "Renderable.h"

namespace cali
{
	template<typename t_VertexFormat>
	inline void reverse_winding(std::vector<UInt32>& indices, std::vector<t_VertexFormat>& vertices_unused)
	{
		for (auto it = indices.begin(); it != indices.end(); it += 3)
		{
			std::swap(*it, *(it + 2));
		}
	}

	inline void invert_normals(std::vector<IvNPVertex>& vertices)
	{
		for (auto it = vertices.begin(); it != vertices.end(); ++it)
		{
			it->normal.x = -it->normal.x;
			it->normal.y = -it->normal.y;
			it->normal.z = -it->normal.z;
		}
	}

	template<typename t_Buffer, typename t_Data>
	class BufferRAIIWrapper
	{
		t_Buffer* m_buffer;
		t_Data* m_data;
	public:
		BufferRAIIWrapper(t_Buffer* buffer, t_Data* m_data) :
			m_buffer(buffer), m_data(m_data)
		{
		}

		~BufferRAIIWrapper()
		{
			m_buffer->EndLoadData();
		}

		t_Data& operator[] (size_t offset)
		{
			return m_data[offset];
		}

		const t_Data& operator[] (size_t offset) const
		{
			return m_data[offset];
		}
	};

    template <typename T>
    T move_value_from(T& that)
    {
        auto temp = that;
        that = {};
        return temp;
    }

	template<IvVertexFormat vertex_format_code, typename t_VertexFormat>
	class model
	{
		IvRenderer& m_renderer;
		IvPrimType m_primitive_type;

		size_t m_indices_count;
		IvIndexBuffer* m_indices;

		size_t m_vertices_count;
		IvVertexBuffer* m_vertices;

	public:
		model() :
			m_renderer(*IvRenderer::mRenderer),
			m_primitive_type(kTriangleListPrim),
			m_indices_count(0),
			m_indices(nullptr),
			m_vertices_count(0),
			m_vertices(nullptr)
		{
		}

        // non copyable for now
        model(const model& that) = delete;
        model& operator=(const model& that) = delete;

        model& operator=(model&& that)
        {
            m_primitive_type = move_value_from(that.m_primitive_type);
            m_indices_count = move_value_from(that.m_indices_count);
            m_indices = move_value_from(that.m_indices);
            m_vertices_count = move_value_from(that.m_vertices_count);
            m_vertices = move_value_from(that.m_vertices);

            return *this;
        }

        model(model&& that) :
            m_renderer(that.m_renderer)
        {
            *this = std::move(that);
        }

		~model() { free(); }

		void set_primitive_type(IvPrimType type)
		{
			m_primitive_type = type;
		}

		IvPrimType get_primitive_type()
		{
			return m_primitive_type;
		}

		bool model::allocate(size_t indicies_count, size_t verticies_count)
		{
			m_indices_count = indicies_count;
			m_vertices_count = verticies_count;

			m_vertices = m_renderer.GetResourceManager()->CreateVertexBuffer(
				vertex_format_code, (unsigned int)m_vertices_count, nullptr, kDefaultUsage);

			m_indices = IvRenderer::mRenderer->GetResourceManager()->CreateIndexBuffer(
				(unsigned int)m_indices_count, nullptr, kDefaultUsage);

			return true;
		}

		void model::free()
		{
			if (m_indices) m_renderer.GetResourceManager()->Destroy(m_indices);
			if (m_vertices) m_renderer.GetResourceManager()->Destroy(m_vertices);
		}

		BufferRAIIWrapper<IvVertexBuffer, t_VertexFormat> load_vertices()
		{
			return BufferRAIIWrapper<IvVertexBuffer, t_VertexFormat>(m_vertices, (t_VertexFormat*) m_vertices->BeginLoadData());
		}

		BufferRAIIWrapper<IvIndexBuffer, UInt32> load_indicies()
		{
			return BufferRAIIWrapper<IvIndexBuffer, UInt32>(m_indices, (UInt32*) m_indices->BeginLoadData());
		}

		void model::render(IvRenderer & renderer, IvShaderProgram* shader) const
		{
			if (shader)	renderer.SetShaderProgram(shader);

			renderer.Draw(m_primitive_type, m_vertices, m_indices);
		}
	};

	template<IvVertexFormat T1, typename T2>
	inline void copy_to_gpu_mem(model<T1, T2>& model, std::vector<T2> vertices_mem, std::vector<UInt32> indices_mem)
	{
		// allocate GPU memory
		model.allocate(indices_mem.size(), vertices_mem.size());

		// start loading data to GPU memory
		auto& vertices = model.load_vertices();
		auto& indices = model.load_indicies();

		for (size_t i = 0; i < vertices_mem.size(); ++i)
			vertices[i] = vertices_mem[i];

		for (size_t i = 0; i < indices_mem.size(); ++i)
			indices[i] = indices_mem[i];
	}
	
    model<kTNPFormat, IvTNPVertex> create_box(const IvVector3& size, bool right_hand_order, bool invert_normals);

	template<IvVertexFormat vertex_format_code, typename t_VertexFormat>
    void create_quad(model<vertex_format_code, t_VertexFormat>& model, const IvVector3& size, bool right_hand_order, bool invert_normals)
	{
		std::vector<t_VertexFormat> vertices_mem;
		std::vector<UInt32> indices_mem;

		IvVector3 tsize = size;
		tsize /= 2.f;

		/*
		-1,1     1,1
		0    |    1
		*    |    *
		-----+-----
		3    |    2
		*    |    *
		-1,-1    1,-1
		*/

		vertices_mem.push_back({{ 0.f, 1.f, 0.f }, IvVector3{ -1.f, 1.f, 0.f} * tsize });
		vertices_mem.push_back({{ 0.f, 1.f, 0.f }, IvVector3{ 1.f, 1.f, 0.f} * tsize });
		vertices_mem.push_back({{ 0.f, 1.f, 0.f }, IvVector3{ 1.f, -1.f, 0.f} * tsize });
		vertices_mem.push_back({{ 0.f, 1.f, 0.f }, IvVector3{ -1.f, -1.f, 0.f} * tsize });

		indices_mem.push_back(0);
		indices_mem.push_back(1);
		indices_mem.push_back(3);
		indices_mem.push_back(1);
		indices_mem.push_back(2);
		indices_mem.push_back(3);

		if (!right_hand_order) reverse_winding(indices_mem, vertices_mem);

		copy_to_gpu_mem(model, vertices_mem, indices_mem);
	}

	inline model<kTNPFormat, IvTNPVertex> create_quad(const IvVector3& size, bool right_hand_order, bool invert_normals)
	{
		std::vector<IvTNPVertex> vertices_mem;
		std::vector<UInt32> indices_mem;

		IvVector3 tsize = size;
		tsize /= 2.f;

		/*
		-1,1     1,1
		0    |    1
		*    |    *
		-----+-----
		3    |    2
		*    |    *
		-1,-1    1,-1
		*/

		vertices_mem.push_back({ { 0.f, 0.f }, { 0.f, 1.f, 0.f }, IvVector3{ -1.f, 1.f, 0.f} * tsize });
		vertices_mem.push_back({ { 1.f, 0.f }, { 0.f, 1.f, 0.f }, IvVector3{ 1.f, 1.f, 0.f} * tsize });
		vertices_mem.push_back({ { 1.f, 1.f }, { 0.f, 1.f, 0.f }, IvVector3{ 1.f, -1.f, 0.f} * tsize });
		vertices_mem.push_back({ { 0.f, 1.f }, { 0.f, 1.f, 0.f }, IvVector3{ -1.f, -1.f, 0.f} * tsize });

		indices_mem.push_back(0);
		indices_mem.push_back(1);
		indices_mem.push_back(3);
		indices_mem.push_back(1);
		indices_mem.push_back(2);
		indices_mem.push_back(3);

		if (!right_hand_order) reverse_winding(indices_mem, vertices_mem);

        model<kTNPFormat, IvTNPVertex> model;
		copy_to_gpu_mem(model, vertices_mem, indices_mem);
        return model;
	}
}
