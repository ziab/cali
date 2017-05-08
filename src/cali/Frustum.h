#pragma once
#include "IvPlane.h"
#include "IvOBB.h"
#include "IvMatrix44.h"

namespace Cali
{
	class Frustum
	{
		IvPlane m_planes[6];

	public:
		Frustum() {}
		~Frustum() {}

		void construct_frustum(float screenDepth, IvMatrix44 projection_matrix, IvMatrix44 view_matrix);

		bool visible(const IvOBB& box);
	};
}