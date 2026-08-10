#pragma once
#include <IvDoubleVector3.h>
#include <IvVector3.h>

namespace cali
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

			// Exit if r�s origin outside s (c > 0) and r pointing away from s (b > 0) 
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

		inline IvDoubleVector3 cube_to_sphere(double x, double y, double sphere_radius, 
			const IvDoubleVector3& sphere_center, IvDoubleVector3& normal)
		{
			IvDoubleVector3 cube{x, sphere_radius, y};
			cube /= sphere_radius;
			IvDoubleVector3 sphere;
			sphere.x = cube.x * sqrt(1.0 - cube.y * cube.y * 0.5 - cube.z * cube.z * 0.5 + cube.y * cube.y * cube.z * cube.z / 3.0);
			sphere.y = cube.y * sqrt(1.0 - cube.z * cube.z * 0.5 - cube.x * cube.x * 0.5 + cube.z * cube.z * cube.x * cube.x / 3.0);
			sphere.z = cube.z * sqrt(1.0 - cube.x * cube.x * 0.5 - cube.y * cube.y * 0.5 + cube.x * cube.x * cube.y * cube.y / 3.0);
			
			normal = sphere;
			return sphere * sphere_radius + sphere_center;
		}

		inline IvDoubleVector3 adjusted_cube_to_sphere(double x, double y, double sphere_radius,
			const IvDoubleVector3& sphere_center, IvDoubleVector3& normal)
		{
			x /= sphere_radius;
			y /= sphere_radius;

			double phi = x * kPI / 4.0;
			double theta = atan(tan(kPI * y / 4.0) * cos(phi));

			IvDoubleVector3 position, tangent;
			position_on_sphere(phi, theta, sphere_radius, sphere_center, position, normal, tangent);

			return position;
		}

		inline void adjusted_sphere_to_cube(double phi, double theta, double sphere_radius, double& x, double& y)
		{
			x = (phi * 4 / kPI) * sphere_radius;
			y = atan( tan(theta) / cos(phi) ) * 4.0 / kPI * sphere_radius;
		}

		// -----------------------------------------------------------------
		// Cube-face helpers for full planet (6 faces). Top face (+Y) is
		// the original adjusted mapping; other faces are obtained by
		// rotating the top-face sphere vector so that +Y maps to the
		// face normal.
		// -----------------------------------------------------------------
		enum class CubeFace : int { PosY = 0, NegY = 1, PosX = 2, NegX = 3, PosZ = 4, NegZ = 5 };

		inline IvDoubleVector3 rotate_top_to_face(const IvDoubleVector3& v, CubeFace face)
		{
			switch (face)
			{
			case CubeFace::PosY: return v; // identity
			case CubeFace::NegY: return IvDoubleVector3{ v.x, -v.y, -v.z };
			case CubeFace::PosX: return IvDoubleVector3{ v.y, -v.x, v.z };
			case CubeFace::NegX: return IvDoubleVector3{ -v.y, v.x, v.z };
			case CubeFace::PosZ: return IvDoubleVector3{ v.x, -v.z, v.y };
			case CubeFace::NegZ: return IvDoubleVector3{ v.x, v.z, -v.y };
			default: return v;
			}
		}

		inline IvDoubleVector3 rotate_face_to_top(const IvDoubleVector3& v, CubeFace face)
		{
			// inverse of rotate_top_to_face
			switch (face)
			{
			case CubeFace::PosY: return v;
			case CubeFace::NegY: return IvDoubleVector3{ v.x, -v.y, -v.z };
			case CubeFace::PosX: return IvDoubleVector3{ -v.y, v.x, v.z };
			case CubeFace::NegX: return IvDoubleVector3{ v.y, -v.x, v.z };
			case CubeFace::PosZ: return IvDoubleVector3{ v.x, v.z, -v.y };
			case CubeFace::NegZ: return IvDoubleVector3{ v.x, -v.z, v.y };
			default: return v;
			}
		}

		inline IvDoubleVector3 adjusted_cube_to_sphere_face(CubeFace face, double x, double y, double sphere_radius,
			const IvDoubleVector3& sphere_center, IvDoubleVector3& normal)
		{
			IvDoubleVector3 top = adjusted_cube_to_sphere(x, y, sphere_radius, IvDoubleVector3{0,0,0}, normal);
			IvDoubleVector3 rotated = rotate_top_to_face(top, face);
			normal = rotate_top_to_face(normal, face);
			return rotated + sphere_center;
		}

		// Determine cube face from world direction and compute face-local x,y
		inline bool world_to_cube_face(const IvDoubleVector3& world_pos, const IvDoubleVector3& sphere_center,
			double sphere_radius, CubeFace& out_face, double& out_x, double& out_y)
		{
			IvDoubleVector3 d = world_pos - sphere_center;
			double ax = fabs(d.x), ay = fabs(d.y), az = fabs(d.z);
			if (ay >= ax && ay >= az)
				out_face = d.y >= 0 ? CubeFace::PosY : CubeFace::NegY;
			else if (ax >= ay && ax >= az)
				out_face = d.x >= 0 ? CubeFace::PosX : CubeFace::NegX;
			else
				out_face = d.z >= 0 ? CubeFace::PosZ : CubeFace::NegZ;

			// rotate to top space
			IvDoubleVector3 d_local = rotate_face_to_top(d, out_face);
			double len = d_local.Length();
			if (len < 1e-9) { out_x = 0; out_y = 0; return false; }
			double R = sphere_radius;
			// spherical coords as in adjusted mapping (pole = +Y)
			double cos_lat = sqrt(d_local.x * d_local.x + d_local.y * d_local.y) / len;
			// guard
			if (cos_lat < 1e-9) cos_lat = 1e-9;
			double lon = atan2(d_local.x, d_local.y); // phi
			double lat = asin(d_local.z / len); // theta
			// map to cube face plane
			adjusted_sphere_to_cube(lon, lat, R, out_x, out_y);
			return true;
		}
	}
}