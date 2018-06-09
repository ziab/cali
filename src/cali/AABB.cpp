#include "AABB.h"
#include "CaliMath.h"

namespace cali
{
	void AABB::reset()
	{
		m_minima = { DBL_MAX, DBL_MAX, DBL_MAX };
		m_maxima = { DBL_MIN, DBL_MIN, DBL_MIN };
	}

	void AABB::add_point(const IvDoubleVector3 & point)
	{
		if (point.x < m_minima.x)
			m_minima.x = point.x;
		else if (point.x > m_maxima.x)
			m_maxima.x = point.x;
		if (point.y < m_minima.y)
			m_minima.y = point.y;
		else if (point.y > m_maxima.y)
			m_maxima.y = point.y;
		if (point.z < m_minima.z)
			m_minima.z = point.z;
		else if (point.z > m_maxima.z)
			m_maxima.z = point.z;
	}

	IvDoubleVector3 AABB::get_center()
	{
		return Math::lerp(m_minima, m_maxima, 0.5);
	}
	IvDoubleVector3 AABB::get_extents()
	{
		return m_maxima - m_minima;
	}
}
