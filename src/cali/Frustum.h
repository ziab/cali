#pragma once
#include "IvPlane.h"
#include "IvOBB.h"
#include "IvMatrix44.h"

#include <DirectXCollision.h>

namespace cali
{
	class frustum
	{
		//IvPlane m_planes[6];
		DirectX::BoundingFrustum m_frustum;

	public:
		frustum() {}
		~frustum() {}

		void construct_frustum(const IvMatrix44& iv_projection_matrix, const IvMatrix44& iv_view_matrix);

		bool visible(const IvOBB& box) const;
		bool contains_aligned_bounding_box(float x, float y, float z, float extent_x, float extent_y, float extent_z) const;
	};
}