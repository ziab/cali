#pragma once

//-------------------------------------------------------------------------------
//-- Dependencies ---------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <IvWriter.h>
#include <IvDoubleVector3.h>

#include <float.h>

namespace Cali
{
	//-------------------------------------------------------------------------------
	//-- Classes --------------------------------------------------------------------
	//-------------------------------------------------------------------------------

	class AABB
	{
	public:
		// constructor/destructor
		AABB() :
			m_minima(DBL_MAX, DBL_MAX, DBL_MAX), 
			m_maxima(DBL_MIN, DBL_MIN, DBL_MIN)
		{}

		AABB(const IvDoubleVector3& min, const IvDoubleVector3& max) :
			m_minima(min), 
			m_maxima(max)
		{}

		~AABB() {}

		// copy operations
		AABB(const AABB& other);
		AABB& operator=(const AABB& other);

		// text output (for debugging)
		friend IvWriter& operator<<(IvWriter& out, const AABB& source);

		// accessors
		inline const IvDoubleVector3& get_minima() const { return m_minima; }
		inline const IvDoubleVector3& get_maxima() const { return m_maxima; }

		// comparison
		bool operator==(const AABB& other) const;
		bool operator!=(const AABB& other) const;

		// manipulators
		void reset();
		void set_points(const IvDoubleVector3* points, unsigned int numPoints);
		void set(const IvDoubleVector3& min, const IvDoubleVector3& max);
		void add_point(const IvDoubleVector3& point);

		IvDoubleVector3 get_center();
		IvDoubleVector3 get_extents();

		template <typename ... Args>
		void set_points(const Args ... args)
		{
			if (sizeof...(Args) == 0) return;
			const IvDoubleVector3* points[] = { args... };

			set(*points[0], *points[0]);

			for (size_t i = 0; i < sizeof...(Args); ++i)
			{
				add_point(*points[i]);
			}
		}

	protected:
		IvDoubleVector3       m_minima, m_maxima;

	private:
	};

	//-------------------------------------------------------------------------------
	//-- Inlines --------------------------------------------------------------------
	//-------------------------------------------------------------------------------

	inline AABB& AABB::operator=(const AABB& other)
	{
		// if same object
		if (this == &other)	return *this;

		m_minima = other.m_minima;
		m_maxima = other.m_maxima;

		return *this;
	}

	inline AABB::AABB(const AABB& other) :
		m_minima(other.m_minima),
		m_maxima(other.m_maxima)
	{
	}


	inline void AABB::set(const IvDoubleVector3 & min, const IvDoubleVector3 & max)
	{
		m_minima = min;
		m_maxima = max;
	}
}