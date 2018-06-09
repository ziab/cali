#pragma once
#include <cmath>

namespace cali
{
	namespace Math
	{
		template<typename T, typename CT>
		inline T lerp(const T& a, const T& b, CT f)
		{
			return a + f * (b - a);
		}

		template<typename T, typename CT>
		T quad_lerp(const T& a, const T& b, const T& c, const T& d, CT u, CT v)
		{
			// Given a (u,v) coordinate that defines a 2D local position inside a planar quadrilateral, find the
			// absolute 3D (x,y,z) coordinate at that location.
			//
			//  0 <----u----> 1
			//  a ----------- b    0
			//  |             |   /|\
			//  |             |    |
			//  |             |    v
			//  |  *(u,v)     |    |
			//  |             |   \|/
			//  d------------ c    1
			//
			// a, b, c, and d are the vertices of the quadrilateral. They are assumed to exist in the
			// same plane in 3D space, but this function will allow for some non-planar error.
			//
			// Variables u and v are the two-dimensional local coordinates inside the quadrilateral.
			// To find a point that is inside the quadrilateral, both u and v must be between 0 and 1 inclusive.  
			// For example, if you send this function u=0, v=0, then it will return coordinate "a".  
			// Similarly, coordinate u=1, v=1 will return vector "c". Any values between 0 and 1
			// will return a coordinate that is bi-linearly interpolated between the four vertices.

			T abu = lerp(a, b, u);
			T dcu = lerp(d, c, u);
			return lerp(abu, dcu, v);
		}
	}
}