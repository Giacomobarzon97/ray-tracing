// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "IndexedFaceSet.h"
#include "ObjMaterial.h"
#include "mt_random.h"
#include "cdf_bsearch.h"
#include "HitInfo.h"
#include "AreaLight.h"

using namespace optix;

bool AreaLight::sample(const float3& pos, float3& dir, float3& L) const
{
	// Compute output and return value given the following information.
	//
	// Input:  pos  (the position of the geometry in the scene)
	//
	// Output: dir  (the direction toward the light)
	//         L    (the radiance received from the direction dir)
	//
	// Return: true if not in shadow
	//
	// Relevant data fields that are available (see Light.h and above):
	// shadows             (on/off flag for shadows)
	// tracer              (pointer to ray tracer)
	// normals             (indexed face set of vertex normals)
	// mesh->face_areas    (array of face areas in the light source)
	//
	// Hints: (a) Use mesh->compute_bbox().center() to get the center of 
	//        the light source bounding box.
	//        (b) Use the function get_emission(...) to get the radiance
	//        emitted by a triangle in the mesh.

	const IndexedFaceSet& normals = mesh->normals;
	int n_faces = mesh->geometry.no_faces();

	uint3 random_triangle = mesh->geometry.face(rand() % n_faces);
	float xi_1 = mt_random();
	float xi_2 = mt_random();

	float u = 1 - sqrt(xi_1);
	float v = (1 - xi_2) * sqrt(xi_1);
	float w = xi_2 * sqrt(xi_1);

	float3 light_pos = u * mesh->geometry.vertex(random_triangle.x) + v * mesh->geometry.vertex(random_triangle.y) + w * mesh->geometry.vertex(random_triangle.z);
	light_pos = mesh->compute_bbox().center();

	dir = light_pos - pos;
	float dir_norm = length(dir);
	dir = normalize(dir);

	float3 intensity = make_float3(0.0f);
	for (int i = 0; i < n_faces; i++) {
		uint3 face = mesh->geometry.face(i);
		float3 face_normal = normalize(normals.vertex(face.x) + normals.vertex(face.y) + normals.vertex(face.z));
		intensity = intensity + dot(-dir, face_normal) * get_emission(i) * mesh->face_areas[i];
	}

	float epsilon = 1e-4;
	float cutoff_distance = dir_norm - epsilon;
	Ray shadow_ray = Ray(pos, dir, 0, epsilon, dir_norm-epsilon);
	HitInfo hit_info = HitInfo();
	tracer->trace_to_any(shadow_ray, hit_info);

	if (!hit_info.has_hit) {
		L = intensity / (dir_norm * dir_norm);
	}
	return !hit_info.has_hit;

}

bool AreaLight::emit(Ray& r, HitInfo& hit, float3& Phi) const
{
  // Generate and trace a ray carrying radiance emitted from this area light.
  //
  // Output: r    (the new ray)
  //         hit  (info about the ray-surface intersection)
  //         Phi  (the flux carried by the emitted ray)
  //
  // Return: true if the ray hits anything when traced
  //
  // Relevant data fields that are available (see Light.h and Ray.h):
  // tracer              (pointer to ray tracer)
  // geometry            (indexed face set of triangle vertices)
  // normals             (indexed face set of vertex normals)
  // no_of_faces         (number of faces in triangle mesh for light source)
  // mesh->surface_area  (total surface area of the light source)
  // r.origin            (starting position of ray)
  // r.direction         (direction of ray)

  // Get geometry info
  const IndexedFaceSet& geometry = mesh->geometry;
	const IndexedFaceSet& normals = mesh->normals;
	const float no_of_faces = static_cast<float>(geometry.no_faces());

  // Sample ray origin and direction
 
  // Trace ray
  
  // If a surface was hit, compute Phi and return true

  return false;
}

float3 AreaLight::get_emission(unsigned int triangle_id) const
{
  const ObjMaterial& mat = mesh->materials[mesh->mat_idx[triangle_id]];
  return make_float3(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
}
