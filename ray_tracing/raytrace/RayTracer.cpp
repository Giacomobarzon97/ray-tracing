// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "ObjMaterial.h"
#include "fresnel.h"
#include "RayTracer.h"

using namespace optix;

bool RayTracer::trace_reflected(const Ray& in, const HitInfo& in_hit, Ray& out, HitInfo& out_hit) const
{
	// Initialize the reflected ray and trace it.
	//
	// Input:  in         (the ray to be reflected)
	//         in_hit     (info about the ray-surface intersection)
	//
	// Output: out        (the reflected ray)
	//         out_hit    (info about the reflected ray)
	//
	// Return: true if the reflected ray hit anything
	//
	// Hints: (a) There is a reflect function available in the OptiX math library.
	//        (b) Set out_hit.ray_ior and out_hit.trace_depth.
	float3 out_direction=reflect(in.direction, in_hit.shading_normal);
	out = Ray(in_hit.position, out_direction,0,1e-4, RT_DEFAULT_MAX);
	trace_to_closest(out, out_hit);
	float3 normal;
	out_hit.ray_ior = in_hit.ray_ior;
	out_hit.trace_depth = in_hit.trace_depth + 1;
	return out_hit.has_hit;
}

bool RayTracer::trace_refracted(const Ray& in, const HitInfo& in_hit, Ray& out, HitInfo& out_hit) const
{
	// Initialize the refracted ray and trace it.
	//
	// Input:  in         (the ray to be refracted)
	//         in_hit     (info about the ray-surface intersection)
	//
	// Output: out        (the refracted ray)
	//         out_hit    (info about the refracted ray)
	//
	// Return: true if the refracted ray hit anything
	//
	// Hints: (a) There is a refract function available in the OptiX math library.
	//        (b) Set out_hit.ray_ior and out_hit.trace_depth.
	//        (c) Remember that the function must handle total internal reflection.
	float3 normal;
	float out_ior = get_ior_out(in,in_hit,normal);
	out_hit.ray_ior = out_ior;
	bool refraction=refract(out.direction, in.direction, normal, out_hit.ray_ior / in_hit.ray_ior);
	if (refraction) {
		out.origin = in_hit.position;
		out.tmin = 1e-4;
		out.tmax = RT_DEFAULT_MAX;
		out_hit.trace_depth = in_hit.trace_depth + 1;
	
		trace_to_closest(out, out_hit);
		return out_hit.has_hit;
	}
	return refraction;
}

bool RayTracer::trace_refracted(const Ray& in, const HitInfo& in_hit, Ray& out, HitInfo& out_hit, float& R) const
{
	// Initialize the refracted ray and trace it.
	// Compute the Fresnel reflectance (see fresnel.h) and return it in R.
	//
	// Input:  in         (the ray to be refracted)
	//         in_hit     (info about the ray-surface intersection)
	//
	// Output: out        (the refracted ray)
	//         out_hit    (info about the refracted ray)
	//
	// Return: true if the refracted ray hit anything
	//
	// Hints: (a) There is a refract function available in the OptiX math library.
	//        (b) Set out_hit.ray_ior and out_hit.trace_depth.
	//        (c) Remember that the function must handle total internal reflection.
	//R = 0.1;
	float3 normal;
	float out_ior = get_ior_out(in, in_hit, normal);
	out_hit.ray_ior = out_ior;
	bool refraction = refract(out.direction, in.direction, normal, out_hit.ray_ior / in_hit.ray_ior);
	if (refraction) {
		out.origin = in_hit.position;
		out.tmin = 1e-4;
		out.tmax = RT_DEFAULT_MAX;
		out_hit.trace_depth = in_hit.trace_depth + 1;

		float cos_theta1 = dot(in.direction, in_hit.shading_normal);
		float cos_theta2 = dot(out.direction, in_hit.shading_normal);
		R = fresnel_R(cos_theta1, cos_theta2, in_hit.ray_ior, out_hit.ray_ior);

		trace_to_closest(out, out_hit);
	}
	else {
		R = 1;
	}

	return out_hit.has_hit;
}

//If a ray it's an object it get's it's material and then
//the index of refraction related to it otherwise returns the index refraction of the "air"
//which is 1 by default
float RayTracer::get_ior_out(const Ray& in, const HitInfo& in_hit, float3& normal) const
{
	normal = in_hit.shading_normal;
	if(dot(normal, in.direction) > 0.0)
	{
		normal = -normal;
		return 1.0f;
	}
	const ObjMaterial* m = in_hit.material;
	return m ? m->ior : 1.0f;
}
