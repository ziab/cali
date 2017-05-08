#include "Frustum.h"

namespace Cali
{
	void Frustum::construct_frustum(float screenDepth, IvMatrix44 projectionMatrix, IvMatrix44 viewMatrix)
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

	bool Frustum::visible(const IvOBB & box)
	{
		for (auto&& plane : m_planes)
		{
			if (box.Classify(plane) >= 0.0f) continue;
			return false;
		}

		return true;
	}
}
