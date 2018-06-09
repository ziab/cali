#include "Frustum.h"


namespace cali
{
	/*
	void frustum::construct_frustum(float screenDepth, IvMatrix44 projectionMatrix, IvMatrix44 viewMatrix)
	{
		// Create the frustum matrix from the view matrix and updated projection matrix.
		IvMatrix44 viewProjection= projectionMatrix * viewMatrix;

		// Left plane
		m_planes[0].Set(
			viewProjection(0,3) + viewProjection(0,0), 
			viewProjection(1,3) + viewProjection(1,0), 
			viewProjection(2,3) + viewProjection(2,0), 
			viewProjection(3,3) + viewProjection(3,0));

		// Right plane
		m_planes[1].Set(
			viewProjection(0,3) - viewProjection(0,0), 
			viewProjection(1,3) - viewProjection(1,0), 
			viewProjection(2,3) - viewProjection(2,0), 
			viewProjection(3,3) - viewProjection(3,0));

		// Top plane
		m_planes[2].Set(
			viewProjection(0,3) - viewProjection(0,1), 
			viewProjection(1,3) - viewProjection(1,1), 
			viewProjection(2,3) - viewProjection(2,1), 
			viewProjection(3,3) - viewProjection(3,1));

		// Bottom plane
		m_planes[3].Set(
			viewProjection(0,3) + viewProjection(0,1), 
			viewProjection(1,3) + viewProjection(1,1),
			viewProjection(2,3) + viewProjection(2,1), 
			viewProjection(3,3) + viewProjection(3,1));

		// Near plane
		m_planes[4].Set(
			viewProjection(0,2), 
			viewProjection(1,2), 
			viewProjection(2,2), 
			viewProjection(3,2));

		// Far plane
		m_planes[5].Set(
			viewProjection(0,3) - viewProjection(0,2), 
			viewProjection(1,3) - viewProjection(1,2), 
			viewProjection(2,3) - viewProjection(2,2), 
			viewProjection(3,3) - viewProjection(3,2));

		return;
	}
	*/

	void frustum::construct_frustum(const IvMatrix44& iv_projection_matrix, const IvMatrix44& iv_view_matrix)
	{
		using namespace DirectX;

		const float* proj_data = iv_projection_matrix;
		const float* view_data = iv_view_matrix;

		assert(sizeof(XMFLOAT4X4A) == sizeof(IvMatrix44));

		XMFLOAT4X4A projection_matrix, view_matrix;
		memcpy(projection_matrix.m, proj_data, sizeof(XMFLOAT4X4A));
		memcpy(view_matrix.m, view_data, sizeof(XMFLOAT4X4A));

		BoundingFrustum::CreateFromMatrix(m_frustum, XMLoadFloat4x4A(&projection_matrix));
		BoundingFrustum tmp(XMLoadFloat4x4A(&projection_matrix));
		XMVECTOR det;
		tmp.Transform(m_frustum, XMMatrixInverse(&det, XMLoadFloat4x4A(&view_matrix)));
		m_frustum.Near = 1.0f;
	}

	bool frustum::visible(const IvOBB & box) const
	{
		/*for (auto&& plane : m_planes)
		{
			if (box.Classify(plane) >= 0.0f) continue;
			return false;
		}
		*/
		return true;
	}
	bool frustum::contains_aligned_bounding_box(float x, float y, float z, float extent_x, float extent_y, float extent_z) const
	{
		using namespace DirectX;
		BoundingBox box(XMFLOAT3{ x, y, z }, XMFLOAT3{ extent_x, extent_y, extent_z });

		return m_frustum.Contains(box) != DISJOINT;
	}
}
