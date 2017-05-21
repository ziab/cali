#pragma once
#include <IvDoubleVector3.h>
#include <IvVector3.h>

namespace Cali
{
	namespace Math
	{
		inline double sum(const IvDoubleVector3& vec)
		{
			return vec.x + vec.y + vec.z;
		}

		inline bool intersect(const IvDoubleVector3& raydir, const IvDoubleVector3& rayorig, const IvDoubleVector3& spherepos,
			double rad, IvDoubleVector3& hitpoint, double& distance, IvDoubleVector3& normal)
		{
			double a = sum(raydir*raydir);
			double b = sum(raydir * (2.0 * (rayorig - spherepos)));
			double c = sum(spherepos*spherepos) + sum(rayorig*rayorig) - 2.0*sum(rayorig*spherepos) - rad*rad;
			double D = b*b + (-4.0)*a*c;

			// If ray can not intersect then stop
			if (D < 0)
				return false;
			D = sqrt(D);

			// Ray can intersect the sphere, solve the closer hitpoint
			double t = (-0.5)*(b + D) / a;
			if (t > 0.0)
			{
				distance = sqrt(a) * t;
				hitpoint = rayorig + t * raydir;
				normal = (hitpoint - spherepos) / rad;
			}
			else
			{
				return false;
			}

			return true;
		}

		// Intersects ray r = p + td, |d| = 1, with sphere s and, if intersecting, 
		// returns t value of intersection and intersection point q 
		inline int intersect_ray_sphere(const IvDoubleVector3 p, const IvDoubleVector3& d, const IvDoubleVector3& C,
			double R, IvDoubleVector3& hit, double &t, IvDoubleVector3& normal)
		{
			IvDoubleVector3 m = p - C;
			double b = Dot(m, d);
			double c = Dot(m, m) - R * R;

			// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0) 
			if (c > 0.0f && b > 0.0f) return 0;
			double discr = b*b - c;

			// A negative discriminant corresponds to ray missing sphere 
			if (discr < 0.0f) return 0;

			// Ray now found to intersect sphere, compute smallest t value of intersection
			t = -b - sqrt(discr);

			// If t is negative, ray started inside sphere
			if (t < 0.0f)
			{
				double a = Dot(d, d);
				t /= (2 * a);
			}

			hit = p + t * d;

			normal = hit - C / R;

			return 1;
		}

		inline void get_lon_lat_from_point_on_sphere(const IvVector3& sphere_center, double sphere_radius,
			const IvVector3& point, double& lon, double& lat)
		{
			IvVector3 point_coord_related_to_sphere = point - sphere_center;
			// note that the origin is shifted by (kPI / 2.0)
			lat = acos(-point_coord_related_to_sphere.z / sphere_radius) - (kPI / 2.0);
			lon = atan(-point_coord_related_to_sphere.y / point_coord_related_to_sphere.x) - (kPI / 2.0);

			if (point_coord_related_to_sphere.x >= 0.0)
				lon += kPI;
		}

		inline void position_on_sphere(double lon, double lat, double R, const IvDoubleVector3& C,
			IvDoubleVector3& position, IvDoubleVector3& normal, IvDoubleVector3& tangent)
		{
			double cos_lat = cos(lat);

			IvDoubleVector3 ps;
			ps.x = R * cos_lat * sin(lon);
			ps.y = R * cos_lat * cos(lon);
			ps.z = R * sin(lat);

			position = ps + C;
			normal = ps;
			normal.Normalize();

			double sin_lon = cos(lat);

			tangent = normal.Cross(IvDoubleVector3::zAxis);
			tangent.Normalize();
		}

		/// R is sphere radius
		/// C is sphere center position
		/// x,y - are coordinates on surface
		inline void position_on_sphere_from_surface(double x, double y, double R, const IvDoubleVector3& C,
			IvDoubleVector3& position, IvDoubleVector3& normal, IvDoubleVector3& tangent)
		{
			double lon = x / R;
			double lat = 2 * atan(exp(y / R)) - kPI / 2.0;

			return position_on_sphere(lon, lat, R, C, position, normal, tangent);
		}

		inline IvDoubleVector3 cube_to_sphere(IvDoubleVector3 cube, double sphere_radius, const IvDoubleVector3& sphere_center)
		{
			cube /= sphere_radius;
			IvDoubleVector3 sphere;
			sphere.x = cube.x * sqrt(1.0 - cube.y * cube.y * 0.5 - cube.z * cube.z * 0.5 + cube.y * cube.y * cube.z * cube.z / 3.0);
			sphere.y = cube.y * sqrt(1.0 - cube.z * cube.z * 0.5 - cube.x * cube.x * 0.5 + cube.z * cube.z * cube.x * cube.x / 3.0);
			sphere.z = cube.z * sqrt(1.0 - cube.x * cube.x * 0.5 - cube.y * cube.y * 0.5 + cube.x * cube.x * cube.y * cube.y / 3.0);
			return sphere * sphere_radius + sphere_center;
		}
	}
}