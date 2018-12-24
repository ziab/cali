#include "Model.h"

namespace cali
{
    model<kTNPFormat, IvTNPVertex> create_box(const IvVector3 & size, bool right_hand_order, bool invert_normals)
	{
		// This method is essentially a copied from DirectXTK / GeometricPrimitive and modified

		// A box has six faces, each one pointing in a different direction.
		const size_t face_count = 6;

		static const IvVector3 faceNormals[face_count] =
		{
			{ 0,  0,  1 },
			{ 0,  0, -1 },
			{ 1,  0,  0 },
			{ -1,  0,  0 },
			{ 0,  1,  0 },
			{ 0, -1,  0 },
		};

		std::vector<IvTNPVertex> vertices_mem;
		std::vector<UInt32> indices_mem;

		IvVector3 tsize = size;
		tsize /= 2.f;

		// Create each face in turn.
		for (int i = 0; i < face_count; i++)
		{
			const IvVector3& normal = faceNormals[i];

			// Get two vectors perpendicular both to the face normal and to each other.
			IvVector3 basis = (i >= 4) ? IvVector3{ 0.0f, 0.0f, 1.0f } : IvVector3{ 0.0f, 1.0f, 0.0f };

			IvVector3 side1 = normal.Cross(basis);
			IvVector3 side2 = normal.Cross(side1);

			// Six indices (two triangles) per face.
			UInt32 vbase = (UInt32)vertices_mem.size();
			indices_mem.push_back(vbase + 0);
			indices_mem.push_back(vbase + 1);
			indices_mem.push_back(vbase + 2);

			indices_mem.push_back(vbase + 0);
			indices_mem.push_back(vbase + 2);
			indices_mem.push_back(vbase + 3);

			// Four vertices per face.
			// TODO: check if the UV values are correct
			vertices_mem.push_back(IvTNPVertex{ { 0.0f, 0.0f }, normal, IvVector3(normal - side1 - side2) * tsize });
			vertices_mem.push_back(IvTNPVertex{ { 1.0f, 0.0f }, normal, IvVector3(normal - side1 + side2) * tsize });
			vertices_mem.push_back(IvTNPVertex{ { 0.0f, 1.0f }, normal, IvVector3(normal + side1 + side2) * tsize });
			vertices_mem.push_back(IvTNPVertex{ { 1.0f, 1.0f }, normal, IvVector3(normal + side1 - side2) * tsize });
		}

		if (!right_hand_order) reverse_winding(indices_mem, vertices_mem);
		//if (invert_normals) Cali::invert_normals(vertices_mem);

        model<kTNPFormat, IvTNPVertex> model;
		copy_to_gpu_mem<kTNPFormat, IvTNPVertex>(model, vertices_mem, indices_mem);
        return model;
	}
}