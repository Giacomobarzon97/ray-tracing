// 02576 OptiX Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#ifndef AREALIGHT_H
#define AREALIGHT_H

#include <optix.h>
#include <optix_math.h>
#include "../cuda/random.h"

// Area light variables
rtBuffer<float3> light_verts;     
rtBuffer<float3> light_norms;
rtBuffer<int3> light_idxs;
rtDeclareVariable(float3, light_emission, , );

__device__ __inline__ void sample_center(const float3& pos, float3& dir, float3& L, float& dist)
{
	// Compute output given the following information.
	//
	// Input:  pos    (observed surface position in the scene)
	//
	// Output: dir    (direction toward the light)
	//         L      (radiance received from the direction dir)
	//         dist   (distance to the sampled position on the light source)
	//
	// Relevant data fields that are available (see above):
	// light_verts    (vertex positions for the indexed face set representing the light source)
	// light_norms    (vertex normals for the indexed face set representing the light source)
	// light_idxs     (vertex indices for each triangle in the indexed face set)
	// light_emission (radiance emitted by the light source)
	//
	// Hint: (a) Find the face normal for each triangle in light_vidxs and use these
	//       to add up triangle areas and find the average normal.
	//       (b) OptiX includes a function normalize(v) which returns the 
	//       vector v normalized to unit length.
	float3 light_pos=make_float3(0.0f);

	for (int i = 0; i < light_verts.size(); i++) {
		light_pos = light_pos+light_verts[i];
	}
	light_pos = light_pos / light_verts.size();

	L = make_float3(0.0f);
	dir = normalize(light_pos - pos);
	dist = length(dir);

	float3 avg_normal = make_float3(0.0f);
	for (int i = 0; i < light_idxs.size(); i++) {
		float3 face_normal = normalize(light_norms[light_idxs[i].x] + light_norms[light_idxs[i].y] + light_norms[light_idxs[i].z]);
		avg_normal = face_normal + avg_normal;
		float3 a = light_verts[light_idxs[i].x];
		float3 b = light_verts[light_idxs[i].y];
		float3 c = light_verts[light_idxs[i].z];
		float face_area = (a.x*(b.y - c.y) + b.x*(c.y - a.y) + c.x*(a.y - b.y))/2;
		L = L + dot(-dir, face_normal)*light_emission*face_area;
	}
	avg_normal = avg_normal / light_idxs.size();
}

__device__ __inline__ void sample(const float3& pos, float3& dir, float3& L, float& dist, uint& t)
{
  // sample a triangle
  
  // sample a point in the triangle

  // compute the sample normal

  // Find distance and direction

  // Compute emitted radiance
}

#endif // AREALIGHT_H