// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "Phong.h"

using namespace optix;

#ifndef M_1_PIf
#define M_1_PIf 0.31830988618379067154
#endif

float3 Phong::shade(const Ray& r, HitInfo& hit, bool emit) const
{
	float3 rho_d = get_diffuse(hit);
	float3 rho_s = get_specular(hit);
	float s = get_shininess(hit);

	// Implement Phong reflection here.
	//
	// Input:  r          (the ray that hit the material)
	//         hit        (info about the ray-surface intersection)
	//         emit       (passed on to Emission::shade)
	//
	// Return: radiance reflected to where the ray was coming from
	//
	// Relevant data fields that are available (see Lambertian.h, Ray.h, and above):
	// lights             (vector of pointers to the lights in the scene)
	// hit.position       (position where the ray hit the material)
	// hit.shading_normal (surface normal where the ray hit the material)
	// rho_d              (difuse reflectance of the material)
	// rho_s              (specular reflectance of the material)
	// s                  (shininess or Phong exponent of the material)
	//
	// Hint: Call the sample function associated with each light in the scene.
	//return Lambertian::shade(r,hit,emit);

	float3 radiance = make_float3(0.0f);

	float3 wo=-r.direction;
	for (int i = 0; i < lights.size(); i++) {
		float3 wi;
		float3 L;
		bool shade = lights[i]->sample(hit.position, wi, L);
		float3 wr=reflect(wi, hit.shading_normal);
		if (dot(wi, hit.shading_normal) > 0) {
			float3 a = rho_d/M_PIf;
			float3 b = rho_s*(s+2)/(2*M_PIf);
			float c = pow(dot(wo,wr), s);
			float3 d = a + b * c;
			float3 e = L * dot(wi, hit.shading_normal);
			radiance = radiance+d * e;
		}
	}
	return radiance;
}
